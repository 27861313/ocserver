#include "ikcpserver.h"
#include "ikcpintrinsic.inc"
#include "ikcpproto.h"
#include <common/iatom.h>
#include <common/idatetime.h>
#include <common/ihashmap.h>
#include <common/ilist.h>
#include <common/isyslog.h>
#include <stdio.h>
#include <string.h>

void kcpconnection_addref(void *arg)
{
	ikcpconnection *lconn = (ikcpconnection *)arg;
	AtomicAdd(&lconn->_ref, 1);
}

void kcpconnection_subref(void *arg) //附带自动删除
{
	ikcpconnection *lconn = (ikcpconnection *)arg;
	if (AtomicSubFetch(&lconn->_ref, 1) == 0)
	{
		kcpconnection_close(lconn);
		kcpconnection_release(lconn);
	}
}

void kcpconnection_clear_timeout(ikcpconnection *lconn)
{
	lconn->_ntimeout = 0;
}

//---------------------------------------------------------------
//function:
//          udpoutput UDP 发送回调函数
//Access:
//           public
//Parameter:
//          [in] const char * buf -
//          [in] int len -
//          [in] struct IKCPCB * kcp -
//          [in] void * user -
//Returns:
//          int -
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
int udpoutput(const char *buf, int len, struct IKCPCB *kcp, void *user)
{
	ikcpconnection *lconn = (ikcpconnection *)user;
	kcpudpsend(lconn->_parent, &lconn->_remote, (char *)buf, len);
	return 0;
}

ikcpconnection *kcpconnection_create(ikcpserver *lsrv, const iendpoint udpremote)
{
	ikcpconnection *lconn = calloc(1, sizeof(ikcpconnection));
	if (lconn == NULL)
		return NULL;

	lconn->_parent = lsrv;
	lconn->_conv = iocworker_nextid(&lsrv->_idgenerate);
	lconn->_ntimeout = 0;
	lconn->_status = IOC_NETCONN_NOMAL;

	lconn->_lkcp = ikcp_create(lconn->_conv, (void *)lconn);
	lconn->_lkcp->output = &udpoutput;
	iocevlistener_create(lsrv->_system, &lconn->_evtlisten, TRUE, TRUE /*可考虑false线程不安全模式，工作在单线程模式下*/);

	// 启动快速模式
	// 第二个参数 nodelay 模式-启用     以后若干常规加速将启动
	// 第三个参数 interval为内部处理时钟，默认设置为 10ms
	// 第四个参数 resend为快速重传指标，设置为2
	// 第五个参数 为是否禁用常规流控，这里禁止
	//ikcp_nodelay(lconn->_lkcp, 1, 10, 2, 1);
	ikcp_nodelay(lconn->_lkcp, 1, 5, 1, 1); // 设置成1次ACK跨越直接重传, 这样反应速度会更快. 内部时钟5毫秒

	memcpy((char *)&lconn->_remote, &udpremote, sizeof(iendpoint));

	return lconn;
}

void kcpconnection_close(ikcpconnection *lconn)
{
	char pakbuf[64];
	if (lconn->_conv != 0 && lconn->_status != IOC_NETCONN_CLOSE)
	{
		if (lconn->_parent != NULL)
		{
			int32_i infosize = kcpserver_disconnect_pack((char *)&pakbuf, lconn->_conv);
			//send kcp info
			kcpudpsend(lconn->_parent, &lconn->_remote, pakbuf, infosize);
		}
		lconn->_status = IOC_NETCONN_CLOSE;
	}
}

void kcpconnection_release(ikcpconnection *lconn)
{
	if (lconn->_conv != 0)
	{
		lconn->_conv = 0;
		iocevlistener_release(lconn->_parent->_system, &lconn->_evtlisten);
		lconn->_parent = NULL;
	}

	if (lconn->_lkcp != NULL)
	{
		ikcp_release(lconn->_lkcp);
		lconn->_lkcp = NULL;
	}

	lconn->_lastrecv = 0;
	lconn->_ntimeout = 0;
}

void kcpconnection_update(ikcpconnection *lconn, uint32_i clock)
{
	uint32_i clnext = ikcp_check(lconn->_lkcp, clock);
	if (clnext == clock)
	{
		ikcp_update(lconn->_lkcp, clock);
	}
}
inline void kcpconnection_recvtts(ikcpconnection *lconn)
{
	lconn->_lastrecv = getts_msec();
	lconn->_ntimeout = 0;
}

void kcpconnection_recv(ikcpconnection *lconn)
{
	//Modified algorithm
	char kcpbuf[1024 * 4] = "";
	int32_i wbytes = 0;
	int32_i kcpbytes_rd = ikcp_recv(lconn->_lkcp, kcpbuf, sizeof(kcpbuf));
	if (kcpbytes_rd <= 0)
		return;
	kcpconnection_recvtts(lconn);
	//The packets are decomposed according to the protocol
	iresolvmsg msg;
	msg._arg = (void *)lconn->_parent;
	msg._connid = (uint32_i)lconn->_conv;
	msg._data = kcpbuf;
	msg._datalen = kcpbytes_rd;
	(*lconn->_parent->_callback._resolve)(msg);
	/*
	explain:Consider only the low intensity packet problem, and solve it in a temporary buffer, regardless of the size of the packet
	*/

	/*while (1)
	{
		if (wbytes == kcpbytes_rd)
			break;
		int32_i wrlen = iocringbuffer_write(lconn, kcpbuf + wbytes, kcpbytes_rd - wbytes, true);
		if (wrlen > 0)
			wbytes += wrlen;
		int32_i result_res = (*lconn->_parent->_resolve)(lconn);
		if (result_res < 0)
			break;
	}*/
}

void kcpconnection_input(ikcpserver *lsrv, uint32_i id, const int32_i revmax, char *udpdata, uint32_i bytes, const iendpoint *udpremote)
{
	ikcpconnection *lconn = (ikcpconnection *)iochashmap_get(lsrv->_connectors, (void *)(int64_i)id);
	if (lconn == NULL)
		return;

	Boolean isbreak = FALSE;
	while (1)
	{
		if (ikcp_waitrev(lconn->_lkcp) >= revmax)
			goto _KCPCONNECTION_INPUT_RECV;
		memcpy((char *)&lconn->_remote, (char *)udpremote, sizeof(lconn->_remote));
		ikcp_input(lconn->_lkcp, udpdata, bytes);
		isbreak = TRUE;
	_KCPCONNECTION_INPUT_RECV:
		kcpconnection_recv(lconn);
		if (isbreak)
			break;
	}
	kcpconnection_subref(lconn);
}

int32_i kcpconnection_send(ikcpconnection *lconn, const int32_i sndmax, char *data, int32_i bytes)
{
	if (ikcp_waitsnd(lconn->_lkcp) > sndmax)
		return -2;
	if (ikcp_send(lconn->_lkcp, data, bytes) == 0)
		return bytes;
	return -1;
}

ikcpserver *kcpserver_create(iocsystem *lsys, const char *addr, const int32_i port, const int32_i sndmax, const int32_i rcvmax)
{
	ikcpserver *lsrv = calloc(1, sizeof(ikcpserver));
	if (lsrv == NULL)
		return NULL;

	lsrv->_system = lsys;
	lsrv->_shutdown = 0;
	lsrv->_sendmax = sndmax;
	lsrv->_recvmax = rcvmax;
	lsrv->_maxlink = IOCKCPSERVER_MAXLINK;
	lsrv->_connectors = iochashmap_create(64, &kcpconnection_hashcode, &kcpconnection_equals, &kcpconnection_addref, &kcpconnection_subref);
	if (lsrv->_connectors == NULL)
		goto _IO_KCPSERVER_CREATE_ERROR;

	if ((lsrv->_udpfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		goto _IO_KCPSERVER_CREATE_ERROR;
	lsrv->_udpep = iocepoll_create(EPOLL_CLOEXEC, 8192, 1024);
	if (lsrv->_udpep == NULL)
		goto _IO_KCPSERVER_CREATE_ERROR;

	strncpy(lsrv->_udpendpoint._addr, addr, IP_ADDR);
	lsrv->_udpendpoint._port = port;

	if (iocnet_setreuseaddr(lsrv->_udpfd) == -1)
		goto _IO_KCPSERVER_CREATE_ERROR;
	if (iocnet_bind(lsrv->_udpfd, lsrv->_udpendpoint) == -1)
		goto _IO_KCPSERVER_CREATE_ERROR;

	iocepoll_add_fd(lsrv->_udpep->_epfd, &lsrv->_udpep->_event, lsrv->_udpfd, EPOLLIN, TRUE);
	return lsrv;

_IO_KCPSERVER_CREATE_ERROR:
	kcpserver_release(lsrv);
	return NULL;
}

inline void kcpserver_bind_callback(ikcpserver *lsrv, ikcpserver_callback *lcallback)
{
	memcpy(&lsrv->_callback, (char *)lcallback, sizeof(ikcpserver_callback));
}

void kcpserver_release(ikcpserver *lsrv)
{
	if (lsrv->_udpfd != 0)
	{
		iocnet_close(lsrv->_udpfd);
		lsrv->_udpfd = 0;
	}
	if (lsrv->_udpep != NULL)
	{
		iocepoll_release(lsrv->_udpep);
		lsrv->_udpep = NULL;
	}
	if (lsrv->_connectors != NULL)
	{
		iochashmap_release(lsrv->_connectors);
		lsrv->_connectors = NULL;
	}

	free(lsrv);
}

void foreach_conn_key(void *key, void *rev)
{
	ioclist *connlist = (ioclist *)rev;
	ioclistnode *node = ioclistnode_create(key);
	if (node == NULL)
	{
		iocsyslog_printf(IOCSYSLOG_FTA, "[foreach_conn_key] create listnode failed!\n");
		return;
	}
	ioclist_pushback(connlist, node);
}

Boolean foreach_conn_stop(ioclistnode *node, void *arg)
{
	ikcpserver *lsrv = (ikcpserver *)arg;
	iochashmap_del(lsrv->_connectors, node->_data);
	return FALSE;
}

void kcpserver_stop(ikcpserver *lsrv)
{
	lsrv->_shutdown = 1;
	if (lsrv->_udpfd != 0)
	{
		iocnet_close(lsrv->_udpfd);
		lsrv->_udpfd = 0;
	}

	//释放所有连接对象
	ioclist *connlist = ioclist_create();
	if (connlist == NULL)
	{
		iocsyslog_printf(IOCSYSLOG_FTA, "[kcpserver_stop] create connlist failed!\n");
		return;
	}
	iochashmap_getkey(lsrv->_connectors, (void *)connlist, foreach_conn_key);
	ioclist_foreach(connlist, (void *)lsrv, foreach_conn_stop);
	ioclist_release(connlist);
}

//---------------------------------------------------------------
//function:
//          kcpserver_wait
//Access:
//           public
//Parameter:
//          [in] const ikcpserver * lsrv -
//Returns:
//          int32_i
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
int32_i kcpserver_wait(ikcpserver *lsrv, int32_i timeout)
{
	if (lsrv->_shutdown)
		return -1;
	int32_i nReady = iocepoll_wait(lsrv->_udpep, timeout);
	if (nReady <= 0)
		return nReady;

	for (int i = 0; i < nReady; ++i)
	{
		//read info
		kcpserver_udprecv(lsrv);
	}
	return nReady;
}

//---------------------------------------------------------------
//function:
//          kcpserver_udprecv
//Access:
//           public
//Parameter:
//          [in] const ikcpserver * lsrv -
//Returns:
//          void -
//Remarks:
//          ...
//author:    Administrator[mirliang]
//--------------------------------------------------------------
void kcpserver_udprecv(ikcpserver *lsrv)
{
	iendpoint clientaddr;
	int32_i ret = iocnet_recvfrom(lsrv->_udpfd, lsrv->_udpdata, UDP_PACKET_MAXLENGTH, &clientaddr);
	if (ret <= 0)
		return;
	if (kcpserver_isconnect(lsrv->_udpdata, ret))
	{
		ikcpaptmsg aptmsg;
		aptmsg._lsrv = lsrv;
		aptmsg._clientaddr = clientaddr;
		(*lsrv->_callback._accept)(aptmsg);
		return;
	}

	kcpserver_udpdata_proc(lsrv, ret, &clientaddr);
}

int32_i kcpserver_accept(ikcpserver *lsrv, iendpoint clientaddr, ikcpconnection **ltconn)
{
	char pakbuf[32];
	//1.
	if (lsrv->_connectors->_size >= lsrv->_maxlink)
	{
		//log
		iocsyslog_printf(IOCSYSLOG_WAR, "[kcpserver_accept] maxlink: %d, curlink: %d.\n", lsrv->_maxlink, lsrv->_connectors->_size);
		return -1;
	}

	//2.create connection
	ikcpconnection *lconn = kcpconnection_create(lsrv, clientaddr);
	if (lconn == NULL)
	{
		//memory error
		iocsyslog_printf(IOCSYSLOG_ERR, "[kcpserver_accept] memory error. remote endpoint: %s:%d\n", clientaddr._addr, clientaddr._port);
		return -2;
	}

	//2.insert map
	if (!iochashmap_put(lsrv->_connectors, (void *)(int64_i)lconn->_conv, lconn))
	{
		kcpconnection_release(lconn);
		//log
		iocsyslog_printf(IOCSYSLOG_ERR, "[kcpserver_accept] put conn into hashmap failed. remote endpoint: %s:%d\n", clientaddr._addr, clientaddr._port);
		return -3;
	}

	//3.get connected back info
	kcpserver_connect_finish_pack(pakbuf, lconn->_conv);

	if (iocnet_sendkeepon(lsrv->_udpfd, pakbuf, IKCPCONNECT_PACKSIZE, &clientaddr) <= 0)
	{
		lconn->_status = IOC_NETCONN_CLOSE;
		//log
		iocsyslog_printf(IOCSYSLOG_ERR, "[kcpserver_accept] iocnet_sendkeepon returns <= 0. remote endpoint: %s:%u\n", clientaddr._addr, clientaddr._port);
		return -4;
	}
	lconn->_status = IOC_NETCONN_NOMAL;
	*ltconn = lconn;
	return 0;
}

void kcpserver_udpdata_proc(ikcpserver *lsrv, const int32_i datalength, const iendpoint *udpremote)
{
	uint32_i conv;
	int ret = ikcp_get_conv(lsrv->_udpdata, datalength, &conv);
	if (ret == 0)
	{
		//这个数据格式错误
		//需要记录错误数据来源地址
		//log
		iocsyslog_printf(IOCSYSLOG_ERR, "[kcpserver_udpdata_proc] ikcp_get_conv returns 0: %s:%u\n", udpremote->_addr, udpremote->_port);
		return;
	}

	//1.获取connection object ref + 1
	ikcpconnection *lpclient = kcpserver_getconnect(lsrv, conv);
	if (lpclient == NULL)
	{
		iocsyslog_printf(IOCSYSLOG_ERR, "[kcpserver_udpdata_proc] kcpserver can't find conn: %d.\n", conv);
		return;
	}
	//2.调用kcp input数据整理
	//3.把数据包分解完成
	//4.input中的lpclient如果要传递给其它线程调用，必须要引计数器++1
	//kcpconnection_input(lpclient, lsrv->_recvmax, lsrv->_udpdata, datalength, udpremote);

	int32_i datapos = 0;
	while (datapos < datalength)
	{
		iockcpdata *kcpdata = (iockcpdata *)malloc(sizeof(iockcpdata));
		if (kcpdata == NULL)
		{
			iocsyslog_printf(IOCSYSLOG_FTA, "[kcpserver_udpdata_proc] malloc iockcpdata error.\n");
			kcpconnection_subref(lpclient);
			return;
		}

		int32_i count = (datalength - datapos) <= sizeof(kcpdata->_data) ? (datalength - datapos) : sizeof(kcpdata->_data);
		memcpy(kcpdata->_data, lsrv->_udpdata + datapos, count);
		kcpdata->_datalen = count;
		datapos += count;
		kcpdata->_arg = (void *)lsrv;
		kcpdata->_conv = lpclient->_conv;
		memcpy(&kcpdata->_udpremote, udpremote, sizeof(iendpoint));
		sendmessage(lsrv->_system, &lpclient->_evtlisten, OC_EVENT_KCPCONN_DATA, (void *)kcpdata);
	}
	kcpconnection_subref(lpclient);
}

void guard_getkey(void *key, void *rev)
{
	ioclist *list = (ioclist *)rev;
	ioclistnode *node = ioclistnode_create(key);
	if (node == NULL)
	{
		iocsyslog_printf(IOCSYSLOG_ERR, "[guard_getkey] create listnode failed.\n");
		return;
	}
	ioclist_pushback(list, node);
}

void kcpserver_guard(ikcpserver *lsrv)
{
	ioclist *list = ioclist_create();
	if (list == NULL)
	{
		iocsyslog_printf(IOCSYSLOG_ERR, "[kcpserver_guard] create list failed.\n");
		return;
	}
	iochashmap_getkey(lsrv->_connectors, (void *)list, guard_getkey);
	uint32_i clock = getclock32();
	ioclistnode *node = ioclist_head(list);
	uint64_i curmsec = getts_msec();
	while (node != NULL)
	{
		void *key = node->_data;
		node = node->_next;
		ikcpconnection *conn = kcpserver_getconnect(lsrv, v_touint32(key));
		if (conn == NULL)
			continue;

		if (conn->_status == IOC_NETCONN_CLOSE)
		{
			iochashmap_del(lsrv->_connectors, key);
			kcpconnection_subref(conn);
			iconnmsg msg;
			msg._arg = (void *)lsrv;
			msg._connid = v_touint32(key);
			(*lsrv->_callback._close)(msg);
			continue;
		}

		if (conn->_status == IOC_NETCONN_NOMAL) //连接验证超时
		{
			if (conn->_lastrecv + IOCNET_HEARTBEAT_TIMEOUT_TIME < curmsec)
			{
				conn->_status = IOC_NETCONN_CLOSE;
				kcpconnection_subref((void *)conn);
				continue;
			}
		}
		else if (conn->_status == IOC_NETCONN_LINK) //心跳检测
		{
			if (conn->_lastrecv + IOCNET_HEARTBEAT_TIMEOUT_TIME < curmsec)
			{
				conn->_lastrecv = curmsec;
				if (++(conn->_ntimeout) >= IOCNET_HEARTBEAT_TIMEOUT_COUNT)
				{
					conn->_status = IOC_NETCONN_CLOSE;
					kcpconnection_subref((void *)conn);
					continue;
				}
			}
		}

		kcpconnection_update(conn, clock);
		kcpconnection_subref((void *)conn);
	}
}

ikcpconnection *kcpserver_getconnect(ikcpserver *lsrv, uint32_i conv)
{
	if (lsrv->_connectors->_size == 0)
		return NULL;
	return iochashmap_get(lsrv->_connectors, (void *)(int64_i)conv);
}

int32_i kcpudpsend(const ikcpserver *lsrv, const iendpoint *udpremote, char *buf, int32_i len)
{
	int32_i nrel = iocnet_sendkeepon(lsrv->_udpfd, (char *)buf, len, udpremote);
	if (nrel <= 0)
	{
#ifndef _DEBUG_
		printf("[WARN] kcp send error Fail Port(%d) IP:(%s) \n", udpremote->_port, udpremote->_addr);
#endif
		return nrel;
	}

	return len;
}
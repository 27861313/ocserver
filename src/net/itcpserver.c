#include "itcpserver.h"
#include <common/iconvert.h>
#include <common/idatetime.h>
#include <common/isyslog.h>
#include <common/ithread.h>

ioctcpconnection *ioctcpconnection_create(itcpserver *tcpser, uint32_i netid)
{
	ioctcpconnection *tcpcon = (ioctcpconnection *)malloc(sizeof(ioctcpconnection));
	if (tcpcon == NULL)
		return NULL;
	tcpcon->_status = IOC_NETCONN_NOMAL;
	tcpcon->_netid = netid;
	tcpcon->_lastrecv = getts();
	tcpcon->_ref = 0;
	tcpcon->_outtimes = 0;
	tcpcon->_rdb = iocringbuffer_create(RINGBUFSIZE);
	tcpcon->_sdb = iocringbuffer_create(RINGBUFSIZE);
	tcpcon->_sque = iocqueue_create(IOCTCP_QUEUESIZE, NULL);
	return tcpcon;
}

void ioctcpconnection_release(ioctcpconnection *tcpcon)
{
	if (tcpcon->_netid > 0)
		iocnet_close(tcpcon->_netid);
	iocringbuffer_release(tcpcon->_rdb);
	iocringbuffer_release(tcpcon->_sdb);
	iocsenddata *p = (iocsenddata *)iocqueue_pop(tcpcon->_sque);
	while (p != NULL)
	{
		free(p->_pdata);
		free(p);
		p = (iocsenddata *)iocqueue_pop(tcpcon->_sque);
	}
	iocqueue_release(tcpcon->_sque);
	free(tcpcon);
}

int32_i ioctcpconnection_push(ioctcpconnection *tcpcon, const char *pdata, uint32_i len)
{
	iocsenddata *psdata = (iocsenddata *)malloc(sizeof(iocsenddata));
	psdata->_pdata = (char *)malloc(sizeof(char) * len);
	memcpy(psdata->_pdata, pdata, len);
	psdata->_len = len;
	if (iocqueue_push(tcpcon->_sque, (void *)psdata) == TRUE)
		return len;
	// 失败直接丢弃
	free(psdata->_pdata);
	free(psdata);
	return 0;
}

int32_i ioctcpconnection_dosend(ioctcpconnection *tcpcon)
{
	uint32_i dsize = iocringbuffer_bytes(tcpcon->_sdb);
	while (dsize > 0)
	{
		int32_i n = iocnet_send(tcpcon->_netid, iocringbuffer_getreadpos(tcpcon->_sdb), dsize);
		if (n > 0) // 发送成功
		{
			iocringbuffer_readbyted(tcpcon->_sdb, n);
			dsize -= n;
		}
		else if (n == 0) // 断开
		{
			tcpcon->_status = IOC_NETCONN_CLOSE;
			return -1;
		}
		else if (n == -1 && errno == EAGAIN) // 缓冲区满
		{
			return -2;
		}
		else // 错误
		{
			return -3;
		}
	}

	return 0;
}

int32_i ioctcpconnection_send(ioctcpconnection *tcpcon)
{
	do
	{
		int32_i ret = ioctcpconnection_dosend(tcpcon);
		if (ret != 0)
			return ret;
		iocsenddata *d = (iocsenddata *)iocqueue_pop(tcpcon->_sque);
		if (d == NULL)
			return 0;
		iocringbuffer_write(tcpcon->_sdb, d->_pdata, d->_len, TRUE);
		free(d->_pdata);
		free(d);
	} while (1);
}

int32_i ioctcpconnection_read(ioctcpconnection *tcpcon, resolve_proc splitcall)
{
	// to do  协议检测
	tcpcon->_outtimes = 0;		 // todo  协议检测通过再做更新
	tcpcon->_lastrecv = getts(); // 更新计时

	uint32_i ail = 0;
	int32_i readnum = 0;
	do
	{
		ail = iocringbuffer_remainbytes(tcpcon->_rdb);
		readnum = iocnet_recv(tcpcon->_netid, iocringbuffer_getwritepos(tcpcon->_rdb), ail);
		if (readnum == 0) // 客户端关闭fd
		{
			tcpcon->_status = IOC_NETCONN_CLOSE;
			return -1;
		}
		if (readnum > 0)
		{
			iocringbuffer_writebyted(tcpcon->_rdb, readnum);
			do
			{
				iresolvmsg msg;
				msg._arg = (void *)tcpcon;
				msg._connid = 0;
				msg._data = iocringbuffer_getreadpos(tcpcon->_rdb);
				msg._datalen = (int32_i)iocringbuffer_bytes(tcpcon->_rdb);

				int32_i rn = (*splitcall)(msg); // 拆包
				if (rn < 0)
					break;
				iocringbuffer_readbyted(tcpcon->_rdb, rn);
			} while (1);
		}
	} while (readnum != -1);
	return 0;
}

void ioctcpconnection_checklastrecv(ioctcpconnection *tcpcon, uint64_i nowtime)
{
	if (tcpcon->_status == IOC_NETCONN_NOMAL)
	{
		if ((nowtime - tcpcon->_lastrecv) > IOCNET_HEARTBEAT_TIMEOUT_TIME)
		{
			//tcpcon->_status = IOC_NETCONN_CLOSE;
		}
	}
	else if (tcpcon->_status == IOC_NETCONN_LINK)
	{
		if ((nowtime - tcpcon->_lastrecv) > IOCNET_HEARTBEAT_TIMEOUT_TIME)
		{
			tcpcon->_lastrecv = nowtime;
			if (++(tcpcon->_outtimes) >= IOCNET_HEARTBEAT_TIMEOUT_COUNT) // 超过次数
			{
				//tcpcon->_status = IOC_NETCONN_CLOSE;
			}
			// to do:  send msg to client
		}
	}
}

uint32_i itcpserver_hashcode(void *key)
{
	return v_touint32(key);
}

int32_i itcpserver_equals(void *key1, void *key2)
{
	uint32_i k1 = v_touint32(key1);
	uint32_i k2 = v_touint32(key2);
	if (k1 > k2)
	{
		return 1;
	}
	else if (k1 < k2)
	{
		return -1;
	}
	return 0;
}

void itcpserver_addref(void *val)
{
	ioctcpconnection *tcpcon = (ioctcpconnection *)val;
	AtomicAddFetch(&(tcpcon->_ref), 1);
}

void itcpserver_delref(void *val)
{
	ioctcpconnection *tcpcon = (ioctcpconnection *)val;
	if (AtomicSubFetch(&(tcpcon->_ref), 1) == 0)
		ioctcpconnection_release(tcpcon);
}

ioctcpconnection *itcpserver_getconntion(itcpserver *tcpser, uint32_i key)
{
	return (ioctcpconnection *)(iochashmap_get(tcpser->_netconhashmap, (void *)(int64_i)key));
}

/*
void itcpserver_send_delay(itcpserver* tcpser, uint32_i connfd)
{

}
*/

void *itcpserver_eventcallbak(uint16_i evtid, void *arg)
{
	ioctcpsystemsendmsg *msg = (ioctcpsystemsendmsg *)arg;
	if (evtid == OC_TCPNET_SEND) // 发送事件
	{
		if (itcpserver_dosend(msg->_tcpser, msg->_connfd) == -2)
			; //-2重发
				//	itcpserver_send_delay(msg->_tcpser, msg->_connfd);
				//	todo
	}
	else if (evtid == OC_TCPNET_RECV)
	{
		itcpserver_read(msg->_tcpser, msg->_connfd);
	}
	free(msg);
}

itcpserver *itcpserver_create(iocsystem *lsys, unsigned short int port, const char *ip, uint32_i linkmax, itcpserver_callback *lcallback)
{
	itcpserver *tcpser = (itcpserver *)malloc(sizeof(itcpserver));
	if (tcpser == NULL)
		return NULL;
	tcpser->_tcpfd = 0;
	tcpser->_shutdown = 0;
	tcpser->_linkmax = linkmax;
	tcpser->_system = lsys;
	memcpy(&tcpser->_callback, lcallback, sizeof(itcpserver_callback));

	if (iocevlistener_create(tcpser->_system, &tcpser->_evtlisten, TRUE, TRUE) != TRUE)
		goto _itcpserver_START_ERROR;
	iocevlistener_register(&(tcpser->_evtlisten), NULL, OC_TCPNET_SEND, &itcpserver_eventcallbak);
	iocevlistener_register(&(tcpser->_evtlisten), NULL, OC_TCPNET_RECV, &itcpserver_eventcallbak);
	// 创建connection的hashmap
	tcpser->_netconhashmap = iochashmap_create(HASHMAPSIZE, itcpserver_hashcode, itcpserver_equals, itcpserver_addref, itcpserver_delref);
	if (tcpser->_netconhashmap == NULL)
		goto _itcpserver_START_ERROR;
	// 创建fd
	tcpser->_tcpfd = ioctcpfd_create();
	if (tcpser->_tcpfd <= 0)
		goto _itcpserver_START_ERROR;
	// 绑定
	if (iocnet_tcpbind(tcpser, ip, port) == -1)
		goto _itcpserver_START_ERROR;
	// 监听
	if (iocnet_listen(tcpser->_tcpfd, 48) != 0)
		goto _itcpserver_START_ERROR;
	// 创建epoll
	tcpser->_tcpep = iocepoll_create(EPOLL_CLOEXEC, 8192, 1024);
	if (tcpser->_tcpep == NULL)
		goto _itcpserver_START_ERROR;
	//加入epoll
	iocepoll_add_fd(tcpser->_tcpep->_epfd, tcpser->_tcpep->_evts, tcpser->_tcpfd, EPOLLIN /*水平触发*/, FALSE);
	return tcpser;
_itcpserver_START_ERROR:
	free(tcpser);
	if (tcpser->_netconhashmap == NULL)
		iochashmap_release(tcpser->_netconhashmap);
	if (tcpser->_tcpfd > 0)
		iocnet_close(tcpser->_tcpfd);
	return NULL;
}

int32_i itcpserver_accept(itcpserver *tcpser, iendpoint *addr)
{
	int32_i connfd = iocnet_accept(tcpser->_tcpfd, addr);
	if (connfd == -1)
		return -1;
	if (tcpser->_netconhashmap->_size >= tcpser->_linkmax)
	{
		iocnet_close(connfd);
		iocsyslog_printf(IOCSYSLOG_WAR, "[tcpser_accept] more max\n");
		return -1;
	}

	int32_i flags = fcntl(connfd, F_GETFL, 0); // 非阻塞
	fcntl(connfd, F_SETFL, flags | O_NONBLOCK);
	ioctcpconnection *tcpcon = ioctcpconnection_create(tcpser, connfd);
	if (tcpcon == NULL)
	{
		iocnet_close(connfd);
		iocsyslog_printf(IOCSYSLOG_WAR, "[tcpser_accept] create connector faild\n");
		return -1;
	}
	iochashmap_put(tcpser->_netconhashmap, (void *)(int64_i)connfd, (void *)tcpcon);
	iocepoll_add_fd(tcpser->_tcpep->_epfd, tcpser->_tcpep->_evts, connfd, EPOLLIN | EPOLLET /*边缘触发*/, FALSE);
	return connfd;
}

void itcpserver_read(itcpserver *tcpser, uint32_i connfd)
{
	ioctcpconnection *tcpcon = itcpserver_getconntion(tcpser, connfd);
	if (tcpcon == NULL)
		return;
	if (ioctcpconnection_read(tcpcon, tcpser->_callback._split) == -1)
		tcpcon->_status = IOC_NETCONN_CLOSE; // 客户端关闭
	itcpserver_delref((void *)tcpcon);		 // 减引用计数
	return;
}

void itcpserver_answer(itcpserver *tcpser, uint32_i esize)
{
	uint32_i nrequest = 0;
	for (uint32_i i = 0; i < esize; ++i)
	{
		if (tcpser->_tcpep->_evts[i].data.fd == tcpser->_tcpfd)
			++nrequest;
		else
		{
			iconnmsg rdmsg;
			rdmsg._arg = tcpser;
			rdmsg._connid = tcpser->_tcpep->_evts[i].data.fd;
			(*tcpser->_callback._read)(rdmsg);
		}
	}

	if (nrequest == 0)
		return;
	itcpaptmsg aptmsg;
	aptmsg._lsrv = tcpser;
	aptmsg._irequest = nrequest;
	(*tcpser->_callback._accept)(aptmsg);
}

int32_i itcpserver_wait(itcpserver *tcpser, int32_i timeout)
{
	if (tcpser->_shutdown == 1)
		return -1;
	int32_i esize = iocepoll_wait(tcpser->_tcpep, timeout);
	if (esize > 0)
		itcpserver_answer(tcpser, esize);
	return esize;
}

Boolean itcpserver_loginsuccess(itcpserver *tcpser, uint32_i connfd)
{
	ioctcpconnection *tcpcon = itcpserver_getconntion(tcpser, connfd);
	if (tcpcon == NULL)
		return FALSE;
	tcpcon->_status = IOC_NETCONN_LINK;
	itcpserver_delref((void *)tcpcon); // 减引用计数
	return TRUE;
}

int32_i itcpserver_send(itcpserver *tcpser, uint32_i connfd, char *pdata, uint32_i len)
{
	ioctcpconnection *tcpcon = itcpserver_getconntion(tcpser, connfd);
	if (tcpcon == NULL)
		return -1;
	int32_i pushnum = ioctcpconnection_push(tcpcon, pdata, len);
	itcpserver_delref((void *)tcpcon); // 减引用计数

	ioctcpsystemsendmsg *tcpsysmsg = (ioctcpsystemsendmsg *)malloc(sizeof(ioctcpsystemsendmsg));
	if (tcpsysmsg == NULL)
		return pushnum;
	tcpsysmsg->_tcpser = tcpser;
	tcpsysmsg->_connfd = tcpcon->_netid;
	sendmessage(tcpser->_system, &(tcpser->_evtlisten), OC_TCPNET_SEND, (void *)tcpsysmsg);
	return pushnum;
}

int32_i itcpserver_dosend(itcpserver *tcpser, uint32_i connfd)
{
	ioctcpconnection *tcpcon = itcpserver_getconntion(tcpser, connfd);
	if (tcpcon == NULL)
		return -3;
	if (tcpcon->_status == IOC_NETCONN_CLOSE)
		return -1;
	int32_i ret = ioctcpconnection_send(tcpcon);
	itcpserver_delref((void *)tcpcon); // 减引用计数
	return ret;
}

void itcpserver_key_callback(void *key, void *rev)
{
	ioclist *connlist = (ioclist *)rev;
	ioclistnode *node = ioclistnode_create(key);
	if (node == NULL)
	{
		iocsyslog_printf(IOCSYSLOG_FTA, "[foreach_tcp_key] create listnode failed!\n");
		return;
	}
	ioclist_pushback(connlist, node);
	return;
}

Boolean itcpserver_guard_callback(ioclistnode *node, void *arg)
{
	itcpserver *tcpser = (itcpserver *)arg;
	ioctcpconnection *tcpcon = itcpserver_getconntion(tcpser, v_touint32(node->_data));
	if (tcpcon == NULL)
		return FALSE;
	uint64_i nowtime = getts();
	if (tcpcon->_status == IOC_NETCONN_CLOSE) // 清理
	{
		uint32_i netid = tcpcon->_netid;
		iochashmap_del(tcpser->_netconhashmap, (void *)(int64_i)tcpcon->_netid);
		iconnmsg closemsg;
		closemsg._arg = tcpser;
		closemsg._connid = netid;
		(*tcpser->_callback._close)(closemsg);
	}
	else // 发包间隔检测
		ioctcpconnection_checklastrecv(tcpcon, nowtime);
	itcpserver_delref((void *)tcpcon); // 减引用计数
	return FALSE;
}

int32_i itcpserver_guard(itcpserver *tcpser)
{
	ioclist *connlist = ioclist_create();
	if (connlist == NULL)
		return -1;
	iochashmap_getkey(tcpser->_netconhashmap, (void *)connlist, itcpserver_key_callback);
	ioclist_foreach(connlist, (void *)tcpser, itcpserver_guard_callback);
	ioclist_clear(connlist);
	return 0;
}

Boolean itcpserver_stop_callback(ioclistnode *node, void *arg)
{
	itcpserver *tcpser = (itcpserver *)arg;
	ioctcpconnection *tcpcon = (ioctcpconnection *)iochashmap_get(tcpser->_netconhashmap, node->_data);
	if (tcpcon != NULL)
		tcpcon->_status = IOC_NETCONN_CLOSE;
	return TRUE;
}

void itcpserver_stop(itcpserver *tcpser)
{
	if (tcpser->_shutdown == 1)
		return;
	tcpser->_shutdown = 1;
	iocnet_close(tcpser->_tcpfd);
	ioclist *connlist = ioclist_create();
	if (connlist == NULL)
	{
		tcpser->_shutdown = 0;
		iocsyslog_printf(IOCSYSLOG_FTA, "[itcpserver_stop] create connlist failed\n");
		return;
	}

	iochashmap_getkey(tcpser->_netconhashmap, (void *)connlist, itcpserver_key_callback);
	ioclist_foreach(connlist, (void *)tcpser, itcpserver_stop_callback);
	ioclist_release(connlist);
	iocevlistener_release(tcpser->_system, &(tcpser->_evtlisten));
}

#include "itcpclient.h"
#include "inet.h"
#include <arpa/inet.h>
#include <common/iatom.h>
#include <common/iconvert.h>
#include <common/isyslog.h>

void ioctcpclientconnection_release(ioctcpclientconnection *tcc);

void itcpclient_send_delay(ioctcpclient *tc, uint32_i connfd);

uint32_i ioctcpclient_hashcode(void *key)
{
    return v_touint32(key);
}

int32_i ioctcpclient_equals(void *key1, void *key2)
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

void ioctcpclient_addref(void *val)
{
    ioctcpclientconnection *tcc = (ioctcpclientconnection *)val;
    AtomicAddFetch(&(tcc->_ref), 1);
}

void ioctcpclient_delref(void *val)
{
    ioctcpclientconnection *tcc = (ioctcpclientconnection *)val;
    if (AtomicSubFetch(&(tcc->_ref), 1) == 0)
    {
        ioctcpclientconnection_release(tcc);
    }
}

void ioctcpclientconnection_release(ioctcpclientconnection *tcc)
{
    if (tcc->_netid > 0)
        iocnet_close(tcc->_netid);
    iocringbuffer_release(tcc->_rdb);
    iocringbuffer_release(tcc->_sdb);

    iocsenddata *p = (iocsenddata *)iocqueue_pop(tcc->_sque);
    while (p != NULL)
    {
        free(p->_pdata);
        free(p);
        p = (iocsenddata *)iocqueue_pop(tcc->_sque);
    }

    iocqueue_release(tcc->_sque);
    /*
		if (tcc->_parent != NULL)
				iocevlistener_release(tcc->_parent->_system, &tcc->_evtlisten);
				*/
    //tcc->_parent = NULL;
    free(tcc);
}

int32_i ioctcpclientconnection_dosend(ioctcpclientconnection *tcc)
{
    uint32_i datasize = iocringbuffer_bytes(tcc->_sdb);
    while (datasize > 0)
    {
        int32_i n = iocnet_send(tcc->_netid, iocringbuffer_getreadpos(tcc->_sdb), datasize);

        if (n > 0) // 发送成功
        {
            iocringbuffer_readbyted(tcc->_sdb, n);
            datasize -= n;
        }
        else if (n == 0) // 断开
        {
            tcc->_status = IOC_NETCONN_CLOSE;
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

int32_i ioctcpclientconnection_send(ioctcpclientconnection *tcc)
{
    do
    {
        if (tcc->_status == IOC_NETCONN_CLOSE)
            return -1;
        int32_i ret = ioctcpclientconnection_dosend(tcc);
        if (ret != 0)
            return ret;

        iocsenddata *d = (iocsenddata *)iocqueue_pop(tcc->_sque);
        if (d == NULL)
            return 0;
        iocringbuffer_write(tcc->_sdb, d->_pdata, d->_len, TRUE);

        free(d->_pdata);
        free(d);
    } while (1);
}

int32_i ioctcpclientconnection_push(ioctcpclientconnection *tcc, char *pdata, uint32_i len)
{
    iocsenddata *psdata = (iocsenddata *)malloc(sizeof(iocsenddata));
    psdata->_pdata = (char *)malloc(sizeof(char) * len);
    memcpy(psdata->_pdata, pdata, len);
    psdata->_len = len;

    if (iocqueue_push(tcc->_sque, (void *)psdata) == TRUE)
    {
        return len;
    }
    else // 失败直接丢弃
    {
        free(psdata->_pdata);
        free(psdata);
        return 0;
    }
}

ioctcpclientconnection *ioctcpclientconnection_create(const char *ip, unsigned short int port)
{
    ioctcpclientconnection *conn = (ioctcpclientconnection *)malloc(sizeof(ioctcpclientconnection));
    if (conn == NULL)
        return NULL;

    conn->_status = IOC_NETCONN_NOMAL;
    conn->_rdb = iocringbuffer_create(RINGBUFSIZE);
    conn->_sdb = iocringbuffer_create(RINGBUFSIZE);
    conn->_sque = iocqueue_create(IOCTCP_QUEUESIZE, NULL);

    conn->_netid = ioctcpfd_create();
    if (conn->_netid <= 0)
        goto _ICPCLIENT_CONNECT_ERROR;

    if (iocnet_setnonblock(conn->_netid) != 0)
        goto _ICPCLIENT_CONNECT_ERROR;

    //select 判断超时

    int ret = iocnet_connect(conn->_netid, ip, port);
    if (ret < 0 && errno != EINPROGRESS)
        goto _ICPCLIENT_CONNECT_ERROR;
    if (ret == 0)
        return conn;
    int32_i times = 0;
    fd_set rfds, wfds;

    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    FD_SET(conn->_netid, &rfds);
    FD_SET(conn->_netid, &wfds);

    tv.tv_sec = 10;
    tv.tv_sec = 0;
    int32_i selres = select(conn->_netid + 1, &rfds, &wfds, NULL, &tv);
    if (selres == -1 || selres == 0)
        goto _ICPCLIENT_CONNECT_ERROR;
    if (!FD_ISSET(conn->_netid, &rfds) && !FD_ISSET(conn->_netid, &rfds))
        goto _ICPCLIENT_CONNECT_ERROR;
    int32_i err, len;
    if (getsockopt(conn->_netid, SOL_SOCKET, SO_ERROR, &err, &len) < 0)
        goto _ICPCLIENT_CONNECT_ERROR;

    return conn;

_ICPCLIENT_CONNECT_ERROR:
    iocringbuffer_release(conn->_rdb);
    iocringbuffer_release(conn->_sdb);
    iocqueue_release(conn->_sque);
    free(conn);
    return NULL;
}

int32_i ioctcpclientconnection_read(ioctcpclientconnection *tcc, resolve_proc splitcall)
{

    //tcpcon->_outtimes = 0;      // todo  协议检测通过再做更新
    //tcpcon->_lastrecv= getts(); // 更新计时

    uint32_i ail = 0;
    int32_i readnum = 0;
    do
    {
        ail = iocringbuffer_remainbytes(tcc->_rdb);
        readnum = iocnet_recv(tcc->_netid, iocringbuffer_getwritepos(tcc->_rdb), ail);
        if (readnum == 0) // 客户端关闭fd
        {
            tcc->_status = IOC_NETCONN_CLOSE;
            return -1;
        }
        if (readnum > 0)
        {
            iocringbuffer_writebyted(tcc->_rdb, readnum);
            do
            {
                iresolvmsg msg;
                msg._arg = (void *)tcc;
                msg._connid = 0;
                msg._data = iocringbuffer_getreadpos(tcc->_rdb);
                msg._datalen = (int32_i)iocringbuffer_bytes(tcc->_rdb);

                int32_i rn = (*splitcall)(msg); // 拆包
                if (rn < 0)
                    break;
                iocringbuffer_readbyted(tcc->_rdb, rn);
            } while (1);
        }

    } while (readnum != -1);
    return 0;
}

ioctcpclientconnection *ioctcpclient_getconntion(ioctcpclient *tc, uint32_i key)
{
    return (ioctcpclientconnection *)(iochashmap_get(tc->_netconhashmap, (void *)(int64_i)key));
}

ioctcpclient *ioctcpclient_create(iocsystem *lsys, itcpserver_callback *lcallback)
{
    ioctcpclient *tc = (ioctcpclient *)malloc(sizeof(ioctcpclient));
    if (tc == NULL)
        return NULL;

    tc->_shutdown = 0;
    tc->_system = lsys;
    tc->_tcpep = iocepoll_create(EPOLL_CLOEXEC, 8192, 1024);
    tc->_netconhashmap = iochashmap_create(HASHMAPSIZE, ioctcpclient_hashcode, ioctcpclient_equals, ioctcpclient_addref, ioctcpclient_delref);
    if (tc->_netconhashmap != NULL)
        return tc;
    return NULL;
}

ioctcpclientconnection *ioctcpclient_connect(ioctcpclient *tc, const char *ip, unsigned short int port)
{
    ioctcpclientconnection *tcc = ioctcpclientconnection_create(ip, port);
    if (tcc != NULL)
        iochashmap_put(tc->_netconhashmap, (void *)(int64_i)(tcc->_netid), (void *)tcc);
    return tcc;
}

int32_i ioctcpclient_dosend(ioctcpclient *tc, uint32_i connfd, void (*itcpclient_send_delay)(ioctcpclient *tc, uint32_i connfd))
{
    ioctcpclientconnection *tcc = ioctcpclient_getconntion(tc, connfd);
    if (tcc == NULL)
        return -1;

    int32_i ret = ioctcpclientconnection_send(tcc);
    if (ret == -2)
        itcpclient_send_delay(tc, connfd);

    ioctcpclient_delref((void *)tcc); // 减引用计数
    return ret;
}

int32_i ioctcpclient_send(ioctcpclient *tc, uint32_i connfd, char *pdata, uint32_i len)
{
    ioctcpclientconnection *tcc = ioctcpclient_getconntion(tc, connfd);
    if (tcc == NULL)
        return -1;

    int32_i pushnum = ioctcpclientconnection_push(tcc, pdata, len);
    ioctcpclient_delref((void *)tcc); // 减引用计数

    //sendmessage(tc->_system, &(tcc->_evtlisten), OC_TCPNET_SEND, (void *)(int64_i)connfd); // 交给system去处理
    return pushnum;
}

void ioctcpclient_read(ioctcpclient *tc, uint32_i connfd)
{
    ioctcpclientconnection *tcc = ioctcpclient_getconntion(tc, connfd);
    if (tcc == NULL)
        return;

    if (ioctcpclientconnection_read(tcc, tc->_callback._split) == -1)
        tcc->_status = IOC_NETCONN_CLOSE; // 客户端关闭

    ioctcpclient_delref((void *)tcc); // 减引用计数
    return;
}

void ioctcpclient_key_callback(void *key, void *rev)
{
    ioclist *connlist = (ioclist *)rev;
    ioclistnode *node = ioclistnode_create(key);
    if (node == NULL)
    {
        iocsyslog_printf(IOCSYSLOG_FTA, "[foreach_tcpclient_key] create listnode failed!\n");
        return;
    }
    ioclist_pushback(connlist, node);
    return;
}

Boolean ioctcpclient_guard_callback(ioclistnode *node, void *arg)
{
    ioctcpclient *tc = (ioctcpclient *)arg;
    ioctcpclientconnection *tcc = ioctcpclient_getconntion(tc, v_touint32(node->_data));
    if (tcc == NULL)
        return FALSE;

    //uint64_i nowtime = getts();
    if (tcc->_status == IOC_NETCONN_CLOSE) // 清理
    {
        uint32_i netid = tcc->_netid;
        iochashmap_del(tc->_netconhashmap, node->_data);
        /*
        iconnmsg closemsg;
        closemsg._arg = tcpser;
        closemsg._connid = netid;

        (*tcpser->_callback._close)(closemsg);
*/
    }

    ioctcpclient_delref((void *)tcc); // 减引用计数
    return FALSE;
}

int32_i ioctcpclient_guard(ioctcpclient *tc)
{
    ioclist *connlist = ioclist_create();
    if (connlist == NULL)
        return -1;

    iochashmap_getkey(tc->_netconhashmap, (void *)connlist, ioctcpclient_key_callback);
    ioclist_foreach(connlist, (void *)tc, ioctcpclient_guard_callback);
    ioclist_clear(connlist);
    return 0;
}

Boolean ioctcpclient_stop_callback(ioclistnode *node, void *arg)
{
    ioctcpclient *tc = (ioctcpclient *)arg;
    ioctcpclientconnection *tcpcon = (ioctcpclientconnection *)iochashmap_get(tc->_netconhashmap, node->_data);
    if (tcpcon == NULL)
        return FALSE;
    tcpcon->_status = IOC_NETCONN_CLOSE;
    return TRUE;
}

void ioctcpclient_stop(ioctcpclient *tc)
{
    if (tc->_shutdown == 1)
        return;
    tc->_shutdown = 1;
    //iocnet_close(tc->_tcpfd);

    ioclist *connlist = ioclist_create();
    if (connlist == NULL)
    {
        tc->_shutdown = 0;
        iocsyslog_printf(IOCSYSLOG_FTA, "[itcpserver_stop] create connlist failed\n");
        return;
    }
    iochashmap_getkey(tc->_netconhashmap, (void *)connlist, ioctcpclient_key_callback);
    ioclist_foreach(connlist, (void *)tc, ioctcpclient_stop_callback);
    ioclist_release(connlist);
}

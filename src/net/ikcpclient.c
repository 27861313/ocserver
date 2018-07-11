#include "ikcpclient.h"
#include "ikcpproto.h"
#include <common/ithread.h>
#include <common/iatom.h>
#include <common/ilist.h>
#include <common/idatetime.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <common/isyslog.h>
#include "ikcpintrinsic.inc"

Boolean is_conn_finished(char *buffer, int32_i len, int32_i *conv)
{
    if (len != IKCPCONNECT_PACKSIZE)
    {
        return FALSE;
    }
    if ( (unsigned char)buffer[0] != IKCPCONNECT_START_BIT ||
         (unsigned char)buffer[1] != IKCPCONNECT_SECOND_BIT ||
         (unsigned char)buffer[7] != IKCPCONNECT_END_BIT)
            return FALSE;
    
    memcpy((char *)conv, buffer + 2, IKCPCONNECT_DATA_SIZE);
    return TRUE;
}

int32_i udp_send(iockcpclientconn* lconn, const char* data, int32_i len)
{
    if (lconn->_sckfd == -1)
    {
        return -1;
    }
    return iocnet_sendkeepon(lconn->_sckfd, (void *)data, len, &lconn->_remote);
}

int32_i on_udp_output(const char* buf, int len, ikcpcb* kcpcb, void *ud)
{
    iockcpclientconn *lconn = (iockcpclientconn *)ud;
    iockcpclient* lcli = lconn->_parent;
    return udp_send(lconn, buf, len);
}

void on_receive(iockcpclient* lcli, int32_i fd, char *buffer, int32_i len)
{
    iockcpclientconn *lconn = (iockcpclientconn *)iochashmap_get(lcli->_conns, (void *)((int64_i)fd));
    if (lconn == NULL)
    {
        return;
    }

    int32_i conv = 0;
    if (is_conn_finished(buffer, len, &conv))
    {
        // 连接成功回馈包
        lconn->_status = IOC_NETCONN_LINK;
        // 创建KCP对象
        lconn->_kcpcb = ikcp_create(conv, (void *)lconn);
        ikcp_nodelay(lconn->_kcpcb, 1, 5, 1, 1);
        lconn->_kcpcb->output = on_udp_output;
        // 连接成功回调
        iconnmsg msg;
        msg._arg = (void *)lcli;
        msg._connid = (uint32_i)lconn->_sckfd;
        lconn->_conncb(msg);
        return;
    }
    // 回调
    iresolvmsg msg;
    msg._arg = (void *)lcli;
    msg._connid = (uint32_i)lconn->_sckfd;
    msg._data = buffer;
    msg._datalen = len;
    lconn->_resolvecb(msg);
}

void client_thrd_getkey(void* key, void* rev)
{
    ioclist* list = (ioclist *)rev;
    ioclistnode* node = ioclistnode_create(key);
    ioclist_pushback(list, node);
}

void * client_thread_recv(void *arg)
{
    iockcpclient* lcli = (iockcpclient *)arg;
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 100 * 1000;

    while (!lcli->_shutdown)
    {
        fd_set rdfds;
        FD_ZERO(&rdfds);
        ioclist *list = ioclist_create();
        if (list == NULL) continue;
        iochashmap_getkey(lcli->_conns, (void *)list, client_thrd_getkey);
        ioclistnode* node = ioclist_head(list);
        while (node != NULL)
        {
            FD_SET(v_touint32(node->_data), &rdfds);
            node = node->_next;
        }
        ioclist_release(list);
        int32_i ret = select(lcli->_maxfd + 1, &rdfds, NULL, NULL, &timeout);
        if (ret == -1)
        {
            iocsyslog_printf(IOCSYSLOG_ERR, "[kcpclient_thread_recv] select io error.\n");
            break;
        }
        else if (ret == 0)
        {
            // timeout
            continue;
        }
        char recvbuf[UDP_PACKET_MAXLENGTH] = { 0 };
        for (int i = 0; i < lcli->_maxfd + 1; ++i)
        {
            if (FD_ISSET(i, &rdfds))
            {
                iendpoint ep;
                int n = iocnet_recvfrom(i, recvbuf, sizeof(recvbuf), &ep);
                if (n < 0)
                {
                    iocsyslog_printf(IOCSYSLOG_ERR, "[kcpclient_thread_recv] iocnet_recvfrom socket error: %d\n", n);
                    continue;
                }
                else if (n == 0)
                {

                }
                else
                {
                    on_receive(lcli, i, recvbuf, n);
                }
            }
        }
    }
    return NULL;
}

void cli_getkey(void* key, void* rev)
{
    ioclist* lst = (ioclist *)rev;
    ioclistnode* node = ioclistnode_create(key);
    if (node == NULL) 
    {
        iocsyslog_printf(IOCSYSLOG_FTA, "kcpclient cli_getkey create listnode error.\n");
        return;
    }
    ioclist_pushback(lst, node);
}

void * client_thread_update(void *arg)
{
    iockcpclient* lcli = (iockcpclient *)arg;
    while (!lcli->_shutdown)
    {
        ISLEEP(50);
        uint32_i msec = getclock32();
        ioclist* lst = ioclist_create();
        if (lst == NULL) continue;
        iochashmap_getkey(lcli->_conns, (void *)lst, cli_getkey);
        ioclistnode *node = ioclist_head(lst);
        while(node != NULL)
        {
            void *key = node->_data;
            node = node->_next;
            iockcpclientconn *lconn = (iockcpclientconn *)iochashmap_get(lcli->_conns, key);
            if (lconn == NULL)
            {
                continue;
            }
            if (lconn->_status == IOC_NETCONN_CLOSE)
            {
                iochashmap_del(lcli->_conns, key);
            }
            else
            {
                if (lconn->_kcpcb != NULL)
                {
                    if (ikcp_check(lconn->_kcpcb, msec) == msec)
                    {
                        ikcp_update(lconn->_kcpcb, msec);
                    }
                }
            }
            iockcpclientconn_subref(lconn);
            
        }
        ioclist_release(lst);
    }
}

uint32_i kcpcli_hashcode(void *key)
{
    if (key == NULL)
		return 0;
	return v_touint32(key);
}

int32_i kcpcli_equals(void *key1, void *key2)
{
	uint32_i k1 = 0;
    if(key1 != NULL)	
	    k1 = v_touint32(key1);
	uint32_i k2 = 0;
    if(key2 != NULL)	
		k2 = v_touint32(key2);

	if (k1 > k2)
		return 1;
	else if (k1 < k2)
		return -1;
	else
		return 0;
}

void iockcpclientconn_addref(void *arg)
{
	iockcpclientconn *lconn = (iockcpclientconn *)arg;
	AtomicAdd(&lconn->_ref, 1);
}

void iockcpclientconn_subref(void *arg)
{
	iockcpclientconn *lconn = (iockcpclientconn *)arg;
	if (AtomicSubFetch(&lconn->_ref, 1) == 0)
	{
		iockcpclientconn_release(lconn);
	}
}

iockcpclient* iockcpclient_create()
{
    iockcpclient* lcli = (iockcpclient *)malloc(sizeof(iockcpclient));
    if (lcli == NULL) 
    {
        return NULL;
    }
    lcli->_shutdown = 0;
    FD_ZERO(&lcli->_fdset);
    lcli->_maxfd = 0;
    lcli->_conns = iochashmap_create(64, kcpcli_hashcode, kcpcli_equals, iockcpclientconn_addref, iockcpclientconn_subref);
    if (lcli->_conns == NULL)
    {
        free(lcli);
        return NULL;
    }
    // 创建接收线程
    iocthread_run(iocthread_create(NULL, client_thread_recv, (void *)lcli));
    iocthread_run(iocthread_create(NULL, client_thread_update, (void *)lcli));
    return lcli;
}

void iockcpclient_stop(iockcpclient* lcli)
{
    lcli->_shutdown = 1;
}

void cli_release(void *key, void *rev)
{
    ioclist *list = (ioclist *)list;
    ioclistnode *node = ioclistnode_create(key);
    if (node == NULL)
    {
        iocsyslog_printf(IOCSYSLOG_FTA, "kcpclient cli_release create listnode error.\n");
        return;
    }
    ioclist_pushback(list, node);
}

void iockcpclient_release(iockcpclient* lcli)
{
    if (lcli->_conns)
    {
        ioclist *list = ioclist_create();
        if (list == NULL)
        {
            iocsyslog_printf(IOCSYSLOG_FTA, "[iockcpclient_release] create list error.\n");
            return;
        }
        iochashmap_getkey(lcli->_conns, (void *)list, cli_release);
        ioclistnode* node = ioclist_head(list);
        while(node != NULL)
        {
            iochashmap_del(lcli->_conns, node->_data);
            node = node->_next;
        }
        ioclist_release(list);
        lcli->_conns = NULL;
    }
    free(lcli);
}

void iockcpclientconn_sendconnect(iockcpclientconn* lconn)
{
    int32_i seq = 30000;
    char data[IKCPCONNECT_PACKSIZE] = {0};
    data[0] = IKCPCONNECT_START_BIT;
	data[1] = IKCPCONNECT_SECOND_BIT;
	data[7] = IKCPCONNECT_END_BIT;
    memcpy((void *)(data + IKCPCONNECT_DATA_POS), (const void *)&seq, IKCPCONNECT_DATA_SIZE);
    int32_i ret = udp_send(lconn, (const char *)data, IKCPCONNECT_PACKSIZE);
}

uint32_i iockcpclientconn_create(iockcpclient* lcli, conn_proc conncb, resolve_proc resolvcb, close_proc closecb, const char* laddr, const int32_i port)
{
    if (conncb == NULL || resolvcb == NULL)
    {
        return 0;
    }
    iockcpclientconn* lconn = (iockcpclientconn *)malloc(sizeof(iockcpclient));
    if (lconn == NULL)
    {
        iocsyslog_printf(IOCSYSLOG_FTA, "[iockcpclientconn_create] create kcpclientconn error.\n");
        return 0;
    }
    lconn->_parent = lcli;
    lconn->_kcpcb = NULL;
    lconn->_conncb = conncb;
    lconn->_resolvecb = resolvcb;
    lconn->_closecb = closecb;
    lconn->_status = IOC_NETCONN_NOMAL;
    lconn->_ref = 0;

    // 创建套接字
    lconn->_remote._sckaddr.sin_family = AF_INET;
    lconn->_remote._sckaddr.sin_port = htons((uint16_i)port);
    lconn->_remote._sckaddr.sin_addr.s_addr = inet_addr(laddr);
    lconn->_sckfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (lconn->_sckfd == -1)
    {
        iocsyslog_printf(IOCSYSLOG_FTA, "[iockcpclientconn_create] create udp socket error.\n");
        goto _KCPCLIENT_CONN_CREATE_ERR;
    }
    if (iocnet_setreuseaddr(lconn->_sckfd) == -1)
    {
        iocsyslog_printf(IOCSYSLOG_FTA, "[iockcpclientconn_create] set reuseaddr error.\n");
        goto _KCPCLIENT_CONN_CREATE_ERR;
    }
    if (lconn->_sckfd > lcli->_maxfd)
    {
        lcli->_maxfd = lconn->_sckfd;
    }
    FD_SET(lconn->_sckfd, &lcli->_fdset);
    if (!iochashmap_put(lcli->_conns, (void *)((int64_i)lconn->_sckfd), (void *)lconn))
    {
        iocsyslog_printf(IOCSYSLOG_FTA, "[iockcpclientconn_create] put socket into map error.\n");
        goto _KCPCLIENT_CONN_CREATE_ERR;
    }
    iockcpclientconn_sendconnect(lconn);
    return lconn->_sckfd;

_KCPCLIENT_CONN_CREATE_ERR:
    if (lconn->_sckfd > 0)
    {
        close(lconn->_sckfd);
    }
    free(lconn);
    return 0;
}

int32_i iockcpclientconn_send(iockcpclient* lcli, uint32_i id, const char* data, int32_i len)
{
    iockcpclientconn* conn = (iockcpclientconn *)iochashmap_get(lcli->_conns, (void *)(uint64_i)id);
    if (conn == NULL)
        return -1;
    if (conn->_status != IOC_NETCONN_LINK)
    {
        iockcpclientconn_subref(conn);
        return -1;
    }
    int32_i ret = ikcp_send(conn->_kcpcb, data, len);
    iockcpclientconn_subref(conn);
    return ret;
}

int32_i iockcpclientconn_recv(iockcpclient* lcli, uint32_i id, const char* src, int32_i srclen, char* dst, int32_i dstlen)
{
    iockcpclientconn* conn = (iockcpclientconn *)iochashmap_get(lcli->_conns, (void *)(uint64_i)id);
    if (conn == NULL)
    {
        return -1;
    }

    // 调用此方法在kcp内部解码
    char recvbuf[UDP_PACKET_MAXLENGTH] = { 0 };
    ikcp_input(conn->_kcpcb, src, srclen);
    int32_i recvlen = ikcp_recv(conn->_kcpcb, recvbuf, sizeof(recvbuf));
    iockcpclientconn_subref(conn);
    if (recvlen < 0)
    {
        // < 0 for EAGAIN
        return recvlen;
    }
    int32_i count = recvlen <= dstlen ? recvlen : dstlen;
    memcpy(dst, recvbuf, count);
    return count;
}

Boolean iockcpclientconn_close(iockcpclient* lcli, uint32_i id)
{
    iockcpclientconn* conn = (iockcpclientconn *)iochashmap_get(lcli->_conns, (void *)(uint64_i)id);
    if (conn == NULL)
        return FALSE;
    conn->_status = IOC_NETCONN_CLOSE;
    iockcpclientconn_subref(conn);
    return TRUE;
}

void iockcpclientconn_release(iockcpclientconn* lconn)
{
    if (lconn->_kcpcb)
    {
        ikcp_release(lconn->_kcpcb);
        lconn->_kcpcb = NULL;
    }
    if (lconn->_closecb != NULL && lconn->_sckfd > 0)
    {
        iconnmsg msg;
        msg._connid = lconn->_sckfd;
        msg._arg = (void *)lconn->_parent;
        lconn->_closecb(msg);
    }
    if (lconn->_sckfd > 0)
    {
        iocnet_close(lconn->_sckfd);
        lconn->_sckfd = -1;
    }
    free(lconn);
}
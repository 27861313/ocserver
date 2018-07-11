#include <stdio.h>
#include "../common/ithread.h"
#include "../common/iatom.h"
#include "../net/ikcpserver.h"
#include "../net/ikcpclient.h"
#include "../net/inet.h"

const char* sendstr_cli = "hello server !";
const char* sendstr_svr = "hello client !";

iockcpclient* g_client = NULL;
ikcpserver* g_server = NULL;

// -------------------------------------------------------------------------------------------------------------

uint32_i g_cliconn = 0;


void cli_conncb(iconnmsg msg)
{
    fprintf(stdout, "clientconn create succ !\n");
}

void cli_closecb(iconnmsg msg)
{
    fprintf(stdout, "clientconn closed !\n");
}

int32_i cli_resolvcb(iresolvmsg msg)
{
    char buffer[2 * UDP_PACKET_MAXLENGTH] = { 0 };
    int32_i len = iockcpclientconn_recv((iockcpclient *)msg._arg, msg._connid, msg._data, msg._datalen, buffer, sizeof(buffer));
    if (len > 0)
    {
        buffer[len] = '\0';
        fprintf(stdout, "%s\n", buffer);
    }
    else
    {
        fprintf(stderr, "cli_resovcb recv error: %d\n", len);
    }
    return 0;
}


void *thrd_client_main(void *ud)
{
    g_client = iockcpclient_create();
    if (g_client == NULL)
    {
        fprintf(stderr, "create client failed !\n");
        exit(2);
    }
    fprintf(stdout, "create client successed !\n");
    g_cliconn = iockcpclientconn_create(g_client, cli_conncb, cli_resolvcb, cli_closecb,"127.0.0.1", 12345);
    if (g_cliconn == NULL)
    {
        fprintf(stderr, "create client conn failed !\n");
        exit(3);
    }
    fprintf(stdout, "create client conn successed !\n");
    while(1)
    {
        ISLEEP(200);
        iockcpclientconn_send(g_client, g_cliconn, sendstr_cli, strlen(sendstr_cli));
        //fprintf(stdout, "client_main_update\n");
    }
    return NULL;
}

// --------------------------------------------------------------------------------------------------------------

int32_i svr_splitcb(iresolvmsg msg)
{
    //fprintf(stdout, "svr_splitcb: %s\n", data);
    return 0;
}

void svr_acceptcb(iacceptedmsg msg)
{
    ikcpconnection* lconn = NULL;
    int32_i ret = kcpserver_accept((ikcpserver*)(msg._lsrv), msg._clientaddr, &lconn);
    if (ret != 0)
    {
        fprintf(stderr, "svr_accept conn failed: ret: %d\n", ret);
        return;
    }
    fprintf(stdout, "svr_acceptcb, accept conn..\n");
    kcpconnection_send(lconn, 32, (char *)sendstr_svr, strlen(sendstr_svr));
}

void svr_warncb(iwarnmsg msg)
{
    fprintf(stdout, "svr_warncb\n");
}

void svr_closecb(iconnmsg msg)
{
    fprintf(stdout, "svr_closecb\n");
}

void *thrd_server_wait(void *arg)
{
    int32_i ret = 0;
    while (ret >= 0 || ret == -2)
    {
        ret = kcpserver_wait(g_server, 100);
    }
    return NULL;
}

void *thrd_server_main(void *arg)
{
    g_server = kcpserver_create(NULL, "0.0.0.0", 12345, 32, 32);
    if (g_server == NULL)
    {
        fprintf(stderr, "create server failed !\n");
        exit(1);
    }
    fprintf(stdout, "create server successed !\n");
    kcpserver_bindcallback(g_server, svr_splitcb, svr_acceptcb, svr_warncb, svr_closecb);
    ithread th;
    ithread_create(&th, NULL, thrd_server_wait, NULL);
    while(1)
    {
        ISLEEP(20);
        kcpserver_guard(g_server);
        //fprintf(stdout, "server_main_update\n");
    }
    return NULL;
}

int test_kcp_main()
{
    ithread thc, ths;
    ithread_create(&ths, NULL, thrd_server_main, NULL);
    ISLEEP(3000);
    ithread_create(&thc, NULL, thrd_client_main, NULL);
    ithread_join(ths);
    ithread_join(thc);
    return 0;
}



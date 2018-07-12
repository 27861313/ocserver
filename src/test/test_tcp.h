#ifndef TEST_TCPSERVER_H
#define TEST_TCPSERVER_H


#ifdef __cplusplus                                                                           
extern "C"                                                                                   
{
#endif

#include "../net/itcpserver.h"
#include "../net/itcpclient.h"
#include "../common/ievent.h"
#include "../common/ithread.h"
#include "../common/iconvert.h"


int32_i split(iresolvmsg msg)
{
	if (msg._datalen == 0)
	{
		return -1;
	}
	printf("get:%d  data  : %s \n\n\n",msg._datalen, msg._data);
	return msg._datalen;
        //return 0;
}
void tcpaptt(itcpaptmsg msg)
{
	iendpoint addr;
	for (uint32_i n = 0; n < msg._irequest; ++n)
	{
		itcpserver_accept((itcpserver*) msg._lsrv, &addr);
	}
}
void readd(iconnmsg msg)
{
	itcpserver* tcpser = (itcpserver*)msg._arg;
	uint32_i connfd = msg._connid;
	ioctcpsystemsendmsg* tcpsysmsg = (ioctcpsystemsendmsg*)malloc(sizeof(ioctcpsystemsendmsg));
	tcpsysmsg->_tcpser = tcpser;
	tcpsysmsg->_connfd = connfd;
	if (sendmessage(tcpser->_system, &(tcpser->_evtlisten), OC_TCPNET_RECV, (void *)tcpsysmsg) == TRUE)
	  printf ("send readmsg succ\n");
}
void closee(iconnmsg msg)
{
}

void iocsystem_initt(void *)
{}
void iocsystem_releasee(void *)
{}
void iocsystem_updatee(void *)
{}

void iocqueue_free(void *)
{}
void* multhread_funn(void* lth, void* lsys)
{
	while(1)
	{
		printf("iocsystem_waitwork\n");
		iocsystem_waitwork((iocsystem*)lsys, (iocmultithread_t*)lth, 0);
	}
}
itcpserver* tcpser_create(iocsystem* sys)
{
		itcpserver_callback lcallbak;
		lcallbak._split = &split; 
		lcallbak._accept = &tcpaptt; 
		lcallbak._read = &readd; 
		lcallbak._close = &closee; 
		itcpserver* tcpser = itcpserver_create(sys, 11223, "192.168.1.203", 10000, &lcallbak);
return tcpser;

}


ioctcpclient* tcpcli_create(iocsystem* sys)
{
		itcpserver_callback lcallbak;
		lcallbak._split = &split; 
		lcallbak._accept = &tcpaptt; 
		lcallbak._read = &readd; 
		lcallbak._close = &closee; 
		ioctcpclient* tcpcli = ioctcpclient_create(sys, &lcallbak);
return tcpcli;

}
void itcpclient_send_delay(ioctcpclient *tc, uint32_i connfd)
{}

void* tcpser_wait(void* arg)
{
		itcpserver* tcpser = (itcpserver*)arg;
		while (1)
		{
				itcpserver_wait(tcpser, 0);
				sleep(0);
		}
}

void* tcpser_guard(void* arg)
{
		itcpserver* tcpser = (itcpserver*)arg;
		while(1)
		{
			itcpserver_guard(tcpser);
			sleep(1);
		}
}

typedef struct CLIMSG climsg;
struct CLIMSG
{
   	ioctcpclient* tc;
	uint32_i connfd;
};

void* tcpclientsend(void* arg)
{
		ioctcpclient* tc = (ioctcpclient*)arg; 
		ioctcpclientconnection* tcc = ioctcpclient_connect(tc, "192.168.1.203", 11223);
	//ioctcpclient* tc = ((climsg*)arg)->tc;
	//ioctcpclientconnection* tcc = ioctcpclient_getconntion(tc, ((climsg*)arg)->connfd);
	while (1)
	{
		printf("\n\n\nclient send data\n");
		ioctcpclient_send(tc, tcc->_netid, "asdfasfsa", sizeof("asdfasfsa") + 1);
		ioctcpclient_dosend(tc, tcc->_netid, &itcpclient_send_delay);
		sleep(1);
	}

}

void* tcpclientread(void* arg)
{
		ioctcpclient* tc = (ioctcpclient*)arg; 
		ioctcpclientconnection* tcc = ioctcpclient_connect(tc, "192.168.1.203", 11223);
	//ioctcpclient* tc = ((climsg*)arg)->tc;
	//ioctcpclientconnection* tcc = ioctcpclient_getconntion(tc, ((climsg*)arg)->connfd);
	int32_i flags = fcntl(tcc->_netid, F_GETFL, 0);
	fcntl(tcc->_netid, F_SETFL, flags | O_NONBLOCK);
	iocepoll_add_fd(tc->_tcpep->_epfd, tc->_tcpep->_evts, tcc->_netid, EPOLLIN | EPOLLET/*边缘触发*/, FALSE);
	while (1)
	{
		int32_i esize = iocepoll_wait(tc->_tcpep, 0);
		if (esize > 0)
		{
			printf("client epoll gert\n");
			for (uint32_i i = 0; i < esize; ++i)
			ioctcpclient_read(tc, tc->_tcpep->_evts[i].data.fd);

		}
	}

}

void test_tcp()
{
		iocsystem sys;

		iocsystem_param sys_param;
		sys_param._initfun = &iocsystem_initt;
		sys_param._releasefun = &iocsystem_releasee;
		sys_param._updatefun = &iocsystem_updatee;


		iocmultithread_param thread_param;
		thread_param._thnum = 4;
		thread_param._evtmax = 10;
		thread_param._freefun = &iocqueue_free;
		thread_param._multitfun = &multhread_funn;
		thread_param._multitarg = &sys;
		thread_param._costacksize = 10;
		thread_param._cogcc = 10;

		iocsystem_init(&sys, "tcpsersys", &thread_param, &sys_param);
		IOCMULTITHREAD_ROUSE(sys._pool_t);  //唤醒线程池




		itcpserver* tcpser = tcpser_create(&sys);
		ioctcpclient* tcpcli = tcpcli_create(&sys); 
		tcpcli->_callback._split = &split; 
		ioctcpclientconnection* tcc = ioctcpclient_connect(tcpcli, "192.168.1.203", 11223);
		iocthread* serwait = iocthread_create(NULL, tcpser_wait, tcpser);
		iocthread* serguard = iocthread_create(NULL, tcpser_guard, tcpser);

	//	climsg msg;
	//	msg->tc = tcpcli;
	//	msg->connfd = tcc->_netid;
		iocthread* clientsend = iocthread_create(NULL, tcpclientsend, tcpcli);
		iocthread* clientread = iocthread_create(NULL, tcpclientread, tcpcli);
		iocthread_run(serwait);
		iocthread_run(clientsend);
		iocthread_run(clientread);
		iocthread_run(serguard);
		//iocthread_run(client);
		//iocthread_run(client);

		while (1)
		{
				ISLEEP(100);
		}
}

#ifdef __cplusplus
}
#endif

#endif

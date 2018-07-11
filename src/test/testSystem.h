#ifndef OC_TEST_SYSTEM_H
#define OC_TEST_SYSTEM_H

#include "../common/imultithread.h"
#include "../common/isystem.h"

#include "../common/iconvert.h"
#include "../common/idatetime.h"

/*
int32_i itest_stop = 0;
Boolean  itest_sended = FALSE;
iocevlisten  itest_listen;

void initcmsystem(void *arg)
{

}

void releasecmsystem(void *arg)
{

}

void updatecmsystem(void *arg)
{
	iocsystem *lsys = (iocsystem*)arg;
	while (1)
	{
		if (itest_stop > 100)
			break;
		++itest_stop;
	}

	if (!itest_sended)
	{
		while (1)
		{
			if (SYSTEM_STOP(lsys))
			{
				printf("stop send ok!\n");
				break;
			}
			else
				sched_yield();
		}
		

		itest_sended = TRUE;
	}
}

void* test_systemworker(void *arg1, void *arg2)
{
	iocmultithread_t *t = (iocmultithread_t*)arg1;
	iocsystem *lsys = (iocsystem*)arg2;
	int32_i iret = waitwork(lsys, t);
	return 0;
}

void test_system_stop()
{
	int32_i threadnum = 2;
	iocevlistener_create(&itest_listen, TRUE);
	

	iocsystem      cmsys;
	initsys(&cmsys, "cm", 1024, threadnum, test_systemworker, initcmsystem, releasecmsystem, updatecmsystem);

	waitstop(&cmsys);
	releasesys(&cmsys);
	printf("System Stoped...\n");

	while (1)
	{
		ISLEEP(100);
	}
}*/

#define TESTCOUNT_MAX 50000

uint32_i testReady = 0;
uint32_i testsqd = 0;
Boolean istestend = FALSE;
uint64_i teststart = 0;
uint64_i testend = 0;
iocevlisten itest_listen_run;

Boolean iscontinue = FALSE;

ilock testlk;

enum test_event_t
{
	OC_TESTI_EVENT = OC_EVENT_USER + 1,
	OC_TESTII_EVENT
};

void *test_system_exec_1(void *arg)
{
	iocsystem *lsys = (iocsystem *)arg;
	int32_i thsend = 0;
	Boolean bReady = FALSE;
	for (;;)
	{
		int32_i therror = 0;
		if (thsend <= TESTCOUNT_MAX && !istestend && !bReady)
		{
			if (!sendmessage(lsys, &itest_listen_run, OC_TESTI_EVENT, NULL))
			{
				++therror;
				sched_yield();
				//printf("error:%d\n", testerro);
			}
			else
			{
				//AtomicAdd(&thsend, 1);
			}
		}
		else
		{
			ilock_k(&testlk);
			if (istestend && !bReady)
			{
				thsend = 0;
				bReady = TRUE;
				if (testReady == 0)
				{
					printfworker_vol(lsys);
					//AtomicRelease(&testsqd);
				}
				++testReady;
				if (testReady >= 2)
				{
					istestend = FALSE;
				}
			}
			else if (!istestend && bReady)
			{
				if (testReady > 0)
					--testReady;
				bReady = FALSE;
			}
			ilock_uk(&testlk);
			ISLEEP(100);
		}
	}
}

void update_run(void *arg)
{
}

void *system_worker_run(void *arg1, void *arg2)
{
	iocmultithread_t *t = (iocmultithread_t *)arg1;
	iocsystem *lsys = (iocsystem *)arg2;

	/*
	while (!iscontinue)
	{
		ISLEEP(1);
	}*/
	//printf("worker\n");
	//int32_i iret = waitwork(lsys, t);

	return 0;
}

void *test_event_fun(uint16_i evtid, void *arg)
{
	int32_i ival = v_toint32(arg);
	//AtomicAdd(&testempcount, 1);

	/*if (AtomicAdd(&testsqd, 1) >= (TESTCOUNT_MAX * 2) && !istestend)
	{
		testend = getdida_msec();
		int32_i tmcnt = testsqd;
		uint64_i curtime = testend - teststart;
		teststart = getdida_msec();
		printf("time consuming  %llu msec count:%d\n", curtime, tmcnt);
		istestend = TRUE;
	}*/
	return NULL;
}

void test_system_run()
{
	/*ilock_init(&testlk);
	int32_i threadnum = 2;
	iocsystem cmsys;
	//initsys(&cmsys, "cm-run", 1024, threadnum, system_worker_run, NULL, NULL, update_run);
	iocevlistener_create(&cmsys, &itest_listen_run, FALSE, TRUE);
	//initsys(&cmsys, "cm-run", 1024, threadnum, system_worker_run, NULL, NULL, update_run);

	iocevlistener_register(&itest_listen_run, NULL, OC_TESTI_EVENT, &test_event_fun);

	ISLEEP(2000);

	//�����������߳�
	iocthread *pth_1 = iocthread_create(NULL, test_system_exec_1, &cmsys);
	iocthread *pth_2 = iocthread_create(NULL, test_system_exec_1, &cmsys);

	teststart = getdida_msec();
	iocthread_run(pth_1);
	iocthread_run(pth_2);

	int32_i testerro = 0;

	while (1)
	{
		ISLEEP(100);
	}*/
	/*while (1)
	{
		if (testsendct <= TESTCOUNT_MAX)
		{
			if (!sendmessage(&cmsys, &itest_listen_run, OC_TESTI_EVENT, (void*)testsendct))
			{
				++testerro;
				sched_yield();
				//printf("error:%d\n", testerro);
			}
			else
			{
				AtomicAdd(&testsendct, 1);
			}
		}
		else
		{
			/ *if (!iscontinue)
			{
				iscontinue = TRUE;
			}* /

			if (istestend)
			{
				printfworker_vol(&cmsys);

				testerro = 0;
				istestend = FALSE;
				__sync_lock_release(&testcount);
				__sync_lock_release(&testsendct);
			}
			ISLEEP(100);
		}
	}*/
}

#endif

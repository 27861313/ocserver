#ifndef OC_THREAD_QUEUE_H
#define OC_THREAD_QUEUE_H

#include "../common/iqueue.h"
#include "../common/ithread.h"

#include <assert.h>
#include <string.h>

Boolean _gstoped = FALSE;
Boolean _gconnect_1 = FALSE;

#define TEST_QUEUE_MAX 4096
#define TEST_QUEUE_INSERT_NUM 5000

void *test_thread_exec_1(void *arg)
{
	int imax = TEST_QUEUE_INSERT_NUM, i = 0;
	iocqueue *lpqu = (iocqueue *)arg;
	while (!_gstoped && i < imax)
	{
		iocqueue_push(lpqu, (void *)1);

		if (i % 10 == 0)
			ISLEEP(5);
		++i;
	}

	_gconnect_1 = TRUE;
	ISLEEP(5);
	printf("queue release begin\n");
	iocqueue_release(lpqu);
	printf("queue release end\n");
	return NULL;
}

void *test_thread_exec_2(void *arg)
{
	iocqueue *lpqu = (iocqueue *)arg;
	while (!_gstoped)
	{
		while (!_gconnect_1)
		{
			ISLEEP(2);
			continue;
		}
		iocqueue_pop(lpqu);
	}
	return NULL;
}

void *test_thread_exec_3(void *arg)
{
	printf("test_thread_exec_3\n");
	return NULL;
}

void test_thread_main()
{
	/*iocqueue *lpqueue = iocqueue_create(TEST_QUEUE_MAX, NULL);
	assert(lpqueue);
	iocthread *pth_1 = iocthread_create(NULL, test_thread_exec_1, lpqueue);
	iocthread *pth_2 = iocthread_create(NULL, test_thread_exec_2, lpqueue);
	assert(pth_1);
	assert(pth_2);
	iocthread_run(pth_1);
	iocthread_run(pth_2);*/
	//test run---------------------------------------------
	iocthread *pth_3 = iocthread_create(NULL, test_thread_exec_3, NULL);
	assert(pth_3);
	iocthread_run(pth_3);
	iocthread_run(pth_3);

	while (1)
	{
		ISLEEP(100);
	}
}

#endif
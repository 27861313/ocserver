#ifndef OC_TEST_RINGBUFFER_H
#define OC_TEST_RINGBUFFER_H

#include "../common/iringbuffer.h"
#include "../common/ithread.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *test_ringbuffer_exec_1(void *arg) //д
{
	iocringbuffer *lbuf = (iocringbuffer *)arg;
	return NULL;
}

void *test_ringbuffer_exec_2(void *arg) //��
{
	iocringbuffer *lbuf = (iocringbuffer *)arg;
	return NULL;
}

void *test_ringbuffer_exec_3(void *arg) //д
{
	iocringbuffer *lbuf = (iocringbuffer *)arg;
	return NULL;
}

void test_ringbuffer()
{
	iocringbuffer *lpbuf = iocringbuffer_create(4096);

	iocthread *pth_1 = iocthread_create(NULL, test_ringbuffer_exec_1, lpbuf);
	iocthread *pth_2 = iocthread_create(NULL, test_ringbuffer_exec_2, lpbuf);
	iocthread *pth_3 = iocthread_create(NULL, test_ringbuffer_exec_3, lpbuf);

	iocthread_run(pth_1);
	iocthread_run(pth_3);
	iocthread_run(pth_2);

	while (1)
	{
		ISLEEP(100);
	}
}

#endif
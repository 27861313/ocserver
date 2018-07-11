
#ifndef OC_TEST_EVENT_H
#define OC_TEST_EVENT_H

//#include "common/ievent.h"
//#include "common/ithread.h"

#include "common/ievent.h"

Boolean _gevent2_co = FALSE;
Boolean _gevent2_cr = FALSE;
Boolean _gevent2_over = FALSE;

int32_i _gtest = 0;

void *test_event_exec_1(void *arg)
{
	/*printf("aaaa\n");
	ioclist *llist = (ioclist*)arg;
	int32_i imax = 2500;
	int32_i i = 101, icnt = 0;
	for (; i < imax; i++)
	{
		if (ioclist_insert(llist, i, NULL))
			++icnt;
	/ *	if (i > 102 && !_gevent2_co)
		{
			_gevent2_co = TRUE;
		}* /
	}

	_gevent2_co = TRUE;
	printf("===========================:%d\n", icnt );

	++_gtest;
	return NULL;*/
	return NULL;
}

void *test_event_exec_2(void *arg)
{
	/*
	ioclist *llist = (ioclist*)arg;

	while (!_gevent2_co)
	{
		ISLEEP(1);
	}

	while (1)
	{
		int32_i imax = 400;
		int32_i i = 100, icnt = 0;;
		for (; i < imax; i++)
		{
			//printf("mmmmmmmmmmmmmmmmmmmmmmmmmmmm\n");
			ioclist_remove(llist, i);
		}

		for (i=100; i < imax; i++)
		{
			if (ioclist_insert(llist, i, NULL))
			{

			}
		}
		printf("kkkkkkkkkkkkkkkkkkkkkkkkkkk:%d\n", icnt);
	}


	
	++_gtest;*/
	return NULL;
}

void *test_event_exec_3(void *arg)
{
	/*
	ioclist *llist = (ioclist*)arg;

	while (!_gevent2_co)
	{
		ISLEEP(1);
	}

	while (1)
	{
		int32_i imax = 1000;
		int32_i i = 100;
		int32_i ierr = 0;

		//for (; i < imax; i++)
		//{
			ioclist_get(llist, 150);
		//}
		
		printf("dddddddddddddddd:%d\n", ierr);
		ISLEEP(1);
	}
	++_gtest;
*/

	return NULL;
}

void test_event()
{

	/*
	ioclist *tlist = ioclist_create();

	iocthread *pth_1 = iocthread_create(NULL, test_event_exec_1, tlist);
	iocthread *pth_2 = iocthread_create(NULL, test_event_exec_2, tlist);
	iocthread *pth_3 = iocthread_create(NULL, test_event_exec_3, tlist);

	iocthread_run(pth_1);
	iocthread_run(pth_3);
	iocthread_run(pth_2);
	

	while (_gtest != 3)
	{

	}*/

	//ithread_join(pth_1->_t);
	//ithread_join(pth_2->_t);
	//ithread_join(pth_3->_t);

	//test_seach(tlist);

	while (1)
	{
		ISLEEP(100);
	}
}

//======================================================================================================

#endif
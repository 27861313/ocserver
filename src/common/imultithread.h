/*
 * Created on Thu Jun 14 2018
 *
 * The MIT License (MIT)
 * Copyright (c) 2018 mr liang
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef OC_IMULTITHREAD_H
#define OC_IMULTITHREAD_H

#include "iatom.h"
#include "icoroutine.h"
#include "iqueue.h"
#include "ithread.h"
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif

	typedef void *(*multhread_fun)(void *, void *);

	typedef struct IOCMULTITHREAD_T iocmultithread_t;
	typedef struct IOCMULTITHREAD iocmultithread;
	typedef struct IOCMULTITHREAD_PARAM iocmultithread_param;

	struct IOCMULTITHREAD_T
	{
		ithread _t;		   // thread id
		ilock _lk;		   //  lock
		icond _cn;		   // cond
		uint32_i _iw;	  // worker number
		uint32_i _ib;	  // bind thread number;
		iocschedule *_cos; // Co Process array
		iocqueue *_e;	  // Event Queue
		int16_i _icn;	  // cond number
	};

	struct IOCMULTITHREAD_PARAM
	{
		int32_i _thnum;				 // thread number
		int32_i _evtmax;			 // event queue max number
		iocqueue_free_data _freefun; // event queue data free function
		multhread_fun _multitfun;	//thread callback function
		void *_multitarg;			 // thread callback function param
		int32_i _costacksize;
		uint64_i _cogcc; //Co Process gcc time sec
	};

	struct IOCMULTITHREAD
	{
		//int16_i _multinum;
		//multhread_fun _multitfun;
		//void *_multitarg;
		iocmultithread_param _multitatt; //线程池参数
		iocmultithread_t **_multit;
		uint8_i _error;
	};

#define IOCMULTITHREAD_HANG_NAME iocmultithread_default
#define IOCMULTITHREAD_HANG_CALL void *IOCMULTITHREAD_HANG_NAME(void *arg)
#define IOCMULTITHREAD_HANG                                                  \
	{                                                                        \
		iocmultithread *lmult = (iocmultithread *)arg;                       \
		ithread _ct = ithread_self();                                        \
		iocmultithread_t *th_t = iocmultithread_handle(lmult, _ct);          \
		if (th_t == NULL)                                                    \
			ithread_exit(NULL);                                              \
		ilock_k(&th_t->_lk);                                                 \
		if (th_t->_icn <= 0)                                                 \
			icond_wait(&th_t->_cn, &th_t->_lk);                              \
		ilock_uk(&th_t->_lk);                                                \
		(*lmult->_multitatt._multitfun)(th_t, lmult->_multitatt._multitarg); \
		return 0;                                                            \
	}

#define IOCMULTITHREAD_ROUSE(lts)                                  \
	{                                                              \
		for (int32_i ith = 0; ith < lts->_multitatt._thnum; ith++) \
		{                                                          \
			ilock_k(&lts->_multit[ith]->_lk);                      \
			++lts->_multit[ith]->_icn;                             \
			icond_broadcast(&lts->_multit[ith]->_cn);              \
			ilock_uk(&lts->_multit[ith]->_lk);                     \
		}                                                          \
	}

#define IOCMULTITHREAD_ROUSE_ONE(lsys, seq)          \
	ilock_k(&lsys->_pool_t->_multit[seq]->_lk);      \
	++lsys->_pool_t->_multit[seq]->_icn;             \
	icond_signal(&lsys->_pool_t->_multit[seq]->_cn); \
	ilock_uk(&lsys->_pool_t->_multit[seq]->_lk);

	// @function 创建多个线程
	// @param    int16_i    - 线程数
	// @param    int16_i    - 每个队列的事件队列最大容量
	// @param    iocqueue_free_data - 队列对象释放函数
	// @param    multhread_fun      - 线程回调函数
	// @param    void*              - 线程回调参数
	// @return   iocmultithread*    - 多线程对象
	/*iocmultithread *
	iocmultithread_create(const int16_i multinum, const int16_i evtmax, iocqueue_free_data efree,
						  multhread_fun multfun, void *arg);*/
	iocmultithread *iocmultithread_create(const iocmultithread_param *att);

	// @function 释放多线程资源
	// @param    iocmultithread*    - 多线程对象
	// @return   void
	void iocmultithread_release(iocmultithread *lmult);

	IOCMULTITHREAD_HANG_CALL;

#ifdef __cplusplus
}
#endif

#endif
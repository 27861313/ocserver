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

#ifndef OC_IQUEUE_H
#define OC_IQUEUE_H

#include "iinc.h"
#include "ithread.h"

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct IOCQUEUE_NODE iocqueue_node;
	typedef struct IOCQUEUE iocqueue;

	typedef void (*iocqueue_free_data)(void *);

	struct IOCQUEUE_NODE
	{
		void *_data;
		iocqueue_node *_next;
	};

	struct IOCQUEUE
	{
		uint32_i _num;
		uint32_i _max;
		uint16_i _status;
		ithread _wt;
		ithread _rt;
		iocqueue_node *_head;
		iocqueue_node *_tail;
		iocqueue_free_data _freecall;
	};

	enum IOCQUEUE_STATUS
	{
		OC_QU_NOMAL = 0,
		OC_QU_RELEASE = 1
	};

	typedef enum IOCQUEUE_STATUS iocqueue_status;

	// @function 创建队列
	// @param    maxnum    队列最大容量限制
	// @param    freecall  队列对象释放回调
	iocqueue *iocqueue_create(uint32_i maxnum, iocqueue_free_data freecall);

	// @function 弹出一个数据
	// @param    lq*    队列对象
	// @return   void*  数据
	void *iocqueue_pop(iocqueue *lq);

	// @function 插入一个数据
	// @param    lq*    队列对象
	// @param    void*  数据
	Boolean iocqueue_push(iocqueue *lq, void *data);

	// @function 计算当前队列大小
	// @param    lq*    	队列对象
	// @return   uint32_i 	当前队列数据个数
	uint32_i iocqueue_count(iocqueue *lq);

	// @function 释放队列资源
	// @param    lq*    	队列对象
	// @param    void
	void iocqueue_release(iocqueue *lq);

#ifdef __cplusplus
}
#endif

#endif
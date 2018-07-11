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

#ifndef OC_ITIMER_H
#define OC_ITIMER_H

#include "ievent.h"
#include "ithread.h"
#include <time.h>

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct IOCSYSTEM iocsystem;
	//
	typedef struct IOC_ITIMER_NODE ioctimer_node;
	typedef struct IOC_ITIMER_LST ioctimer_lst;
	typedef struct IOC_ITIMER ioctimer;
	typedef struct IOC_ITIMEWHEEL ioctimewheel;

	struct IOC_ITIMER
	{
		int32_i _id;
		int32_i _oldrotation;
		int32_i _rotation;
		int32_i _time_slot;
		int32_i _interval; //重复定时次数 0.不重复 1.重复
		int32_i _ithid;	//线程绑定ID
		ievent_callback _event_call;
		void *_event_arg;

		time_t _expire;		//set time
		time_t _start_time; //set time in start
	};

	struct IOC_ITIMER_NODE
	{
		ioctimer _timer;
		ioctimer_node *_next;
	};

	struct IOC_ITIMER_LST
	{
		int32_i _num;
		ioctimer_node *_head;
		ioctimer_node *_tail;
		ilock _lck;
	};

	struct IOC_ITIMEWHEEL
	{
		ioctimer_lst *_slots;
		int32_i _slotmax;
		int32_i _timerseq;
		int32_i _curslot;
	};

	// @function 创建定时器
	// @param    slotnum			- 轮盘数
	// @return   ioctimewheel       - 定时对象
	ioctimewheel *ioctimerwheel_create(int32_i slotnum);

	// @function 释放定时器对象
	// @param    ioctimewheel		- 定时器对象
	// @return   void				-
	void ioctimerwheel_release(ioctimewheel *lwh);

	// @function 注册一个定时
	// @param    ioctimewheel		- 定时器对象
	// @param    int32_i			- 定时毫秒
	// @param    int32_i			- 定时器是否循环
	// @param    int32_i			- 定时器回调是否绑定线程 -1.不绑定
	// @param    ievent_callback    - 定时器事件回调函数
	// @param    void*              - 定时器回调arg
	// @return   int32_i			- 定时器ID
	int32_i ioctimerwheel_register(ioctimewheel *lwh, int32_i msec, int32_i interval, int32_i ithid, ievent_callback timecallback, void *timearg);

	// @function 注销一个定时器
	// @param    ioctimewheel		- 定时器对象
	// @param    int32_i			- 定时器ID
	// @return   Boolean            - 是否注销成功

	Boolean ioctimerwheel_unregister(ioctimewheel *lwh, int32_i timeid);

	// @function 定时器 Tick
	// @param    ioctimewheel		- 定时器对象
	// @param    iocsystem			- 系统对象
	// @param    uint64_i			- 当前系统毫秒数
	// @return   void               -
	void ioctimerwheel_tick(ioctimewheel *lwh, iocsystem *lsys, uint64_i usec);

#ifdef __cplusplus
}
#endif

#endif

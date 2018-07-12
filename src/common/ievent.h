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

#ifndef OC_IEVENT_H
#define OC_IEVENT_H

#include "icoroutine.h"
#include "iinc.h"
#include "iqueue.h"
#include "ithread.h"
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif

	//===============================================================================================================

	//event status
	enum IOCEV_HANDLE_STATUS
	{
		OC_EV_NOMAL, //正常状态
		OC_EV_CANCEL //取消状态
	};

	typedef void *(*ievent_callback)(uint16_i evtid, void *arg);
	typedef struct IOCEV_LISTEN iocevlisten;
	typedef struct IOCEV_HANDLE iocevhandle;
	typedef struct IOCEV_EVENT iocevent;
	typedef struct IOCV_EVENTMSG iocevent_msg;
	typedef struct IOCSYSTEM	 iocsystem;
	typedef enum IOCEV_HANDLE_STATUS iocevhandle_st;

	//event default id
	enum IOCEV_EVENTID
	{
		OC_EVENT_START     = 100,  // 开始运行，一般没使用
		OC_EVENT_STOP      = 101,  // 停止运行
		OC_EVENT_TIMER     = 102,  // 定时事件
		OC_EVENT_COROUSE   = 103,  // 协程唤醒事件
		OC_EVENT_USER      = 104,
		OC_TCPNET_SEND     = 105,  // 网络发送事件
		OC_TCPNET_RECV     = 106,  // 网络接收事件
		OC_EVENT_KCPCONN_DATA = 107, //KCP连接有数据可读
	};

	//register event handle
	struct IOCEV_HANDLE
	{
		uint16_i _evt_tye; //event id
		uint16_i _evt_rf;  //event ref
		void *_evt_owner;
		ievent_callback _evt_call; //event callback function
		uint8_i _evt_status;	   //event status
	};

	//register event node
	struct IOCEV_EVENT
	{
		iocevhandle *_evt;
		iocevent *_evt_next;
	};

	//event message
	struct IOCV_EVENTMSG
	{
		iocevhandle *_evt; //event handle
		void *_evt_arg;	//event callback arg
	};

	//event listen
	struct IOCEV_LISTEN
	{
		Boolean _evt_issync; //is sync
		int32_i _evt_thidx;  //bind thread index
		iocevent *_evt_head;
		iocevent *_evt_tail;
		irwlock _evt_lk;
	};

#define IEVENT_RELEASE_EVENT(lp) \
	{                            \
		if (lp->_evt != NULL)    \
		{                        \
			free(lp->_evt);      \
		}                        \
		free(lp);                \
	}

#ifdef OC_MULTITHREADING
	//release event handle ref
#define RELEASE_EVENT_HANDLE(handle)                       \
	{                                                      \
		if (AtomicSubFetch(&handle->_evt_rf, 1) == 0)      \
		{                                                  \
			iocevhandle *hfree = handle;                   \
			handle = NULL;                                 \
			memset((char *)hfree, 0, sizeof(iocevhandle)); \
			/*free(hfree);*/                                   \
		}                                                  \
	}
#else
//release event handle ref
#define RELEASE_EVENTHANDLE(handle)                            \
	{                                                          \
		handle->_evt_rf -= 1;                                  \
		if (handle->_evt_rf) == 0)                             \
			{                                                  \
				iocevhandle *hfree = handle;                   \
				handle = NULL;                                 \
				memset((char *)hfree, 0, sizeof(iocevhandle)); \
				free(hfree);                                   \
			}                                                  \
	}
#endif

	// @function create listener
	// @param    listener*   out listener
	// @param    isBindThread TRUE 绑定  FALSE 不绑定
	// @param    isSync       TRUE 同步 FALSE 不同步
	// @return   boolean      TRUE:success FALSE:fail
	Boolean iocevlistener_create(iocsystem *lsys, iocevlisten *listener, Boolean isBindThread, Boolean isSync);

	// @function register event
	// @param    listener*    listener object
	// @param    void*          owner object
	// @param    event_type  event id
	// @param    event_call    event callback function
	// @return   boolean         TRUE:success FALSE:fail
	Boolean iocevlistener_register(iocevlisten *listener, void *owner, uint16_i event_type, ievent_callback event_call);

	// @function unregister event
	// @param    listener*    listener object
	// @param    event_type  event id
	// @return   boolean      TRUE:success FALSE:fail
	Boolean iocevlistener_unregister(iocevlisten *listener, uint16_i event_type);

	// @function get event registered handle
	// @param    listener*         listener object
	// @param    event_type      event id
	// @return   iocevhandle*     event handle ref + 1
	iocevhandle *iocevlistener_gethandle(iocevlisten *listener, uint16_i event_type);

	// @function release listener object
	// @param    listener*    listener object
	// @return   void
	void iocevlistener_release(iocsystem *lsys, iocevlisten *listener);

	// @function send event
	// @param    iocqueue*      dest event queue
	// @param    sourlisten*   listener object
	// @param    event_type  event id
	// @param    event_arg    event callback arg
	// @return   boolean     TRUE:success FALSE:fail
	Boolean iocevsend_event(iocqueue *dest, iocevlisten *sourlisten, uint16_i event_type, void *event_arg);

	// @function send event extern
	// @param    iocqueue*           dest event queue
	// @param    event_type        event id
	// @param    event_callback   event callback function
	// @param    event_arg   event callback arg
	// @return   boolean     TRUE:success FALSE:fail
	Boolean iocevsend_event_extern(iocqueue *dest, uint16_i event_type, ievent_callback event_callback, void *event_arg);

#ifdef __cplusplus
}
#endif

#endif

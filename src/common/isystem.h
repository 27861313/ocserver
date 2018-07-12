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

#ifndef OC_ISYSTEM_H
#define OC_ISYSTEM_H

#include "ievent.h"
#include "iinc.h"
#include "imultithread.h"
#include "iqueue.h"
#include "ithread.h"
#include "itimer.h"

#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif
#define SYSTEM_LOG_LEVEL IOCSYSLOG_ALL
#define SYSTEM_LOG_MODE IOCSYSLOG_MODE_CONSOLE | IOCSYSLOG_MODE_FILE

#define SYSTEM_TIMER_SLOTMAX 10
#define SYSTEM_STOP(lsys) sendmessage_extern(lsys, OC_EVENT_STOP, NULL, NULL)

#define SYSEVENT_RUN(msg)                                                \
	{                                                                    \
		if (msg->_evt->_evt_status == OC_EV_CANCEL)                      \
			return;                                                      \
		if (msg->_evt->_evt_call != NULL)                                \
			(*msg->_evt->_evt_call)(msg->_evt->_evt_tye, msg->_evt_arg); \
	}

#define SYSTEM_NAME_MAX 32
#ifdef OC_MULTITHREADING
#define SYSTEM_MULTI_WAITSYS_OUT 10
#endif

//define system init function name
#define SYSTEM_INIT_FUNCTION(name) void##iocsyste_init_##name##(void *arg)
#define SYSTEM_RELEASE_FUNCTION(name) void##iocsystem_release_##name##(void *arg)
#define SYSTEM_UPDATE_FUNCTION(name) void##iocsystem_update_##name##(void *arg)

#define SYSTEM_INIT_FUNCTION_CALL(lsys) \
	{                                   \
		if (lsys->_init != NULL)        \
			(*lsys->_init)(lsys);       \
	}
#define SYSTEM_RELEASE_FUNCTION_CALL(lsys) \
	{                                      \
		if (lsys->_release != NULL)        \
			(*lsys->_release)(lsys);       \
	}
#define SYSTEM_UPDATE_FUNCTION_CALL(lsys) \
	{                                     \
		if (lsys->_update != NULL)        \
			(*lsys->_update)(lsys);       \
	}

#define SYSTEM_RELEASE_MESSAGE(lmsg)                   \
	{                                                  \
		RELEASE_EVENT_HANDLE(lmsg->_evt);              \
		memset((char *)lmsg, 0, sizeof(iocevent_msg)); \
		free(lmsg);                                    \
	}

	typedef void (*iocsystem_callback)(void *);

	enum IOCSYSTEM_STATUS
	{
		IOC_SYS_NOMAL,
		IOC_SYS_RUNING,
		IOC_SYS_STOPING
	};

	struct IOCSYSTEM_PARAM
	{
		iocsystem_callback _initfun;
		iocsystem_callback _releasefun;
		iocsystem_callback _updatefun;
	};

	struct IOCSYSTEM
	{
		/*系统名称*/
		char _name[SYSTEM_NAME_MAX];
		/*系统状态*/
		uint8_i _status;
		/*系统定时器*/
		ioctimewheel *_timers;
		/*系统初始化回调函数*/
		iocsystem_callback _init;
		/*系统资源释放回调函数*/
		iocsystem_callback _release;
		/*系统循环update回调函数*/
		iocsystem_callback _update;
		/*单线程与多线程*/
		iocthread *_main_t;
		uint32_i _dispatchseq;
		iocmultithread *_pool_t;
	};

	typedef struct IOCSYSTEM iocsystem;
	typedef struct IOCSYSTEM_PARAM iocsystem_param;

	iocsystem *sys_global;


// @function 初始化系统
// @param    iocsystem    - 系统对象
// @param    name		  - 系统名字
// @param    tnum		  - 事务调度最大缓冲区数
// @param    initfun	- 初始化回调函数
// @param    releasefun - 资源释放回调函数
// @param    updatefun  - update回调函数
// @return   Boolean - 初始化是否成功
//Boolean initsys(iocsystem *lsys, const char *name, const int32_i evtsize, const int32_i tnum, multhread_fun worker_callback, iocsystem_callback initfun, iocsystem_callback releasefun, iocsystem_callback updatefun);
Boolean iocsystem_init(iocsystem *lsys, const char *name, iocmultithread_param *multitatt, iocsystem_param *satt);


	// @function 释放系统资源
	// @param    iocsystem    - 系统对象
	// @return   void
	//void releasesys(iocsystem *lsys);
	void iocsystem_release(iocsystem *lsys);

	// @function 默认主线程挂起函数
	// @param    iocsystem    - 系统对象
	// @param    timeout      - 挂起超时 -1表示永不
	// @return   Boolean      - TRUE 继续 FALSE 退出系统
	Boolean iocsystem_waitsys(iocsystem *lsys, int32_i timeout);

	// @function 发送消息到事务调度器
	// @param    iocsystem    - 系统对象
	// @param    iocevlisten  - 监听器
	// @param    uint16_i     - 事务ID
	// @param    void*        - 事务调度参数
	// @return   Boolean      - TRUE 发送成功
	Boolean sendmessage(iocsystem *lsys, iocevlisten *sourlisten, uint16_i event_type, void *arg);

	// @function ����ֱ����Ϣ���������
	// @param   iocsystem       - 系统对象
	// @param   uint16_i        - 事务ID
	// @param   ievent_callback - 事务回调函数
	// @param   void*           - 事务调度参数
	// @param   int32_i			- 绑定线程ID  -1.不绑定
	// @return  Boolean         - TRUE 发送成功
	Boolean sendmessage_extern(iocsystem *lsys, uint16_i event_type, ievent_callback event_call, void *arg, int32_i ithid);

// @function 工作线程挂起
// @param    iocsystem×         - 系统对象
// @param    iocmultithread_t	- 线程对象
// @param    timesec  -挂起超时
// @return   in32_i		  -
int32_i iocsystem_waitwork(iocsystem *lsys, iocmultithread_t *lth, int32_i timesec);

// @function
// @param    void*    - 是
// @return   void* -
//void *single_main(void *arg);

// @function 对线程事务分离器
// @param    iocsystem    - 系统对象
// @return   void		  -
//void multi_dispatch(iocsystem *lsys, iocmultithread_t *lth);

// @function  等待系统停止
// @param     iocsystem - 系统对象
// @param     void -
void iocsystem_waitstop(iocsystem *lsys);

// @function  绑定线程
// @param     iocsystem - 系统对象
// @return    返回绑定的索引编号
int32_i iocsystem_bindth(iocsystem *lsys);

// @function 解除线程绑定
// @param    iocsystem - 系统对象
// @param    int32_i   - 解除线程索引
// @return   void
void iocsystem_unbindth(iocsystem *lsys, int32_i ithidx);

// @function 测试用答应工作状态或信心
void printfworker_vol(iocsystem *lsys);
#endif

#ifdef __cplusplus
}
#endif


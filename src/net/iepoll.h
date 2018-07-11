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

#ifndef OC_IEPOLL_H
#define OC_IEPOLL_H

#include "../common/iinc.h"
#include <errno.h>
#include <sys/epoll.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C"
{
#endif

	/*
	 epoll 
	 */

	struct IOCEPOLL
	{
		int _epfd;
		int _evtnum;
		struct epoll_event *_evts;
		struct epoll_event _event;
	};

	typedef struct IOCEPOLL iocepoll;

	//@function    创建 iocepoll 对象
	//@param int   EPOLL_CLOEXEC
	//@param int   文件打开数
	//@param int   事件数组大小
	//@return        iocepoll 对象
	iocepoll *iocepoll_create(int flags /*EPOLL_CLOEXEC*/, int noFile /*8192*/, int eventsize /*128*/);

	//@function 添加事件
	//@param    iocepoll*  		  iocepoll对象
	//@param    int          		 fd
	//@param    unsigned int   事件
	//@param   ETorNot         默认false
	//@return  int                   0.success
	int iocepoll_add_fd(int epfd, struct epoll_event *hevt, int fd, unsigned int events, unsigned char ETorNot /*false*/);

	//@function 删除事件
	//@param   iocepoll*  iocepoll对象
	//@param    int          		 fd
	//@param    unsigned int   事件
	//@param   ETorNot         默认false
	//@return  int                   0.success
	int iocepoll_del_fd(int epfd, struct epoll_event *hevt, int fd, unsigned int events, unsigned char ETorNot /*false*/);

	//@function 获取事件绑定的FD
	//@param   iocepoll*  iocepoll对象
	//@param    int          eventindex
	//@return   int          fd
	int iocepoll_get_fd(iocepoll *ocepoll, int eventIndex);

	//@function 获取事件
	//@param   iocepoll*  	iocepoll对象
	//@param    int     	   eventindex
	//@return   uint32_t   evt
	uint32_i iocepoll_get_evt(iocepoll *ocepoll, int eventIndex);

	//@function 等待epoll IO事件
	//@param   iocepoll*  	iocepoll对象
	//@param   int			   timeout
	//@return  int
	int iocepoll_wait(iocepoll *ocepoll, int timeout);

	//@function 释放 icoepoll
	//@param   iocepoll*  	iocepoll对象
	//@return  int  void
	void iocepoll_release(iocepoll *ocepoll);

#ifdef __cplusplus
}
#endif

#endif
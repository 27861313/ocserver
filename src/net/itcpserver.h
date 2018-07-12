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

#ifndef OC_ITCPSERVER_H
#define OC_ITCPSERVER_H

#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "iepoll.h"
#include "inet.h"
#include <common/iatom.h>
#include <common/ievent.h>
#include <common/ihashmap.h>
#include <common/iinc.h>
#include <common/ilist.h>
#include <common/iqueue.h>
#include <common/iringbuffer.h>
#include <common/isystem.h>

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct IOCTCP_CALLBACK itcpserver_callback;
	typedef struct IOCTCP_SERVER itcpserver;
	typedef struct IOCTCP_CONNECTION ioctcpconnection;
	typedef struct IOCTCP_SYSTEMSENDMSG ioctcpsystemsendmsg;

	struct IOCTCP_SYSTEMSENDMSG
	{
		itcpserver* _tcpser;
		uint32_i _connfd;
	};

	struct IOCTCP_CONNECTION
	{
		uint8_i _status;		// 套接字状态
		uint32_i _netid;		// connfd
		uint32_i _ref;			// 应用计数
		uint32_i _outtimes;		// 超时次数
		uint64_i _lastrecv;		// 上次通信时间
		iocringbuffer *_rdb;	// 读环形缓冲区
		iocringbuffer *_sdb;	// 发缓冲区
		iocqueue *_sque;		// 发送数据队列
	};

	struct IOCTCP_CALLBACK
	{
		resolve_proc _split;    // 解包回调
		tcpapt_proc _accept;    // 连接请求回调
		read_proc _read;	    // 读数据回调
		close_proc _close;      // 关闭回调
	};

	struct IOCTCP_SERVER
	{
		uint8_i _shutdown;			   // server stop 0.runing 1.stop
		uint32_i _linkmax;			   // 最大连接
		uint32_i _tcpfd;			   // tcp listen fd
		iocepoll *_tcpep;			   // epoll
		iendpoint _tcpendpoint;		   // port ip
		iochashmap *_netconhashmap;    // 存放tcpconn的hashmap
		iocsystem *_system;            // 系统线程
		itcpserver_callback _callback; // 回调函数群
		iocevlisten _evtlisten;        // event listen handle of object
	};

	//////////////////////////////////////////////////////////////////////

	// @function create tcp connector
	// @param    uint32_i          - key
	// @return   void              -
	//ioctcpconnection* ioctcpconnection_create(uint32_i netid);

	// @function release tcp connector
	// @param    ioctcpconnector*  - tcpcontor
	// @return   void              -
	//void ioctcpconnection_release(ioctcpconnection* tcpcon);

	// @function push data to queue
	// @param    ioctcpconnector*  - tcpcontor
	// @param    char*             - data
	// @param    uint32_i          - len
	// @return   int32_i           - -1 faild >0 send num
	//int32_i ioctcpconnection_push(ioctcpconnection* tcpcon, char* pdata, uint32_i len);

	// @function 发送
	// @param    ioctcpconnector*  - tcpcontor
	// @return   int32_i           - -1 faild >0 net send num
	//int32_i ioctcpconnection_send(ioctcpconnection* tcpcon);

	// @function 网络发送
	// @param    ioctcpconnector*  - tcpcontor
	// @return   int32_i           - succ num
	//int32_i ioctcpconnection_dosend(ioctcpconnection* tcpcon);

	// @function 发送缓冲区满 延时回调
	// @param    uint16_i          - evtid
	// @param    void*             - tcpcon
	// @return   void*             -
	//void* ioctcpconnection_resend(uint16_i evtid, void* tcpcon);

	// @function net recive data
	// @param    ioctcpconnector*  - tcpcontor
	// @return   int32_i           - -1 faild >0 net send num
	//int32_i ioctcpconnection_read(ioctcpconnection* tcpcon);

	// @function net recive data
	// @param    ioctcpconnector*  - tcpcontor
	// @return   int32_i           - -1 faild >0 net send num
	//void ioctcpconnection_checklastrecv(ioctcpconnection* tcpcon, uint64_i nowtime,  uint32_i linkchecktime);

	// @function get hashmap mood
	// @param    void key*      - key
	// @return   void -
	//uint32_i itcpserver_hashcode(void* key);

	// @function equal
	// @param    void*          - key1
	// @param    char*          - key2
	// @return   int32_i        - <返回-1 =返回0 大于返回1
	//int32_i itcpserver_equals(void* key1, void* key2);

	// @function 引用计数增加回调函数
	// @param    void*          - val
	// @return   void           -
	//void itcpserver_addref(void* val);

	// @function 引用计数减少回调函数
	// @param    void*          - val
	// @return   void           -
	//void itcpserver_delref(void* val);

	// @function 创建TCP服务
	// @param    short int*     - port
	// @param    char*          - ip
	// @return   void           -
	itcpserver *itcpserver_create(iocsystem *lsys, unsigned short int port, const char *ip, uint32_i linkmax, itcpserver_callback *lcallback);

	// @function tcp server wait(epoll)
	// @param    itcpserver*  - server object
	// @param    int32_i        - timeout msec -1 not, > 0 time
	// @return   void -
	int32_i itcpserver_wait(itcpserver *tcpser, int32_i timeout);

	// @function tcp receive 应答
	// @param    itcpserver*  - server object
	// @param    int32_i        - epoll size
	// @return   void           -
	//void itcpserver_answer(itcpserver* tcpser, uint32_i esize);

	// @function 登录成功设置
	// @param    itcpserver*  - server object
	// @param    uint32_i       - connfd
	// @return   void -
	Boolean itcpserver_loginsuccess(itcpserver *tcpser, uint32_i connfd);

	// @function tcp accept
	// @param    itcpserver*  - server object
	// @return   void -
	int32_i itcpserver_accept(itcpserver *tcpser, iendpoint* addr/*传出参数*/);

	// @function ioctcpreaddata
	// @param    itcpserver*  - server object
	// @param    int32_i        - connfd
	// @return   void           -
	void itcpserver_read(itcpserver *tcpser, uint32_i connfd);

	// @function tcp receive
	// @param    itcpserver*  - server object
	// @param    int32_i        - connfd
	// @return   ioctcpconnector- tcpcon对象
	//ioctcpconnection* itcpserver_getconntion(itcpserver* tcpser, uint32_i connfd);

	// @function itcpserver_send
	// @param    itcpserver*  - server object
	// @param    int32_i        - connfd
	// @param    char*          - pdata
	// @param    int32_i        - datalen
	// @return   void           -
	int32_i itcpserver_send(itcpserver *tcpser, uint32_i connfd, char *pdata, uint32_i len);

	// @function itcpserver_dosend
	// @param    itcpserver *tcpser - TCP 服务对象
	// @param    uint32_i      connfd - 连接套接字
	// @return   int32_i        - 0成功 -1断开 -2重发 -3错误
	int32_i itcpserver_dosend(itcpserver *tcpser, uint32_i connfd);

	// @function ioctcpstopcheck
	// @param    itcpserver*  - server object
	// @return   void           - 守护线程函数
	int32_i itcpserver_guard(itcpserver *tcpser);

	// @function foreach_tcp_key
	// @param    void*          - key
	// @param    rev*           - rev
	// @return   void           - 获取key回调函数
	//void itcpserver_key_callback(void* key, void* rev);

	// @function foreach_tcp_stop
	// @param    ioclistnode*   - node
	// @param    rev*           - arg
	// @return   void           - 轮询回调函数
	//void itcpserver_guard_callback(ioclistnode* node, void *arg);

	// @function 关闭tcp服
	// @param    itcpserver*  - tcp server
	// @return   void           -
	void itcpserver_stop(itcpserver *tcpser);

	// @function 关闭回调函数
	// @param    ioclistnode*   - node
	// @param    rev*           - itcpserver*
	// @return   void           -
	//void itcpserver_stop_callback(ioclistnode *node, void *arg);

#ifdef __cplusplus
}
#endif

#endif

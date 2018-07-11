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

#ifndef OC_IKCPSERVER_H
#define OC_IKCPSERVER_H

#include "ikcpproto.h"
#include "inet.h"
#include <common/idatetime.h>
#include <common/ievent.h>
#include <common/isystem.h>

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct IKCPCB ikcpcb;
	typedef uint32_i kcp_conv_t;

	typedef struct IOCKCPCONNECTION ikcpconnection;
	typedef struct IOCKCPSERVER ikcpserver;
	typedef struct IOCKCPSERVER_CALLBACK ikcpserver_callback;

	//前置预定义
	typedef struct IOCEV_LISTEN iocevlisten;
	typedef struct IOCHASHMAP iochashmap;
	typedef struct IOCKCPDATA iockcpdata;

#define IOCKCPSERVER_MAXLINK 50000

	struct IOCKCPCONNECTION
	{
		kcp_conv_t _conv;		// kcp connection id
		uint32_i _ref;			// kcp connection 引用计数器
		uint64_i _lastrecv;		// kcp connnection 最后一个数据包到达时间
		ikcpcb *_lkcp;			// kcp connection handle
		iocevlisten _evtlisten; // kcp event listen handle of object
		ikcpserver *_parent;	// kcp server object
		iendpoint _remote;		// kcp connection remote address
		int32_i _ntimeout;		// kcp timeout count
		uint8_i _status;		// kcp connection status in IOCCONNECTION_STATUS in "inet.h"
	};

	struct IOCKCPSERVER_CALLBACK
	{
		resolve_proc _resolve; // kcp split data packet function
		kcpapt_proc _accept;   // kcp accept process function
		close_proc _close;	 // kcp close connection function
	};

	struct IOCKCPSERVER
	{
		uint32_i _udpfd;							//udp socket
		int32_i _recvmax;							//kcp recv buffer queue max
		int32_i _sendmax;							//kcp send buffer queue max
		int32_i _maxlink;							//kcp client max link
		uint32_i _udpclock;							// udp clock
		iendpoint _udpendpoint;						// dup listen address
		iocepoll *_udpep;							// udp epoll handle
		iocworkerid _idgenerate;					//kcp id generater (id 生成器)
		iochashmap *_connectors;					//kcp connects manage (连接管理)
		char _udpdata[UDP_CONNECT_RECV_BUUFER_MAX]; // udp read data buffer
		uint8_i _shutdown;							// server stop 0.runing 1.stop
		iochashmap *_iperrs;						// record err counts of per ip.
		iocsystem *_system;							// system object
		ikcpserver_callback _callback;
	};

	struct IOCKCPDATA
	{
		void *_arg;
		uint32_i _conv;
		char _data[UDP_PACKET_MAXLENGTH];
		int32_i _datalen;
		iendpoint _udpremote;
	};

	// @function 创建KCP连接对象
	// @param    ikcpserver    - kcp server
	// @param    iendpoint     - address
	// @return   ikcpconnection* - kcp connection
	ikcpconnection *kcpconnection_create(ikcpserver *lsrv, const iendpoint udpremote);

	//@function 关闭KCP连接对象
	void kcpconnection_close(ikcpconnection *lconn);

	// @function 释放KCP连接对象
	// @param    ikcpconnection*    - kcp connection object
	// @return   void-
	void kcpconnection_release(ikcpconnection *lconn);

	//@function 维护connection状态数据
	//@param   ikcpconnection*   - kcp connection object
	//@param   uint32_i               - 内部时钟
	void kcpconnection_update(ikcpconnection *lconn, uint32_i clock);

	//@function kcp 数据整理
	//@param  lsrv   KCP Server 对象
	//@param  id     kcp连接ID
	//@param  revmax 最大接收长度
	//@param  char*    udp data info
	//@param   uint32_i	  udp data length
	//@return  void
	//@explain   Internal call functions do not require header file declarations
	void kcpconnection_input(ikcpserver *lsrv, uint32_i id, const int32_i revmax, char *udpdata, uint32_i bytes, const iendpoint *udpremote);

	//@function kcp连接对象更新状态时间
	//@param  ikcpconnection*   - 连接对象
	//@explain   Internal call functions do not require header file declarations
	//void kcpconnection_recvtts(ikcpconnection *lconn);

	//@function 读取kcp整理后kcp缓冲区中的数据
	//@param     ikcpconnection*	-	连接对象
	//@return	 void
	//@explain   Internal call functions do not require header file declarations
	//void kcpconnection_recv(ikcpconnection *lconn);

	//@function
	//@param ikcoconnection*  - 连接对象
	//@param int32                  -  限制发送缓冲区最大限制 单位队列个数   单节点不到与 一个mtu
	//@param char*                  - 发送的数据
	//@param int32_i			   - 发送数据的字节数
	int32_i kcpconnection_send(ikcpconnection *lconn, const int32_i sndmax, char *data, int32_i bytes);

	//@function 释放连接对象的拥有权，如果引用为0，删除
	//@param  需要释放的连接对象
	//@return void -
	void kcpconnection_subref(void *arg);

	//@function 清空连接超时计数
	//@param 连接对象
	void kcpconnection_clear_timeout(ikcpconnection *lconn);

	// @function 创建KCP服务
	// @param    lsys    -system对象
	// @param    addr    -listen ip address
	// @param    port    -listen port
	// @param    int32_i - server send queue max
	// @param    int32_i - server recv queue max
	// @return   ikcpserver* - server object
	ikcpserver *kcpserver_create(iocsystem *lsys, const char *addr, const int32_i port, const int32_i sndmax, const int32_i rcvmax);

	//@function 服务绑定回调函数
	//@param    disconnect_proto   主动断开连接协议生成函数
	//@param    proto_proc  分解协议数据包回调函数
	//@param    accept_proc 接受连接处理回调函数
	//@param    warncall    连接数据异常警告回调函数
	//@param    closecall   连接关闭回调函数
	//@return   void
	void kcpserver_bindcallback(ikcpserver *lsrv, ikcpserver_callback *lcallback);

	//@function kcp server release
	//@param    ikcpserver*    kcpserver object
	//@return   void
	void kcpserver_release(ikcpserver *lsrv);

	//@function kcp server stop
	//@param  ikcpserver* kcpserver object
	//@return void
	void kcpserver_stop(ikcpserver *lsrv);

	// @function kcp server wait(epoll)
	// @param    ikcpserver*    - server object
	// @param    int32_i        - timeout msec -1 not, > 0 time
	// @return   int32_i        - -1.shutdown,  -2.timeout, -3.error
	int32_i kcpserver_wait(ikcpserver *lsrv, int32_i timeout);

	// @function kcp server 维护逻辑
	// @param    ikcpserver*  kcp服务对象
	// @return   void
	void kcpserver_guard(ikcpserver *lsrv);

	// @function kcp server accept
	// @param    ikcpserver*    - kcp server object
	// @param    iendpoint      - address
	// @param    ikcpconnection - kcp connection out
	// @return   int32_i - 0.Success -1.memory full -2.通知客户端失败
	int32_i kcpserver_accept(ikcpserver *lsrv, iendpoint clientaddr, ikcpconnection **ltconn);

	// @function 处理UDP接收过来的数据
	// @param    ikcpserver    - kcp server object
	// @param    int32_i       -   数据长度
	// @param    iendpoint     -  客户端过来的地址
	// @return   void -
	void kcpserver_udpdata_proc(ikcpserver *lsrv, int32_i datalength, const iendpoint *udpremote);

	// @function read udp data
	// @param    ikcpserver*    - server object
	// @return   void -
	void kcpserver_udprecv(ikcpserver *lsrv);

	//@function get connection handle
	//@param ikcpserver* 	-	server object
	//@return ikcpconnection*	- connection object
	ikcpconnection *kcpserver_getconnect(ikcpserver *lsrv, uint32_i conv);

	// @function wirte udp data
	// @param    ikcpserver*    - server object
	// @param    iendpoint*      - object addrsss
	// @param    char*          - write buffer
	// @param    int32_i        - write data length
	// @return   void -
	int32_i kcpudpsend(const ikcpserver *lsrv, const iendpoint *udpremote, char *buf, int32_i len);

#ifdef __cplusplus
}
#endif

#endif

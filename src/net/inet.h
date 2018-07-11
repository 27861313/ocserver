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

#ifndef IOC_INET_H
#define IOC_INET_H

#include "iepoll.h"
#include "ikcp.h"
#include <common/iinc.h>
#include <common/iworker.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
//#include "itcpserver.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define HASHMAPSIZE 200
#define RINGBUFSIZE 4096

#define IP_ADDR 20
#define IP_BUF IP_ADDR + 1

#define UDP_PACKET_MAXLENGTH 1080 //UPD mtu 默认最大限制 576 (576-8-20 - 8) * 2
#define UDP_BUFFER_SIZE 1024 * 8

#define UDP_CONNECT_RECV_BUUFER_MAX 1080 << 2

#define IOCNET_HEARTBEAT_TIMEOUT_TIME 1000 // 网络连接心跳超时时间(毫秒)
#define IOCNET_HEARTBEAT_TIMEOUT_COUNT 5   // 网络连接心跳超时次数

#define IOCTCP_QUEUESIZE 16

//获取端口信息
#define IOCTCP_TOPORT(v)    \
	{                       \
		return v->sin_port; \
	}
//获取地址信息
#define IOCTCP_TOIP(v)                        \
	{                                         \
		return inet_ntoa(v->sin_addr.s_addr); \
	}

	//地址信息数据结构
	typedef struct IOCENDPOINT iendpoint;
	//
	typedef struct itcpserver itcpserver;
	//回调函数定义
	typedef struct IOCACCEPT_KCPMSG ikcpaptmsg;
	typedef struct IOCACCEPT_TCPMSG itcpaptmsg;
	typedef struct IOCWARNMSG iwarnmsg;
	typedef struct IOCCONNMSG iconnmsg;
	typedef struct IOCRESOLVMSG iresolvmsg;
	typedef struct IOCSEND_DATA iocsenddata;

	enum IOCCONNECTION_STATUS
	{
		IOC_NETCONN_NOMAL = 0, // 正常状态
		IOC_NETCONN_LINK  = 1, // 已连接状态
		IOC_NETCONN_KLEEP = 2, // 心跳检测状态
		IOC_NETCONN_CLOSE = 3, // 连接已关闭状态
	};

	struct IOCSEND_DATA
	{
			char *_pdata;  // 数据
			uint32_i _len; // 长度
    };


	struct IOCENDPOINT
	{
		uint32_i _port;
		struct sockaddr_in _sckaddr;
		char _addr[IP_BUF];
	};

	/*struct IOCACCEPTEDMSG
	{
		void *_lsrv;
		iendpoint _clientaddr;
	};*/

	/*struct IOCACCEPTMSG
	{
		void *_lsrv;
		uint32_i _requestnum;
	};*/

	struct IOCACCEPT_KCPMSG
	{
		void *_lsrv;
		iendpoint _clientaddr;
	};

	struct IOCACCEPT_TCPMSG
	{
		void *_lsrv;
		uint32_i _irequest;
	};

	struct IOCWARNMSG
	{
		void *_lsrv;
		uint32_i _connid;
		iendpoint _clientaddr;
	};

	struct IOCCONNMSG
	{
		void *_arg;
		uint32_i _connid;
	};

	struct IOCRESOLVMSG
	{
		void *_arg;
		uint32_i _connid;
		char *_data;
		uint32_i _datalen;
	};

	typedef int32_i (*resolve_proc)(iresolvmsg msg);
	typedef void (*kcpapt_proc)(ikcpaptmsg msg);
	typedef void (*tcpapt_proc)(itcpaptmsg msg);
	typedef void (*conn_proc)(iconnmsg msg);
	typedef void (*read_proc)(iconnmsg msg);
	typedef void (*close_proc)(iconnmsg msg);

	// @function create_tcpfd
	// @return   uint32_i     - tcpfd
	uint32_i ioctcpfd_create();

	int32_i ioctcp_connect(uint32_i fd, iendpoint remote);

	// @function sockaddr_int size
	// @param    iendpoint    - 地址信息
	// @return   socklen_t    - 数据结构大小
	socklen_t iocnet_getsockaddrsize(const iendpoint *addrs);

	// @function 设置IO复用
	// @param    int32_i    - 套接字
	// @return   int32_i    - 0.success  -1.fail
	int32_i iocnet_setreuseaddr(int32_i sck);

	// @function 设置为非阻塞
	// @param    int32_i    - 套接字
	// @return   int32_i    - 0.success  -1.fail
	int32_i iocnet_setnonblock(int32_i sck);

	// @function 关闭套接字
	// @param    int32_i    - 套接字
	// @return   int32_i    - 0.success  -1.fail
	int32_i iocnet_close(int32_i sck);

	// @function sockaddr_in to iendpoint
	// @param    iendpoint     -  out
	// @param    socketaddr_in - in
	// @return   void          -
	void iocnet_toaddr(iendpoint *outaddr, const struct sockaddr_in *inaddr);

	// @function 绑定套接字
	// @param    int32_i        - 套接字
	// @param    iendpoint     - address
	// @return   int32_i        - 0.success  -1.fail
	int32_i iocnet_bind(int32_i sck, iendpoint addr);

	// @function 绑定tcp套接字
	// @param    int32_i        - 套接字
	// @param    char*          - ip
	// @param    short int      - port
	// @return   int32_i        - 0.success  -1.fail
	int32_i iocnet_tcpbind(itcpserver *tcpser, const char *addr, unsigned short int port);

	// @function 监听套接字
	// @param    int32_i    - 套接字
	// @param    int32_i    - backlog size
	// @return   int32_i    - 0.success  -1.fail
	int32_i iocnet_listen(int32_i sck, int32_i backlog);

	// @function tcp accept
	// @param    int32_i      - 套接字
	// @param    iendpoint* - out address
	// @return   int32_i      - 0.success  -1.fail
	int32_i iocnet_accept(int32_i sck, iendpoint *addrs);

	// @function read socket data
	// @param    int32_i    - 套接字
	// @param    void*       - out buffer
	// @param    int32_i    - out buffer size
	// @return   int32_i    - read bytes
	int32_i iocnet_recv(int32_i sck, void *buf, int32_i size);

	// @function send socket data
	// @param    int32_i    - 套接字
	// @param    void*        - in buffer
	// @param    int32_i    -  length
	// @return   int32_i    -  send bytes  <= 0 error
	int32_i iocnet_send(int32_i sck, const void *buf, int size);

	// @function UDP read socket data
	// @param    int32_i    - 套接字
	// @param    void*      -  out buffer
	// @param    int32_i    - out buffer size
	// @param    iendpoint* - out address
	// @return   int32_i    - read bytes  <= 0 error
	int32_i iocnet_recvfrom(int32_i sck, void *buf, int size, const iendpoint *addrs);

	// @function UDP send data
	// @param    int32_i    - 套接字
	// @param    void*      - in data
	// @param    int32_i    - in data size
	// @param    iendpoint* - object address
	// @return   int32_i    -  send bytes >= 0 error
	int32_i iocnet_sendkeepon(int32_i sck, void *buf, int size, const iendpoint *addrs);

	// @function send socket data
	// @param    int32_i    - 套接字
	// @param    char*      - ip
	// @param    short int  - port
	// @return   int32_i    - >0成功 <0失败
	int32_i iocnet_connect(int32_i sck, const char *ip, unsigned short int port);



#ifdef __cplusplus
}
#endif

#endif

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



#ifndef OC_ITCPCLIENT_H
#define OC_ITCPCLIENT_H

#include <errno.h>                                                                           
#include <fcntl.h>
#include <netinet/in.h>                                                                      
#include <signal.h>                                                                          
#include <string.h>
#include <sys/socket.h>                                                                      
#include <unistd.h>
#include <common/iatom.h>
#include <common/iinc.h>
#include <common/iringbuffer.h>
#include <common/iqueue.h> 
#include <common/ievent.h>
#include <common/ihashmap.h>
#include <common/ilist.h>

#include "inet.h"
#include "itcpserver.h"




#ifdef __cplusplus                                                                           
extern "C"                                                                                   
{
#endif

#define IOCTCP_QUEUESIZE 16

typedef struct IOCTCPCLIENTCONNECTION ioctcpclientconnection;
typedef struct IOCTCPCLIENT ioctcpclient;
//typedef struct IOCSYSTEMMSG ioctcpclient;
//typedef struct IOCCLIENTDATA iocclidata;


/*
struct IOCCLIENTDATA                                                                      
{                                                                                        
		char *_pdata;  // 数据                                                               
		uint32_i _len; // 长度                                                               
};
*/

/*
struct IOCSYSTEMMSG
{
		uint32_i connfd;
		ioctcpclient* tc;
}
*/
struct IOCTCPCLIENTCONNECTION
{
		uint8_i _status;        // 套接字状态
		uint32_i _netid;        // connfd
		uint32_i _ref;          // 引用计数
		iendpoint _addr;        // 连接信息
		iocringbuffer *_rdb;    // 读环形缓冲区
		iocringbuffer *_sdb;    // 发缓冲区
		iocqueue *_sque;        // 发送数据队列
		//ioctcpclient* _parent;  // 管理类 
		iocevlisten _evtlisten; // event listen handle of object
};


struct IOCTCPCLIENT
		{
				uint8_i _shutdown;          // server stop 0.runing 1.stop
				iocepoll *_tcpep;           // epoll iochashmap *_netconhashmap; 
				iochashmap *_netconhashmap; // 存放cliconn的hashmap
				iocsystem *_system;
				itcpserver_callback _callback;
		};


//ioctcpclientconnection* ioctcpclientconnection_create(ioctcpclient* tcpcli, const char *ip, unsigned short int port);
//void ioctcpclientconnection_release(ioctcpclientconnection* tcc);
//int32_i ioctcpclientconnection_dosend(ioctcpclientconnection *tcc, void (*itcpclient_send_delay)(uint32_i connfd));
//int32_i ioctcpclientconnection_send(ioctcpclientconnection *tcc, void (*itcpclient_send_delay)(uint32_i connfd));
//int32_i ioctcpclientconnection_read(ioctcpclientconnection *tcc, resolve_proc splitcall);





// @function 创建tcpclient
// @param    iocsystem*          - system                                                
// @return   void                - 
ioctcpclient* ioctcpclient_create(iocsystem* lsys, itcpserver_callback *lcallback);

// @function 连接服务器
// @param    ioctcpclient*       - tc                                                
// @param    char*               - ip                                                
// @param    short int           - port                                                
// @return   ioctcpclientconnection* - 失败NULL
ioctcpclientconnection* ioctcpclient_connect(ioctcpclient* tc, const char *ip, unsigned short int port);

// @function 网络发送数据
// @param    ioctcpclient*       - tc                                                
// @param    uint32_i            - connfd                                                
// @param    ioctcpclient_send_delay  - callbak 
// @return   int32_i             - -1失败或延迟发送 其他发送字节数 
int32_i ioctcpclient_dosend(ioctcpclient *tc, uint32_i connfd, void (*ioctcpclient_send_delay)(ioctcpclient *tc, uint32_i connfd));

// @function 发送数据到发送队列
// @param    ioctcpclient*       - tc                                                
// @param    uint32_i            - connfd                                                
// @param    char*               - data 
// @param    uint32_i            - len 
// @return   int32_i             - -1失败 其他发送字节数 
int32_i ioctcpclient_send(ioctcpclient* tc, uint32_i connfd, char *pdata, uint32_i len);

// @function 读数据
// @param    ioctcpclient*       - tc                                                
// @param    uint32_i            - connfd                                                
// @return   void                -  
void ioctcpclient_read(ioctcpclient *tc, uint32_i connfd);

// @function 守护线程函数
// @param    ioctcpclient*       - tc                                                
// @return   void                -  
int32_i ioctcpclient_guard(ioctcpclient *tc);

// @function 关闭
// @param    ioctcpclient*       - tc                                                
// @return   void                -  
void ioctcpclient_stop(ioctcpclient *tc);



#ifdef __cplusplus
}
#endif

#endif

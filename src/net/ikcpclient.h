/*
 * Created on Thu Jun 14 2018
 *
 * The MIT License (MIT)
 * Copyright (c) 2018 korialuo
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

#ifndef OC_IKCPCLIENT_H
#define OC_IKCPCLIENT_H

#include "inet.h"
#include <common/iworker.h>
#include <common/ievent.h>
#include <common/ithread.h>
#include <common/ihashmap.h>
#include <sys/select.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct IOCKCPCLIENT         iockcpclient;
    typedef struct IOCKCPCLIENTCONN     iockcpclientconn;

    struct IOCKCPCLIENTCONN
    {
        ikcpcb* _kcpcb;              // kcp对象
        iockcpclient* _parent;       // kcpclient 对象
        iendpoint _remote;           // 远端地址
        int32_i _sckfd;              // 套接字
        conn_proc _conncb;           // 连接回调
        resolve_proc _resolvecb;     // 数据回调
        close_proc _closecb;         // 关闭回调
        int32_i _status;             // 状态
        int32_i _ref;                // 引用计数
    };

    struct IOCKCPCLIENT
    {
        fd_set          _fdset;           // fdset
        int32_i         _maxfd;           // 
        iocthread*      _clithd;          // 客户端线程
        iochashmap     *_conns;           // 所有客户端连接
        int8_i          _shutdown;        // 关闭标记
    };

    //@function 创建KCPClient
    //@return   KCPClient对象
    iockcpclient* iockcpclient_create();

    //@function 关闭KCPClient
    //@param    lcli KCPClient对象
    //@return   void
    void iockcpclient_stop(iockcpclient* lcli);

    //@function 释放KCPClient
    //@param    lcli KCPClient对象
    //@return   void
    void iockcpclient_release(iockcpclient* lcli);

    //@function 创建连接
    //@param    lcli KCPClient对象
    //@param    conncb 连接回调
    //@param    resolvcb 数据回调
    //@param    closecb 关闭回调
    //@param    laddr  远端地址
    //@param    port   远端端口
    //@return   kcp连接ID
    uint32_i iockcpclientconn_create(iockcpclient* lcli, conn_proc conncb, resolve_proc resolvcb, close_proc closecb, const char* laddr, const int32_i port);
    
    //@function 发送数据
    //@param    lcli KCPClient对象
    //@param    id 连接ID
    //@param    data 数据指针
    //@param    len  数据长度
    //@return   int32_i -1.错误 >0.发送字节数
    int32_i iockcpclientconn_send(iockcpclient* lcli, uint32_i id, const char* data, int32_i len);

    //@function 接收数据
    //@param    lcli KCPClient对象
    //@param    id 连接ID
    //@param    src 源数据指针
    //@param    srclen 源数据长度
    //@param    dst 目标数据指针
    //@param    dstlen 目标数据长度
    //@return   int32_i   <0. EAGAIN  >0. 实际长度
    int32_i iockcpclientconn_recv(iockcpclient* lcli, uint32_i id, const char* src, int32_i srclen, char* dst, int32_i dstlen);

    //@function 关闭KCP连接
    //@param    lcli KCPClient对象
    //@param    id   kcp连接ID
    //@return   bool
    Boolean iockcpclientconn_close(iockcpclient* lcli, uint32_i id);

    //@function 减引用计数
    void iockcpclientconn_subref(void* arg);
    
    //@function 释放连接
    void iockcpclientconn_release(iockcpclientconn* lconn);

#ifdef __cplusplus
}
#endif

#endif // OC_IKCPCLIENT_H

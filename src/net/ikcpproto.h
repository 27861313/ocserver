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

#ifndef OC_IKCPPROTO_H
#define OC_IKCPPROTO_H

#include "ikcpserver.h"
#include "inet.h"

#ifdef __cplusplus                                                                           
extern "C"
{
#endif

#define IKCPCONNECT_START_BIT 0xFF
#define IKCPCONNECT_SECOND_BIT 0x66
#define IKCPCONNECT_DATA_SIZE 4
//#define IKCPCONNECT_DATA_TAG
#define IKCPCONNECT_DATA_POS 2
#define IKCPCONNECT_END_BIT 0xFF
#define IKCPCONNECT_PACKSIZE 8

typedef uint32_i kcp_conv_t;

// @function 判断是否是连接申请
// @param   char*      申请数据
// @param   int32_i    数据长度
// @return  Boolean    是否是连接申请 TRUE 是 FALSE 否
Boolean kcpserver_isconnect(const char *data, const int32_i length);

// @function 连成成功后，返回连接返回数据包
// @param    char*		返回的数据信息 IKCPCONNECT_PACKSIZE
// @param    kcp_conv_t	 Kcp connection ID
// @return   void
void kcpserver_connect_finish_pack(char *outdata, const kcp_conv_t conv);

// @functrion 断开连接，数据返回包
// @param     char*  返回数据包
// @param     kcp_conv_t Kcp connection ID
// @return     int32_i       data packet of length
int32_i kcpserver_disconnect_pack(char *outdata, const kcp_conv_t conv);

#ifdef __cplusplus
}
#endif

#endif

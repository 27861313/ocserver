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


/*
 * 性能测试
 * 
 * 测试环境: 
 * CPU : Xeon E5-2680v2
 * 内存: 16GB DDR4
 * 硬盘: SSD
 * 
 * 10,000,000次写入文件总耗时: 15s
 * QPS: 平均66W次
 */


#ifndef IOCSYSLOG_H
#define IOCSYSLOG_H

#include "iinc.h"

#define IOCSYSLOG_SWITCH         1   // 1 - ON, 0 - OFF
#define IOCSYSLOG_MODE_CONSOLE   (int8_i)(1 & 0xff) 
#define IOCSYSLOG_MODE_FILE      (int8_i)((1 << 1) & 0xff)

typedef enum {
    IOCSYSLOG_ALL = 0,
    IOCSYSLOG_DBG,
    IOCSYSLOG_INF,
    IOCSYSLOG_WAR,
    IOCSYSLOG_ERR,
    IOCSYSLOG_FTA,
} IOCSYSLOG_LV;

#ifdef __cplusplus
extern "C"
{
#endif

    // @function 系统日志模块初始化
    // @param    level    日志等级
    // @param    mode     日志模式
    // @param    file     日志文件
    // @return   Boolean  初始化是否成功
    Boolean iocsyslog_init(int8_i level, int8_i mode, const char* file);

    // @function 打印系统日志
    // @param    level    日志等级
    // @param    fmt      格式化字符串
    void iocsyslog_printf(int8_i level, const char* fmt, ...);

    // @function 系统日志模块销毁
    // @return
    void iocsyslog_release();

#ifdef __cplusplus
}
#endif

#endif // OC_ISYSLOG_H
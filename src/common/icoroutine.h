/*
 * Created on Mon Jun 25 2018
 *
 * The MIT License (MIT)
 * Copyright (c) 2018 mr liang yu
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

#ifndef OC_ICOROUTINE_H
#define OC_ICOROUTINE_H

#include "iinc.h"

struct IOC_COROUTINE_ROUSE_MESSAGE
{
    int32_i _coid;
    void *_coarg;
};

struct IOC_COROUTINE_MESSAGE
{
    uint16_i _evtid;
    void *_evtcallback;
    void *_evtarg;
};

enum IOC_COROUTINE_STATE
{
    IOC_COROUTINE_DEAD = 0,
    IOC_CONROUTINE_READY,
    IOC_CONROUTINE_RUNNING,
    IOC_COROUTINE_SUSPEND,
};

typedef struct IOCSCHEDULE iocschedule;
typedef struct IOC_COROUTINE_MESSAGE ioccoroutine_msg;
typedef struct IOC_COROUTINE_ROUSE_MESSAGE ioccoroutine_rousemsg;

typedef void (*icoroutine_func)(iocschedule *, void *ud);

iocschedule *iocschedole_open(int32_i stacksize /*堆栈大小 1024 * 1024*/, int32_i gcctimeout /* 资源空闲回收时间  /sec  秒*/);

void iocscedole_close(iocschedule *);

int32_i ioccoroutine_new(iocschedule *, icoroutine_func, void *ud);

void ioccoroutine_resume(iocschedule *, int32_i id, void *rousearg);

int32_i ioccoroutine_status(iocschedule *, int32_i id);

int32_i ioccoroutine_running(iocschedule *);

void ioccoroutine_yield(iocschedule *);

void ioccoroutine_gcc(iocschedule *);

ioccoroutine_rousemsg *ioccoroutine_rouse_create(int32_i coid, void *arg);

#endif
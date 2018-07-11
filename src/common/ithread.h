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

#ifndef OC_ITHREAD_H
#define OC_ITHREAD_H

#include "iinc.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef __OC_ITHREAD_DEFINED
#define __OC_ITHREAD_DEFINED
#if defined(_MSC_VER) || defined(__BORLANDC__)
#include <process.h>
#include <windows.h>
    typedef HANDLE ithread;
#else
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

    typedef struct
    {
        int _v;
    } pthraed_spinlock_t;

    typedef pthread_t ithread;
    typedef pthread_mutex_t ilock;        //共享锁
    typedef pthread_rwlock_t irwlock;     //读写锁
    typedef pthraed_spinlock_t ispinlock; //自旋锁
    typedef pthread_cond_t icond;         //条件
    typedef pthread_attr_t ithread_attr;  //线程参数
    typedef void *(*ithread_fun)(void *);
    typedef __sigset_t isigset;

    Boolean ispinlock_tryk(ispinlock *lk);

#define ithread_create(thid, tharr, fun, arg) pthread_create(thid, tharr, fun, arg)
#define ilock_init(lk) pthread_mutex_init(lk, NULL)
#define ilock_destory(lk) pthread_mutex_destroy(lk)
#define iwrlock_init(lk) pthread_rwlock_init(lk, NULL)
#define iwrlock_destory(lk) pthread_rwlock_destroy(lk)
#define ilock_k(lk) pthread_mutex_lock(lk)
#define ilock_uk(lk) pthread_mutex_unlock(lk)
#define iwlock_k(lk) pthread_rwlock_wrlock(lk)
#define irlock_k(lk) pthread_rwlock_rdlock(lk)
#define iwrlock_uk(lk) pthread_rwlock_unlock(lk)
#define ispinlock_init(lk) (((ispinlock *)lk)->_v = 0)
#define ispinlock_destory(lk) (((ispinlock *)lk)->_v = 0)
//#define ispinlock_tryk(lk) {if(__sync_lock_test_and_set( &((ispinlock*)lk)->_v, 1 ) == 0) return TRUE; return FALSE;}
#define ispinlock_k(lk)                                             \
    {                                                               \
        for (unsigned k = 0; !ispinlock_tryk((ispinlock *)lk); ++k) \
        {                                                           \
            sched_yield();                                          \
        }                                                           \
    }
#define ispinlock_uk(lk)                             \
    {                                                \
        __sync_lock_release(&((ispinlock *)lk)->_v); \
    }
#define icond_init(lcond) pthread_cond_init(lcond, NULL)
#define icond_destory(lcond) pthread_cond_destroy(lcond)
#define icond_wait(lcond, lk) pthread_cond_wait(lcond, lk)
#define icond_timewait(lcond, lk, ts) pthread_cond_timedwait(lcond, lk, ts)
#define icond_signal(lcond) pthread_cond_signal(lcond)
#define icond_broadcast(lcond) pthread_cond_broadcast(lcond)
#define ithread_exit(p) pthread_exit(p)
#define ithread_self() pthread_self()
#define ithread_kill(thid, signo) pthread_kill(thid, signo)
#define ithread_join(thid) pthread_join(thid, NULL)
#define ithread_detach(thid) pthread_detach(thid)
#define ithread_sigmask(how, newmask, oldmask) pthread_sigmask(how, newmask, oldmask)
#define isigemptyset(mask) sigemptyset(mask)
#define isigaddset(set, signo) sigaddset(set, signo)
#define isigwait(set, sig) sigwait(set, sig)
#define isignal(sig, sigcall) signal(sig, sigcall)
#define isigusr1 SIGUSR1

#if !defined SIG_BLOCK
#define isig_block 0
#else
#define isig_block SIG_BLOCK
#endif

#if !defined SIG_SETMASK
#define isig_setmask 2
#else
#define isig_setmask SIG_SETMASK
#endif

#define ISLEEP(msec)                                \
    {                                               \
        struct timeval tv;                          \
        tv.tv_sec = msec / 1000;                    \
        tv.tv_usec = (msec % 1000) * 1000;          \
        int32_i err;                                \
        do                                          \
        {                                           \
            err = select(0, NULL, NULL, NULL, &tv); \
        } while (err < 0 && errno == EINTR);        \
    }

#endif
#endif

#define THREAD_WAITSIG_DEF             \
    isigset signal_mask, oldmask;      \
    int32_i rc, sig_caught;            \
    isignal(isigusr1, sigusr1_shield); \
    isigemptyset(&oldmask);            \
    isigemptyset(&signal_mask);        \
    isigaddset(&signal_mask, isigusr1);

#define THREAD_WAITSIG_SET(lp)                            \
    rc = ithread_sigmask(isig_block, &signal_mask, NULL); \
    if (rc != 0)                                          \
    {                                                     \
        if (lp != 0)                                      \
        {                                                 \
            lp->_error = OCTH_ER_SIGBLOCK;                \
        }                                                 \
        ithread_exit(NULL);                               \
    }

#define THREAD_WAITSIG(lp)                    \
    rc = isigwait(&signal_mask, &sig_caught); \
    if (rc != 0)                              \
    {                                         \
        if (lp != NULL)                       \
        {                                     \
            lp->_error = OCTH_ER_SIGWAIT;     \
        }                                     \
        ithread_exit(NULL);                   \
    }

#define THREAD_WAITSIG_OLD(lp)                          \
    rc = ithread_sigmask(isig_setmask, &oldmask, NULL); \
    if (rc != 0)                                        \
    {                                                   \
        if (lp != NULL)                                 \
        {                                               \
            lp->_error = OCTH_ER_SIG_SETMASK;           \
        }                                               \
        ithread_exit(NULL);                             \
    }

    struct IOCTHREAD
    {
        ithread _t;
        ithread_fun _fun; //�̻߳ص�
        void *_arg;       //�̲߳���
        uint8_i _error;
        uint8_i _runf;
        uint8_i _status;
        uint8_i _shutdown;
    };

    enum IOCTHREAD_ERRNO
    {
        OCTH_ER_SIGBLOCK,
        OCTH_ER_SIGWAIT,
        OCTH_ER_SIG_SETMASK
    };

    enum IOCTHREAD_STATUS
    {
        OCTH_ST_NOMAL,
        OCTH_ST_SLEEP,
        OCTH_ST_RUN
    };

    typedef struct IOCTHREAD iocthread;
    typedef enum IOCTHREAD_ERRNO iocthread_errno;

    // @function 创建线程
    // @param    ithread_attr     - thread attr
    // @param    ithread_fun      - thread callback function
    // @param    void*              - thread callback arg
    // @return   iocthread       - thread object
    iocthread *iocthread_create(ithread_attr *attr, ithread_fun call, void *arg);

    // @function run thread
    // @param    ithread_attr    - thread object
    // @return   void            -
    void iocthread_run(iocthread *t);

    // @function exit object thread and wait end
    // @param    iocthread    - thread object
    // @return   void            -
    void iocthread_exit(iocthread *t);

#ifdef __cplusplus
}
#endif

#endif
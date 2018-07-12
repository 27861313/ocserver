#include "../common/ithread.h"
#include "../common/isystem.h"
#include "../common/idatetime.h"
#include <stdio.h>
#include <unistd.h>

iocsystem    g_iocsys;

typedef struct {
    uint64_i        _starttime;
    int32_i         _timermsec;
} TIMER_ARG;

void* on_timer(uint16_i evtid, void *arg)
{
    TIMER_ARG* ta = (TIMER_ARG *)arg;
    assert(ta);

    uint64_i curtime = getts_msec();
    fprintf(
        stdout, 
        "on_timer evtid(%d), elapsed time: %d, timermsec: %d\n",
        evtid,
        curtime - ta->_starttime,
        ta->_timermsec
    );
    free(ta);
    return NULL;
}

void* thread1_proc(void *arg)
{
    for (int i = 0; i < 10; i++)
    {
        TIMER_ARG* a = (TIMER_ARG *)malloc(sizeof(TIMER_ARG));
        assert(a);
        a->_timermsec = 1000;
        a->_starttime = getts_msec();
        ioctimerwheel_register(g_iocsys._timers, a->_timermsec, 0, on_timer, a);
        ISLEEP(1000);
    }
    return NULL;
}

void* thread2_proc(void *arg)
{
    for (int i = 0; i < 10; i++)
    {
        TIMER_ARG* a = (TIMER_ARG *)malloc(sizeof(TIMER_ARG));
        assert(a);
        a->_timermsec = 2000;
        a->_starttime = getts_msec();
        ioctimerwheel_register(g_iocsys._timers, a->_timermsec, 0, on_timer, a);
        ISLEEP(1000);
    }
    return NULL;
}

void* thread_listen_proc(void *arg)
{
    return NULL;
}

void* thread_worker_cb(void *arg1, void *arg2)
{
    iocmultithread_t *t = (iocmultithread_t *)arg1;
	iocsystem *lsys = (iocsystem *)arg2;
    waitwork(lsys, t);
    return NULL;
}

void thread_update_cb(void *arg)
{
    static uint64_i elapsed = 0ull;
    static uint64_i lasttime = 0ull;

    ioctimerwheel_tick(g_iocsys._timers, &g_iocsys, 0);
}

void test_timer_main()
{
	initsys(&g_iocsys, "cm-run", 1024, 2, thread_worker_cb, NULL, NULL, thread_update_cb);
    fprintf(stdout, "create timerwheel ok!\n");
    ISLEEP(2000);
    fprintf(stdout, "begin timer...\n");
    ithread ith1, ith2;
    assert(ithread_create(&ith1, NULL, thread1_proc, NULL) == 0);
    assert(ithread_create(&ith2, NULL, thread2_proc, NULL) == 0);
    fprintf(stdout, "press any key to exit..\n");
    getchar();
    releasesys(&g_iocsys);
}
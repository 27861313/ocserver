#include "icoroutine.h"
#include "idatetime.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if __APPLE__ && __MACH__
#include <sys/ucontext.h>
#else
#include <ucontext.h>
#endif

#define DEFAULT_COROUTINE 16
#define DEFAULT_COROUTINE_ARRAY_MIN 4

typedef struct IOCCOROUTINE ioccoroutine;
typedef struct IOCCOROUTINE_NODE ioccoroutine_node;

struct IOCCOROUTINE_NODE
{
    ioccoroutine_node *_prev;
    ioccoroutine_node *_next;
    uint64_i _usedtts;
    ioccoroutine **_co;
};

struct IOCSCHEDULE
{
    char *_stack;
    int32_i _stacksize;
    ucontext_t _main;
    int32_i _nco;
    int32_i _cap;
    int32_i _running;
    int32_i _gcctime;
    int32_i _iary;
    ioccoroutine_node *_head, *_tail;
};

struct IOCCOROUTINE
{
    icoroutine_func _func;
    void *_ud;
    ucontext_t _ctx;
    iocschedule *_sch;
    ptrdiff_t _cap;
    ptrdiff_t _size;
    int32_i _status;
    void *_rsarg;
    char *_stack;
};

ioccoroutine_rousemsg *ioccoroutine_rouse_create(int32_i coid, void *arg)
{
    ioccoroutine_rousemsg *lmsg = malloc(sizeof(ioccoroutine_rousemsg));
    if (lmsg == NULL)
        return NULL;
    lmsg->_coid = coid;
    lmsg->_coarg = arg;
    return lmsg;
}

ioccoroutine *
ioc_co_new(iocschedule *S, icoroutine_func func, void *ud)
{
    ioccoroutine *co = malloc(sizeof(*co));
    co->_func = func;
    co->_ud = ud;
    co->_sch = S;
    co->_cap = 0;
    co->_size = 0;
    co->_rsarg = 0;
    co->_status = IOC_CONROUTINE_READY;
    co->_stack = NULL;
    return co;
}

void ioc_co_delete(ioccoroutine *co)
{
    if (co->_ud != NULL)
    {
        free(co->_ud);
        co->_ud = NULL;
    }

    /* if (co->_rsarg != NULL)
    {
        free(co->_rsarg);
        co->_rsarg = NULL;
    }*/
    co->_rsarg = NULL;
    free(co->_stack);
    free(co);
}

iocschedule *iocschedole_open(int32_i stacksize, int32_i gcctimeout)
{
    iocschedule *S = malloc(sizeof(*S));
    S->_stacksize = stacksize;
    S->_stack = malloc(stacksize);
    assert(S->_stack);
    S->_nco = 0;
    S->_cap = DEFAULT_COROUTINE;
    S->_running = -1;
    S->_gcctime = gcctimeout;
    S->_iary = 1;

    S->_head = S->_tail = malloc(sizeof(ioccoroutine_node));
    assert(S->_head);
    memset(S->_head, 0, sizeof(ioccoroutine_node));
    S->_head->_co = malloc(sizeof(ioccoroutine *) * DEFAULT_COROUTINE);
    memset(S->_head->_co, 0, sizeof(ioccoroutine *) * DEFAULT_COROUTINE);
    S->_head->_usedtts = getdida_msec();
    return S;
}

void iocscedole_close(iocschedule *S)
{
    ioccoroutine_node *idlenode = S->_head;
    while (idlenode)
    {
        int32_i i;
        for (i = 0; i < DEFAULT_COROUTINE; i++)
        {
            ioccoroutine *co = idlenode->_co[i];
            if (co)
            {
                ioc_co_delete(co);
            }
        }
        ioccoroutine_node *freenode = idlenode;
        idlenode = idlenode->_next;
        free(freenode->_co);
        freenode->_co = NULL;
        free(freenode);
    }

    S->_head = S->_tail = NULL;
    free(S->_stack);
    free(S);
}

static ioccoroutine_node *ioccoroutine_get(iocschedule *S, int32_i id, int32_i *iary, int32_i *idx)
{
    *idx = id % DEFAULT_COROUTINE;  //id to array index pos
    *iary = id / DEFAULT_COROUTINE; // id to array pos

    int32_i curarray = 0;
    int32_i mode = (*iary > (S->_iary * 0.5)) ? 1 : 0;
    ioccoroutine_node *idlenode = NULL;
    if (mode == 0)
        idlenode = S->_head;
    else
    {
        curarray = S->_iary;
        idlenode = S->_tail;
    }

    while (idlenode)
    {
        if (curarray == *iary)
        {
            return idlenode;
        }

        if (mode == 0)
            ++curarray;
        else
            --curarray;

        if (mode == 0)
            idlenode = idlenode->_next;
        else
            idlenode = idlenode->_prev;
    }

    return NULL;
}

static ioccoroutine *ioccoroutine_gethandle(iocschedule *S, int32_i id)
{
    int32_i idx = 0, iary = 0; // id to array pos
    ioccoroutine_node *node = ioccoroutine_get(S, id, &iary, &idx);
    if (node == NULL)
        return NULL;
    return node->_co[idx];
}

int32_i ioccoroutine_new(iocschedule *S, icoroutine_func func, void *ud)
{
    ioccoroutine *co = ioc_co_new(S, func, ud);
    if (S->_nco >= S->_cap)
    {
        int32_i id = S->_cap;
        ioccoroutine_node *newnode = malloc(sizeof(ioccoroutine_node));
        if (newnode == NULL)
        {
            ioc_co_delete(co);
            return -1;
        }

        memset(newnode, 0, sizeof(ioccoroutine_node));

        newnode->_co = malloc(sizeof(ioccoroutine *) * DEFAULT_COROUTINE);
        if (newnode->_co == NULL)
        {
            free(newnode);
            ioc_co_delete(co);
            return -1;
        }

        memset(newnode->_co, 0, sizeof(ioccoroutine *) * DEFAULT_COROUTINE);
        newnode->_co[0] = co;
        S->_cap += DEFAULT_COROUTINE;
        ++S->_nco;
        S->_tail->_next = newnode;
        newnode->_prev = S->_tail;
        S->_tail = newnode;
        S->_tail->_usedtts = getdida_msec();
        ++S->_iary;
        return id;
    }
    else
    {
        int32_i i;
        for (i = 0; i < S->_cap; i++)
        {
            int32_i id = (i + S->_nco) % S->_cap;
            int32_i idx = 0, iary = 0;
            ioccoroutine_node *cnode = ioccoroutine_get(S, id, &iary, &idx);
            if (cnode == NULL)
                return -1;
            if (cnode->_co[idx] == NULL)
            {
                cnode->_co[idx] = co;
                cnode->_usedtts = getdida_msec();
                ++S->_nco;
                return id;
            }
        }
    }

    assert(0);
    return -1;
}

static void
mainfunc(void *ptr1, void *ptr2)
{
    iocschedule *S = (iocschedule *)ptr1;
    ioccoroutine *C = (ioccoroutine *)ptr2;
    int32_i id = S->_running;
    C->_func(S, C->_ud);
    ioc_co_delete(C); //这个执行的时间
    int32_i idx = 0, iary = 0;
    ioccoroutine_node *node = ioccoroutine_get(S, id, &idx, &iary);
    assert(node);
    node->_co[idx] = NULL;
    --S->_nco;
    S->_running = -1;
}

void ioccoroutine_resume(iocschedule *S, int32_i id, void *rousearg)
{
    assert(S->_running == -1);
    assert(id >= 0 && id < S->_cap);
    ioccoroutine *C = ioccoroutine_gethandle(S, id);
    if (C == NULL)
        return;
    int32_i status = C->_status;
    switch (status)
    {
    case IOC_CONROUTINE_READY:
        getcontext(&C->_ctx);
        C->_ctx.uc_stack.ss_sp = S->_stack;
        C->_ctx.uc_stack.ss_size = S->_stacksize;
        C->_ctx.uc_link = &S->_main;
        S->_running = id;
        C->_status = IOC_CONROUTINE_RUNNING;
        makecontext(&C->_ctx, (void (*)(void))mainfunc, 2, S, C);
        swapcontext(&S->_main, &C->_ctx);
        break;
    case IOC_COROUTINE_SUSPEND:
        memcpy(S->_stack + S->_stacksize - C->_size, C->_stack, C->_size);
        S->_running = id;
        C->_rsarg = rousearg;
        C->_status = IOC_CONROUTINE_RUNNING;
        swapcontext(&S->_main, &C->_ctx);
        break;
    default:
        assert(0);
    }
}

static void
_save_stack(ioccoroutine *C, char *top, int32_i stacksize)
{
    char dummy = 0;
    assert(top - &dummy <= stacksize);
    if (C->_cap < top - &dummy)
    {
        free(C->_stack);
        C->_cap = top - &dummy;
        C->_stack = malloc(C->_cap);
    }
    C->_size = top - &dummy;
    memcpy(C->_stack, &dummy, C->_size);
}

void ioccoroutine_yield(iocschedule *S)
{
    int id = S->_running;
    assert(id >= 0);
    ioccoroutine *C = ioccoroutine_gethandle(S, id);
    /*if (C->_rsarg != NULL) //释放掉回复协程传入的参数
    {
        free(C->_rsarg);
        C->_rsarg = NULL;
    }*/
    assert((char *)&C > S->_stack);
    _save_stack(C, S->_stack + S->_stacksize, S->_stacksize);
    C->_status = IOC_COROUTINE_SUSPEND;
    S->_running = -1;
    swapcontext(&C->_ctx, &S->_main);
}

int ioccoroutine_status(iocschedule *S, int32_i id)
{
    assert(id >= 0 && id < S->_cap);
    int32_i idx = 0, iary = 0;
    ioccoroutine_node *node = ioccoroutine_get(S, id, &iary, &idx);
    if (node->_co[idx] == NULL)
    {
        return IOC_COROUTINE_DEAD;
    }
    return node->_co[idx]->_status;
}

int32_i ioccoroutine_running(iocschedule *S)
{
    return S->_running;
}

void ioccoroutine_gcc(iocschedule *S)
{
    if (S->_iary <= DEFAULT_COROUTINE_ARRAY_MIN)
        return;
    uint64_i curtimes = getdida_msec();
    uint64_i difftime = curtimes - S->_tail->_usedtts;
    if (difftime <= 0 || difftime < S->_gcctime * 1000 * 1000)
        return;
    for (int32_i i = 0; i < DEFAULT_COROUTINE; i++)
    {
        if (S->_tail->_co[i] != NULL)
            return;
    }

    ioccoroutine_node *idlenode = S->_tail;
    idlenode->_prev->_next = NULL;
    S->_tail = idlenode->_prev;
    S->_cap -= DEFAULT_COROUTINE;
    --S->_iary;
    free(idlenode->_co);
    free(idlenode);
}

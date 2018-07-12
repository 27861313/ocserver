#include "isystem.h"
#include "iatom.h"
#include "iconvert.h"
#include "ievent.h"
#include "isyslog.h"

void *mutits_main(void *arg)
{
	iocsystem *lsys = (iocsystem *)arg;
	while (1)
	{
		if (!iocsystem_waitsys(lsys, SYSTEM_MULTI_WAITSYS_OUT))
			break;
	}

	return NULL;
}

//---------------------------------------------------------------
//function: sysevent_release
//           释放event 资源
//Access:
//           public
//Parameter:
//          [in] void * arg -
//Returns:
//          void -
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
//void sysevent_release(void *arg)
void sysevent_release(void *arg)
{
	iocevent_msg *lmsg = (iocevent_msg *)arg;
	//release event handle ref -----------------
	RELEASE_EVENT_HANDLE(lmsg->_evt);
	//-------------------------------
	memset((char *)lmsg, 0, sizeof(iocevent_msg));
	free(lmsg);
}

#ifndef OC_MULTITHREADING
//Boolean initsys(iocsystem *lsys, const char *name, const int32_i evtsize, iocsystem_callback initfun, iocsystem_callback releasefun, iocsystem_callback updatefun)
Boolean iocsystem_init(iocsystem *lsys, const char *name, int32_i evtsize, const iocsystem_param *satt)
{

	lsys->_e = iocqueue_create(evtsize, sysevent_release);
	//lsys->_timers = ioctimerwheel_create(SYSTEM_TIMER_SLOTMAX);
	lsys->_timers = ioctimerwheel_create(10);
	strncpy(lsys->_name, name, SYSTEM_NAME_MAX - 1);
	lsys->_init = satt->_initfun;
	lsys->_release = satt->_releasefun;
	lsys->_update = satt->_updatefun;
	/* 
	
	*/
	lsys->_main_t = ithread_self();
	lsys->_status = IOC_SYS_RUNING;

	SYSTEM_INIT_FUNCTION_CALL(lsys);
	return TRUE;
}
#else
//Boolean initsys(iocsystem *lsys, const char *name, const int32_i evtsize, const int32_i tnum, multhread_fun worker_callback, iocsystem_callback initfun, iocsystem_callback releasefun, iocsystem_callback updatefun)
Boolean iocsystem_init(iocsystem *lsys, const char *name, iocmultithread_param *multitatt, iocsystem_param *satt)
{
	iocsyslog_init(SYSTEM_LOG_LEVEL, SYSTEM_LOG_MODE, "oclog");
	multitatt->_multitarg = lsys;
	multitatt->_freefun = sysevent_release;
	lsys->_pool_t = iocmultithread_create(multitatt);
	//lsys->_pool_t = iocmultithread_create(tnum, evtsize, sysevent_release, worker_callback, lsys);
	lsys->_dispatchseq = 0;
	lsys->_timers = ioctimerwheel_create(SYSTEM_TIMER_SLOTMAX);
	strncpy(lsys->_name, name, SYSTEM_NAME_MAX - 1);
	lsys->_init = satt->_initfun;
	lsys->_release = satt->_releasefun;
	lsys->_update = satt->_updatefun;

	lsys->_main_t = iocthread_create(NULL, mutits_main, (void *)lsys);
	iocthread_run(lsys->_main_t);
	lsys->_status = IOC_SYS_RUNING;
	SYSTEM_INIT_FUNCTION_CALL(lsys);

	//rouse thread all
	IOCMULTITHREAD_ROUSE(lsys->_pool_t);

	return TRUE;
}

#endif

//free system
//void releasesys(iocsystem *lsys)
void iocsystem_release(iocsystem *lsys)
{
	ioctimerwheel_release(lsys->_timers);
#ifndef OC_MULTITHREADING
	_main_t = 0;
#else
	if (lsys->_main_t != NULL)
		free(lsys->_main_t);
	iocmultithread_release(lsys->_pool_t);
#endif
}

Boolean iocsystem_waitsys(iocsystem *lsys, int32_i timeout)
{
#ifndef OC_MULTITHREADING //单线程模式
	single_dispatch(lsys, timeout);
	SYSTEM_RELEASE_FUNCTION_CALL(lsys);
	lsys->_status = IOC_SYS_NOMAL; //已处于退出状态
	return TRUE;
#else //多线程模式
	while (lsys->_status != IOC_SYS_STOPING && lsys->_status != IOC_SYS_NOMAL)
	{
		//update
		SYSTEM_UPDATE_FUNCTION_CALL(lsys);
		if (timeout > 0)
			ISLEEP(timeout);
	}

	if (lsys->_status == IOC_SYS_STOPING)
	{
		/*wait more thread stoped*/
		for (int16_i ith = 0; ith < lsys->_pool_t->_multitatt._thnum; ith++)
		{
			if (lsys->_pool_t->_multit[ith]->_t > 0)
			{
				IOCMULTITHREAD_ROUSE_ONE(lsys, ith);
				ithread_join(lsys->_pool_t->_multit[ith]->_t);
			}
		}
		return FALSE;
	}
	return TRUE;
#endif
}

#ifndef OC_MULTITHREADING //单线程
void single_dispatch(iocsystem *lsys, int32_i timeout)
{
	iocevent_msg *lmsg = NULL;

	while (lsys->_status != IOC_SYS_STOPING && lsys->_status != IOC_SYS_NOMAL)
	{
		//
		SYSTEM_UPDATE_FUNCTION_CALL(lsys);
		if (iocqueue_count(lsys->_events) == 0) //event empty
			goto _OC_SYSTEM_SINGLE_DISPATCH_END;
		/*pop event*/
		lmsg = iocqueue_pop(lsys->_events);
		if (lmsg == NULL)
			goto _OC_SYSTEM_SINGLE_DISPATCH_END;
		SYSEVENT_RUN(lmsg);
		SYSTEM_RELEASE_MESSAGE(lmsg);
	_OC_SYSTEM_SINGLE_DISPATCH_END:
		if (timeout > 0)
			ISLEEP(timeout);
	}
}
#else
//---------------------------------------------------------------
//function:
//          dispacth_seqcalc 多线程分离均衡计算
//Access:
//           public
//Parameter:
//          [in] iocsystem * lsys -
//Returns:
//          uint32_i -
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
uint32_i dispacth_seqcalc(iocsystem *lsys)
{
	uint32_i newdispatch = AtomicAddFetch(&lsys->_dispatchseq, 1);
	uint32_i seq = newdispatch % lsys->_pool_t->_multitatt._thnum;
	if (seq == 0 && newdispatch > 0)
		CAS(&lsys->_dispatchseq, newdispatch, 0);
	return seq;
}

//
//---------------------------------------------------------------
//function:
//          multi_dispatch 多线程分离器
//Access:
//           public
//Parameter:
//          [in] iocsystem * lsys -
//          [in] iocmultithread_t * lth -
//Returns:
//          void -
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
void multi_codispatch(iocschedule *S, void *arg) //协程，分离器
{
	ioccoroutine_msg *lcormsg = (ioccoroutine_msg *)arg;
	if (lcormsg->_arg_2 != NULL)
	{
		(*(ievent_callback)lcormsg->_arg_2)(v_toint32(lcormsg->_arg_1), lcormsg->_arg_3);
	}
}

void multi_dispatch(iocsystem *lsys, iocmultithread_t *lth)
{
	iocevent_msg *lmsg = NULL;
	while (lsys->_status != IOC_SYS_STOPING && lsys->_status != IOC_SYS_NOMAL)
	{
		lmsg = iocqueue_pop(lth->_e);
		if (lmsg == NULL)
			return;
		AtomicAdd(&lth->_iw, 1);

		if (lmsg->_evt->_evt_tye == OC_EVENT_STOP)
		{
			//exit system
			lsys->_status = IOC_SYS_STOPING;
			lsys->_main_t->_shutdown = 1;
			ithread_kill(lsys->_main_t->_t, isigusr1);
		}
		else if (lmsg->_evt->_evt_tye == OC_EVENT_COROUSE) //唤醒协程
		{
			ioccoroutine_msg *cordata = (ioccoroutine_msg *)lmsg->_evt_arg;
			ioccoroutine_resume(lth->_cos, v_toint32(cordata->_arg_1), cordata->_arg_2);
			free(cordata);
			lmsg->_evt_arg = NULL;
		}
		else
		{
			if (lmsg->_evt->_evt_status == OC_EV_CANCEL)
				goto _ISYSTEM_MULTI_DISPACTCH_RELEASE;

			ioccoroutine_msg *lcormsg = malloc(sizeof(ioccoroutine_msg));
			if (lcormsg != NULL)
			{
				lcormsg->_arg_1 = (void *)(int64_i)lmsg->_evt->_evt_tye;
				lcormsg->_arg_2 = lmsg->_evt->_evt_call;
				lcormsg->_arg_3 = lmsg->_evt_arg;

				int32_i coid = ioccoroutine_new(lth->_cos, multi_codispatch, lcormsg);
				if (coid != -1)
				{
					ioccoroutine_resume(lth->_cos, coid, NULL); //运行协程*/
					goto _ISYSTEM_MULTI_DISPACTCH_RELEASE;
				}
				else
					free(lcormsg);
			}

			SYSEVENT_RUN(lmsg);
		}

	_ISYSTEM_MULTI_DISPACTCH_RELEASE:
		SYSTEM_RELEASE_MESSAGE(lmsg);
	}
}

int32_i iocsystem_waitwork(iocsystem *lsys, iocmultithread_t *lth, int32_i timesec)
{
	int32_i rc = 0;
	while (lsys->_status != IOC_SYS_STOPING && lsys->_status != IOC_SYS_NOMAL)
	{
		ilock_k(&lth->_lk);
		if (iocqueue_count(lth->_e) <= 0 &&
			lth->_icn <= 0 &&
			lsys->_status == IOC_SYS_RUNING)
		{
			if (timesec == 0)
				icond_wait(&lth->_cn, &lth->_lk);
			{
				struct timespec ts;
				clock_gettime(CLOCK_REALTIME, &ts);
				ts.tv_sec += timesec;
				rc = icond_timewait(&lth->_cn, &lth->_lk, &ts);
			}
		}
		if (lth->_icn > 0)
			--lth->_icn;
		ilock_uk(&lth->_lk);
		multi_dispatch(lsys, lth);
		if (timesec > 0 && rc == 0) //如果是等待超时，去查看，是否要回收协程池
			ioccoroutine_gcc(lth->_cos);
	}
	return 0;
}

void iocsystem_waitstop(iocsystem *lsys)
{
	if (lsys->_main_t <= 0)
		return;
	ithread_join(lsys->_main_t->_t);
}

void printfworker_vol(iocsystem *lsys)
{
	for (int16_i ith = 0; ith < lsys->_pool_t->_multitatt._thnum; ith++)
	{
#ifdef _DEBUG_
		printf("thread id(%u) woker vol(%d)\n", lsys->_pool_t->_multit[ith]->_t, lsys->_pool_t->_multit[ith]->_iw);
#endif
		lsys->_pool_t->_multit[ith]->_iw = 0;
	}
}
#endif

//send message
Boolean sendmessage(iocsystem *lsys, iocevlisten *sourlisten, uint16_i event_type, void *arg)
{
	if (lsys->_status != IOC_SYS_RUNING && lsys->_status != IOC_SYS_NOMAL)
		return FALSE;
#ifdef OC_MULTITHREADING //mult thread  2018/06/19 加入自动绑定到线程功能，绑定分配算法还有改进空间
	uint32_i seq = 0;
	if (sourlisten->_evt_thidx == -1)
		seq = dispacth_seqcalc(lsys);
	else
		seq = sourlisten->_evt_thidx;

	if (!iocevsend_event(lsys->_pool_t->_multit[seq]->_e, sourlisten, event_type, arg))
		return FALSE;
	IOCMULTITHREAD_ROUSE_ONE(lsys, seq);
#endif

	return TRUE;
}

//send message no listen object
Boolean sendmessage_extern(iocsystem *lsys, uint16_i event_type, ievent_callback event_call, void *arg, int32_i ithid)
{
	if (lsys->_status != IOC_SYS_RUNING && lsys->_status != IOC_SYS_NOMAL)
		return FALSE;
#ifdef OC_MULTITHREADING //多线程

	uint32_i seq = 0;
	if (ithid == -1)
		seq = dispacth_seqcalc(lsys);
	else
		seq = ithid;
	if (!iocevsend_event_extern(lsys->_pool_t->_multit[seq]->_e, event_type, event_call, arg))
		return FALSE;
	IOCMULTITHREAD_ROUSE_ONE(lsys, seq);
#endif

	return TRUE;
}

int32_i iocsystem_bindth(iocsystem *lsys)
{ //算法优化
	int32_i nrc = 0, minval = 0;

	for (int32_i i = 0; i < lsys->_pool_t->_multitatt._thnum; i++)
	{
		if (lsys->_pool_t->_multit[i]->_ib == 0)
		{
			nrc = i;
			break;
		}
		else if (minval > lsys->_pool_t->_multit[i]->_ib)
		{
			minval = lsys->_pool_t->_multit[i]->_ib;
			nrc = i;
		}
	}
	return nrc;
}

void iocsystem_unbindth(iocsystem *lsys, int32_i ithidx)
{ //算法优化
	if (ithidx < 0 || ithidx >= lsys->_pool_t->_multitatt._thnum)
		return;
	AtomicSub(&lsys->_pool_t->_multit[ithidx]->_ib, 1);
}

/*
int32_i iocsystem_bindth(iocsystem *lsys)
{//算法优化
	int32_i nrc = 0, minval = 0;
	
	for(int32_i i=0;i < lsys->_pool_t->_multitatt._thnum;i++)
	{
		if(lsys->_pool_t->_multit[i]->_ib == 0)
		{
			nrc = i;
			break;
		}
		else if(minval > lsys->_pool_t->_multit[i]->_ib)
		{
			minval = lsys->_pool_t->_multit[i]->_ib;
			nrc = i;
		}
	}
	return nrc;
}

void iocsystem_unbindth(iocsystem *lsys, int32_i ithidx)
{//算法优化
	if(ithidx < 0 || ithidx >= lsys->_pool_t->_multitatt._thnum) return;
	AtomicSub(&lsys->_pool_t->_multit[ithidx]->_ib, 1);
}
*/
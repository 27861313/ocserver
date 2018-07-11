#include "ithread.h"
#include "iatom.h"
#include <assert.h>
#include <string.h>
#include <malloc.h>

Boolean ispinlock_tryk(ispinlock *lk)
{
	if (__sync_lock_test_and_set(&lk->_v, 1) == 0) return TRUE;
	return FALSE;
}

void sigusr1_shield(int32_i signo)
{

}

void* iocthread_entry(void *arg)
{
	iocthread *lpt = (iocthread*)arg;

	THREAD_WAITSIG_DEF
	while (1)
	{
		THREAD_WAITSIG_SET(lpt)


		while (lpt->_runf == 0 && !lpt->_shutdown)
		{
			lpt->_status = OCTH_ST_SLEEP;
			THREAD_WAITSIG(lpt)
		}
		

		AtomicSub(&lpt->_runf, 1);

		THREAD_WAITSIG_OLD(lpt)


		if (lpt->_fun == NULL) continue;
		lpt->_status = OCTH_ST_RUN;
		(*lpt->_fun)(lpt->_arg);
		if (lpt->_shutdown) break;
	}

	return NULL;
}

//---------------------------------------------------------------
//function: 
//          iocthread_create 
//Access:
//           public  
//Parameter:
//          [in] ithread_attr * attr - 
//          [in] ithread_fun call - 
//          [in] void * arg - 
//Returns:
//          iocthread * - 
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
iocthread *iocthread_create(ithread_attr *attr, ithread_fun call, void* arg)
{
	iocthread* pt = malloc(sizeof(iocthread));
	assert(pt);
	pt->_shutdown = 0;
	pt->_runf = 0;
	pt->_status   = OCTH_ST_NOMAL;
	pt->_fun = call;
	pt->_arg = arg;
	if (ithread_create(&pt->_t, attr, &iocthread_entry, pt) != 0)
	{
		free(pt);
		return NULL;
	}

	while (1)  //�߳��Ƿ�׼����
	{
		if (pt->_status != OCTH_ST_NOMAL)
			break;
		ISLEEP(5);  //�ȴ�5����
	}
	
	return pt;
}

//---------------------------------------------------------------
//function: 
//          iocthread_run 
//Access:
//           public  
//Parameter:
//          [in] iocthread * t - 
//Returns:
//          void - 
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
void iocthread_run(iocthread *t)
{
	assert(t);
	AtomicAdd(&t->_runf, 1);
	ithread_kill(t->_t, isigusr1);
}

//---------------------------------------------------------------
//function: 
//          iocthread_exit 
//Access:
//           public  
//Parameter:
//          [in] iocthread * t - 
//Returns:
//          void - 
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
void iocthread_exit(iocthread *t)
{
	assert(t);
	t->_shutdown = 1;
	ithread_kill(t->_t, isigusr1);
	ithread_join(t->_t);
}

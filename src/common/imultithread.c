#include "imultithread.h"
#include "iatom.h"
#include "ievent.h"
#include <syslog.h>

//---------------------------------------------------------------
//function:
//          iocmultithread_create
//Access:
//           public
//Parameter:
//          [in] const int16_i multinum -
//          [in] ithread_fun multfun -
//          [in] void * arg -
//Returns:
//          iocmultithread* -
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
/*iocmultithread *iocmultithread_create(const int16_i multinum,
									  const int16_i evtmax,
									  iocqueue_free_data efree,
									  multhread_fun multfun,
									  void *arg)*/
iocmultithread *iocmultithread_create(const iocmultithread_param *att)
{
	int16_i ith = 0;
	iocmultithread *lmult = (iocmultithread *)malloc(sizeof(iocmultithread));
	if (lmult == NULL)
		return NULL;
	//copy param
	memcpy((char *)&lmult->_multitatt, (char *)att, sizeof(iocmultithread_param));
	lmult->_error = 0;

	lmult->_multit = malloc(sizeof(iocmultithread_t *) * lmult->_multitatt._thnum);
	if (lmult->_multit == NULL)
	{
		free(lmult);
		return NULL;
	}

	for (ith = 0; ith < lmult->_multitatt._thnum; ith++)
	{
		lmult->_multit[ith] = malloc(sizeof(iocmultithread_t));
		if (lmult->_multit[ith] == NULL)
		{
			SYSLOG_ERROR("malloc mult object fail.(exit)");
			exit(0);
		}

		ilock_init(&lmult->_multit[ith]->_lk);
		icond_init(&lmult->_multit[ith]->_cn);
		lmult->_multit[ith]->_icn = 0;
		lmult->_multit[ith]->_iw = 0;
		lmult->_multit[ith]->_ib = 0;
		lmult->_multit[ith]->_e = iocqueue_create(lmult->_multitatt._evtmax, lmult->_multitatt._freefun);
		if (lmult->_multit[ith]->_e == NULL)
		{
			SYSLOG_ERROR("malloc mult create event fail.(exit)");
			exit(0);
		}
	}

	for (ith = 0; ith < lmult->_multitatt._thnum; ith++)
	{
		//create co process
		lmult->_multit[ith]->_cos = iocschedole_open(lmult->_multitatt._costacksize, lmult->_multitatt._cogcc);
		assert(lmult->_multit[ith]->_cos);
		if (ithread_create(&lmult->_multit[ith]->_t, NULL, IOCMULTITHREAD_HANG_NAME, (void *)lmult) != 0)
		{
			SYSLOG_ERROR("malloc mult create thread fail.(exit)");
			exit(0);
		}
	}

	return lmult;
}

//---------------------------------------------------------------
//function:
//          iocmultithread_release
//Access:
//           public
//Parameter:
//          [in] iocmultithread * lmult -
//Returns:
//          void -
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
void iocmultithread_release(iocmultithread *lmult)
{
	int32_i ith;
	for (ith = 0; ith < lmult->_multitatt._thnum; ith++)
	{
		iocqueue_release(lmult->_multit[ith]->_e);
		ilock_destory(&lmult->_multit[ith]->_lk);
		icond_destory(&lmult->_multit[ith]->_cn);
		lmult->_multit[ith]->_icn = 0;
		lmult->_multit[ith]->_t = 0;
		//释放协程资源
		iocscedole_close(lmult->_multit[ith]->_cos);
		//释放线程组资源
		free(lmult->_multit[ith]);
	}
	free(lmult->_multit);
	free(lmult);
}

//---------------------------------------------------------------
//function:
//          iocmultithread_handle
//Access:
//           public
//Parameter:
//          [in] iocmultithread * lmult -
//          [in] ithread crt -
//Returns:
//          iocmultithread_t* -
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
iocmultithread_t *iocmultithread_handle(iocmultithread *lmult, ithread crt)
{
	int32_i ith;
	for (ith = 0; ith < lmult->_multitatt._thnum; ith++)
	{
		if (lmult->_multit[ith]->_t == crt)
			return lmult->_multit[ith];
	}
	return NULL;
}

IOCMULTITHREAD_HANG_CALL
IOCMULTITHREAD_HANG

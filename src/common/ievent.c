#include "ievent.h"
#include "iatom.h"
#include "isystem.h"
#include <assert.h>

//=======================================================================================================================

Boolean iocevlistener_create(iocsystem *lsys, iocevlisten *listener, Boolean isBindThread, Boolean isSync)
{
	assert(listener);
	memset((char *)listener, 0, sizeof(iocevlisten));
	listener->_evt_head = (iocevent *)malloc(sizeof(iocevent));
	if (listener->_evt_head == NULL)
		return FALSE;
	listener->_evt_tail = (iocevent *)malloc(sizeof(iocevent));
	if (listener->_evt_tail == NULL)
	{
		free(listener->_evt_head);
		return FALSE;
	}

	memset((char *)listener->_evt_head, 0, sizeof(iocevent));
	memset((char *)listener->_evt_tail, 0, sizeof(iocevent));

	listener->_evt_head->_evt_next = listener->_evt_tail;
	listener->_evt_issync = isSync;
	//listener->_evt_isbind = isBindThread;
	//listener->_evt_thidx = -1;
	if (isBindThread)
	{
		listener->_evt_thidx = iocsystem_bindth(lsys);
	}

	if (listener->_evt_issync)
		ilock_init((ilock *)&listener->_evt_lk);

	return TRUE;
}

Boolean iocevlistener_register(iocevlisten *listener, void *owner, uint16_i event_type, ievent_callback event_call)
{
	Boolean isInsert = FALSE;
	iocevent *lnew = malloc(sizeof(iocevent));
	if (lnew == NULL)
		return isInsert;
	lnew->_evt = malloc(sizeof(iocevhandle));
	if (lnew->_evt == NULL)
	{
		free(lnew);
		return isInsert;
	}

	lnew->_evt->_evt_status = OC_EV_NOMAL;
	lnew->_evt->_evt_tye = event_type;
	lnew->_evt->_evt_owner = owner;
	lnew->_evt->_evt_call = event_call;
	lnew->_evt->_evt_rf = 1;
	lnew->_evt_next = NULL;
#ifdef OC_MULTITHREADING
	if (listener->_evt_issync)
		iwlock_k(&listener->_evt_lk);
#endif
	iocevent *ln = listener->_evt_head;
	for (;;)
	{
		if (ln == NULL)
		{
			IEVENT_RELEASE_EVENT(lnew);
			goto _EVENT_REIGSTER_END;
		}
		else if (ln != listener->_evt_head)
		{
			if (ln->_evt->_evt_tye == event_type)
			{
				IEVENT_RELEASE_EVENT(lnew);
				goto _EVENT_REIGSTER_END;
			}
		}

		if (ln->_evt_next == listener->_evt_tail)
		{
			ln->_evt_next = lnew;
			lnew->_evt_next = listener->_evt_tail;

			goto _EVENT_REIGSTER_END;
		}

		ln = ln->_evt_next;
	}
_EVENT_REIGSTER_END:
#ifdef OC_MULTITHREADING
	if (listener->_evt_issync)
		iwrlock_uk(&listener->_evt_lk);
#endif
	return isInsert;
}

Boolean iocevlistener_unregister(iocevlisten *listener, uint16_i event_type)
{
	Boolean bresult = FALSE;
#ifdef OC_MULTITHREADING
	if (listener->_evt_issync)
		iwlock_k(&listener->_evt_lk);
#endif
	iocevent *ln = listener->_evt_head;
	for (;;)
	{
		if (ln == NULL)
			goto _EVENT_UNREIGSTER_END;
		else if (ln->_evt_next == listener->_evt_tail)
			goto _EVENT_UNREIGSTER_END;
		if (ln->_evt_next->_evt->_evt_tye == event_type)
		{
			iocevent *lfr = ln->_evt_next;
			ln->_evt_next = lfr->_evt_next;

			iocevhandle *lfrhandle = lfr->_evt;
			lfrhandle->_evt_status = OC_EV_CANCEL;
			lfr->_evt = NULL;
			free(lfr);
#ifdef OC_MULTITHREADING
			if (AtomicSubFetch(&lfrhandle->_evt_rf, 1) == 0)
			{
#else
			if ((lfrhandle->_evt_rf -= 1) == 0)
			{
#endif
				free(lfrhandle);
			}
			bresult = TRUE;
			goto _EVENT_UNREIGSTER_END;
		}

		ln = ln->_evt_next;
	}

_EVENT_UNREIGSTER_END:
#ifdef OC_MULTITHREADING
	if (listener->_evt_issync)
		iwrlock_uk(&listener->_evt_lk);
#endif
	return bresult;
}
iocevhandle *iocevlistener_gethandle(iocevlisten *listener, uint16_i event_type)
{
	iocevhandle *lhresult = NULL;
#ifdef OC_MULTITHREADING
	if (listener->_evt_issync)
		irlock_k(&listener->_evt_lk);
#endif
	iocevent *ln = listener->_evt_head;

	for (;;)
	{
		if (listener->_evt_head == NULL)
			break;

		if (ln == listener->_evt_head)
			goto _EVENT_GETHANDLE_NEXT;
		else if (ln == listener->_evt_tail)
			break;
		else if (ln->_evt->_evt_tye == event_type)
		{
			if (ln->_evt->_evt_status == OC_EV_CANCEL)
				break;
#ifdef OC_MULTITHREADING
			AtomicAdd(&ln->_evt->_evt_rf, 1);
#else
			ln->_evt->_evt_rf += 1;
#endif
			lhresult = ln->_evt;
			break;
		}
	_EVENT_GETHANDLE_NEXT:
		ln = ln->_evt_next;
	}

#ifdef OC_MULTITHREADING
	if (listener->_evt_issync)
		iwrlock_uk(&listener->_evt_lk);
#endif
	return lhresult;
}

void iocevlistener_release(iocsystem *lsys, iocevlisten *listener)
{
#ifdef OC_MULTITHREADING
	if (listener->_evt_issync)
		iwlock_k(&listener->_evt_lk);
#endif
	iocevent *ln = listener->_evt_head;
	for (;;)
	{
		if (ln->_evt_next == listener->_evt_tail)
			break;

		iocevent *lfr = ln->_evt_next;
		ln->_evt_next = lfr->_evt_next;

		iocevhandle *lfrhandle = lfr->_evt;
		lfrhandle->_evt_status = OC_EV_CANCEL;
		lfr->_evt = NULL;
		free(lfr);
#ifdef OC_MULTITHREADING
		if (AtomicSubFetch(&lfrhandle->_evt_rf, 1) == 0)
		{
  
#else
		if ((lfrhandle->_evt_rf -= 1) == 0) 
		{
#endif
			free(lfrhandle);
		}
	}

	memset(listener->_evt_head, 0, sizeof(iocevhandle));
	memset(listener->_evt_tail, 0, sizeof(iocevhandle));
	free(listener->_evt_head);
	free(listener->_evt_tail);
	if (listener->_evt_thidx > 0)
	{
		iocsystem_unbindth(lsys, listener->_evt_thidx);
		listener->_evt_thidx = -1;
	}
#ifdef OC_MULTITHREADING
	if (listener->_evt_issync)
	{
		iwrlock_uk(&listener->_evt_lk);
		iwrlock_destory(&listener->_evt_lk);
	}
#endif
}

//---------------------------------------------------------------
//function:
//          iocevsend_event
//Access:
//           public
//Parameter:
//          [in] iocqueue * dest -
//          [in] iocevlisten * sourlisten -
//          [in] uint16_i event_type -
//          [in] void * event_arg -
//Returns:
//          Boolean -
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
Boolean iocevsend_event(iocqueue *dest, iocevlisten *sourlisten, uint16_i event_type, void *event_arg)
{
	Boolean isSend = FALSE;
	if (sourlisten == NULL)
		return isSend;
	iocevent_msg *lmsg = (iocevent_msg *)malloc(sizeof(iocevent_msg));
	if (lmsg == NULL)
		return isSend;
	lmsg->_evt = iocevlistener_gethandle(sourlisten, event_type);

	if (lmsg->_evt == NULL)
	{
		free(lmsg);
		lmsg = NULL;

		return isSend;
	}

	lmsg->_evt_arg = event_arg;
	if (!iocqueue_push(dest, lmsg))
	{
		RELEASE_EVENT_HANDLE(lmsg->_evt);
		lmsg->_evt = NULL;
		lmsg->_evt_arg = NULL;
		free(lmsg);
		lmsg = NULL;
		return isSend;
	}

	isSend = TRUE;
	return isSend;
}

//---------------------------------------------------------------
//function:
//          iocevsend_event_extern
//Access:
//           public
//Parameter:
//          [in] iocqueue * dest -
//          [in] uint16_i event_type -
//          [in] ievent_callback event_callback -
//          [in] void * event_arg -
//Returns:
//          Boolean -
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
Boolean iocevsend_event_extern(iocqueue *dest, uint16_i event_type, ievent_callback event_callback, void *event_arg)
{
	Boolean isSend = FALSE;
	iocevent_msg *lmsg = (iocevent_msg *)malloc(sizeof(iocevent_msg));
	if (lmsg == NULL)
		return isSend;
	lmsg->_evt = (iocevhandle *)malloc(sizeof(iocevhandle));
	if (lmsg->_evt == NULL)
	{
		free(lmsg);
		lmsg = NULL;
		return isSend;
	}

	lmsg->_evt->_evt_tye = event_type;
	lmsg->_evt->_evt_status = OC_EV_NOMAL;
	lmsg->_evt->_evt_rf = 1;
	lmsg->_evt->_evt_owner = NULL;
	lmsg->_evt->_evt_call = event_callback;
	lmsg->_evt_arg = event_arg;

	if (!iocqueue_push(dest, lmsg))
	{
		RELEASE_EVENT_HANDLE(lmsg->_evt);
		lmsg->_evt = NULL;
		lmsg->_evt_arg = NULL;
		free(lmsg);
		lmsg = NULL;
		return isSend;
	}
	isSend = TRUE;
	return isSend;
}

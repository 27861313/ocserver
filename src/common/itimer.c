#include "itimer.h"
#include "iatom.h"
#include "idatetime.h"
#include "isystem.h"
#include <syslog.h>

ioctimer_lst *iocitimer_lstcreate(int32_i iarray)
{
	ioctimer_lst *ls = calloc(1, sizeof(ioctimer_lst) * (int64_i)iarray);
	for (int32_i ilst = 0; ilst < iarray; ilst++)
	{
		ls[ilst]._head = calloc(1, sizeof(ioctimer_node));
		ls[ilst]._tail = calloc(1, sizeof(ioctimer_node));
		assert(ls[ilst]._head);
		assert(ls[ilst]._tail);
		ls[ilst]._head->_next = ls[ilst]._tail;
		ilock_init(&ls[ilst]._lck);
	}

	return ls;
}

Boolean iocitimer_lstinsert(ioctimer_lst *ls, ioctimer *ltm)
{
	Boolean bresult = FALSE;
	ioctimer_node *lnew = calloc(1, sizeof(ioctimer_node));
	memcpy(&lnew->_timer, (char *)ltm, sizeof(ioctimer));
	ilock_k(&ls->_lck);
	ioctimer_node *ln = ls->_head;
	for (;;)
	{
		if (ln == NULL)
		{
			free(lnew);
			break;
		}
		else if (ln != ls->_head)
		{
			if (ln->_timer._id == ltm->_id)
			{
				free(lnew);
				break;
			}
		}

		if (ln->_next == ls->_tail)
		{
			++ls->_num;
			ln->_next = lnew;
			lnew->_next = ls->_tail;
			bresult = TRUE;
			break;
		}
		ln = ln->_next;
	}

	ilock_uk(&ls->_lck);
	return bresult;
}

Boolean iocitimer_lstremove(ioctimer_lst *ls, int32_i id)
{
	Boolean bresult = FALSE;
	ilock_k(&ls->_lck);
	ioctimer_node *ln = ls->_head;
	for (;;)
	{
		if (ln == NULL)
			break;
		else if (ln->_next == ls->_tail)
			break;
		else if (ln == ls->_head)
		{
			ln = ln->_next;
			continue;
		}
		else if (ln->_next->_timer._id == id)
		{
			--ls->_num;
			ioctimer_node *lfree = ln->_next;
			ln->_next = ln->_next->_next;
			lfree->_next = NULL;
			free(lfree); //, sizeof(ioctimer_node)
			bresult = TRUE;
			break;
		}
		ln = ln->_next;
	}

	ilock_uk(&ls->_lck);
	return bresult;
}

//---------------------------------------------------------------
//function:
//          ioctimerwheel_create
//Access:
//           public
//Parameter:
//          [in] int32_i slotnum -
//Returns:
//          ioctimewheel* -
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
ioctimewheel *ioctimerwheel_create(int32_i slotnum)
{
	ioctimewheel *lwheel = calloc(1, sizeof(ioctimewheel));
	if (lwheel == NULL)
		return NULL;
	lwheel->_slotmax = slotnum;
	lwheel->_slots = iocitimer_lstcreate(slotnum);
	lwheel->_timerseq = 10000;

	if (lwheel->_slots == NULL)
	{
		free(lwheel);
		return NULL;
	}

	return lwheel;
}

//---------------------------------------------------------------
//function:
//          ioctimerwheel_node_release
//Access:
//           public
//Parameter:
//          [in] ioctimer_lst * lst -
//Returns:
//          void -
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
void ioctimerwheel_node_release(ioctimer_lst *lst)
{
	ilock_k(&lst->_lck);
	ioctimer_node *ln = lst->_head;
	for (;;)
	{
		if (ln->_next == lst->_tail)
			break;
		ioctimer_node *lfree = ln->_next;
		ln = ln->_next;
		lfree->_next = NULL;
		free(lfree);
		--lst->_num;
	}
	free(lst->_head);
	free(lst->_tail);
	lst->_head = NULL;
	lst->_tail = NULL;
	ilock_uk(&lst->_lck);
	ilock_destory(&lst->_lck);
}

//---------------------------------------------------------------
//function:
//          ioctimerwheel_release
//Access:
//           public
//Parameter:
//          [in] ioctimewheel * lwh -
//Returns:
//          void -
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
void ioctimerwheel_release(ioctimewheel *lwh)
{
	for (int32_i idx = 0; idx < lwh->_slotmax; idx++)
		ioctimerwheel_node_release(&lwh->_slots[idx]);
	int32_i maxnum = lwh->_slotmax;
	free(lwh->_slots); // sizeof(sizeof(ioctimer_lst)) * maxnum
	free(lwh);
}

//---------------------------------------------------------------
//function:
//          ioctimerwheel_register
//Access:
//           public
//Parameter:
//          [in] ioctimewheel * lwh -
//          [in] int32_i msec -
//          [in] int32_i interval -
//          [in] ievent_callback timecallback -
//          [in] void * timearg -
//Returns:
//          int32_i -
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
int32_i ioctimerwheel_register(ioctimewheel *lwh, int32_i msec, int32_i interval, int32_i ithid, ievent_callback timecallback, void *timearg)
{
	ioctimer tmhanle;
	tmhanle._id = AtomicAdd(&lwh->_timerseq, 1);
	tmhanle._interval = interval;
	tmhanle._event_call = timecallback;
	tmhanle._event_arg = timearg;
	tmhanle._ithid = ithid;

	tmhanle._start_time = getts_msec();

	int32_i ticks = 0;

	int32_i timeout = msec;

	if (timeout < lwh->_slotmax)
		ticks = 1;
	else
		ticks = timeout / lwh->_slotmax;

	int32_i rotation = ticks / lwh->_slotmax;

	int32_i ts = (lwh->_curslot + ticks % lwh->_slotmax) % lwh->_slotmax;

	tmhanle._rotation = rotation;
	tmhanle._time_slot = ts;
	tmhanle._expire = timeout;
	tmhanle._oldrotation = rotation;
	if (!iocitimer_lstinsert(&lwh->_slots[ts], &tmhanle))
		return -1;
	return tmhanle._id;
}

//---------------------------------------------------------------
//function:
//          ioctimerwheel_unregister
//Access:
//           public
//Parameter:
//          [in] ioctimewheel * lwh -
//          [in] int32_i timeid -
//Returns:
//          Boolean -
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
Boolean ioctimerwheel_unregister(ioctimewheel *lwh, int32_i timeid)
{
	int32_i ts = lwh->_curslot;
	if (iocitimer_lstremove(&lwh->_slots[ts], timeid))
		return TRUE;
	for (int isolt = 0; isolt < lwh->_slotmax; isolt++)
	{
		if (isolt == ts)
			continue;
		if (iocitimer_lstremove(&lwh->_slots[isolt], timeid))
			return TRUE;
	}
	return FALSE;
}

//---------------------------------------------------------------
//function:
//          ioctimerwheel_tickone
//Access:
//           public
//Parameter:
//          [in] ioctimer * t -
//Returns:
//          int32_i -
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
int32_i ioctimerwheel_tickone(ioctimer *t)
{
	if (t->_rotation > 0)
	{
		t->_rotation--;
	}
	else
	{
		uint64_i cur_time = getts_msec();
		if (cur_time >= t->_start_time + t->_expire)
		{
			return 1;
		}
	}
	return 0;
}

void ioctimerwheel_tick(ioctimewheel *lwh, iocsystem *lsys, uint64_i usec)
{
	ioctimer_node *lhead, *ltail;
	lhead = ltail = NULL;
	ilock_k(&lwh->_slots[lwh->_curslot]._lck);
	ioctimer_node *ln = lwh->_slots[lwh->_curslot]._head;
	for (;;)
	{
		if (ln == NULL)
		{
			break;
		}
		if (ln->_next == lwh->_slots[lwh->_curslot]._tail)
		{
			break;
		}
		else
		{
			int32_i nresult = ioctimerwheel_tickone(&ln->_next->_timer);
			if (nresult == 1)
			{
				ioctimer_node *curnode = ln->_next;
				ln->_next = curnode->_next;
				curnode->_next = NULL;

				sendmessage_extern(lsys, OC_EVENT_TIMER, curnode->_timer._event_call, curnode->_timer._event_arg, curnode->_timer._ithid);
				if (curnode->_timer._interval > 0)
				{
					if (lhead == NULL)
					{
						lhead = ltail = curnode;
					}
					else
					{
						ltail->_next = curnode;
						ltail = curnode;
					}
				}
				else
					free(curnode);
				continue;
			}
		}

		ln = ln->_next;
	}

	ilock_uk(&lwh->_slots[lwh->_curslot]._lck);
	lwh->_curslot = ++lwh->_curslot % lwh->_slotmax;

	for (;;)
	{
		if (lhead == NULL)
			break;

		int32_i ticks = 0;
		int32_i timeout = lhead->_timer._expire;

		if (timeout < lwh->_slotmax)
			ticks = 1;
		else
			ticks = timeout / lwh->_slotmax;

		int32_i rotation = ticks / lwh->_slotmax;
		int32_i ts = (lwh->_curslot + ticks % lwh->_slotmax) % lwh->_slotmax;

		lhead->_timer._rotation = rotation;
		lhead->_timer._time_slot = ts;

		lhead->_timer._expire = timeout;
		lhead->_timer._oldrotation = rotation;

		if (!iocitimer_lstinsert(&lwh->_slots[ts], &lhead->_timer))
		{
			syslog(LOG_WARNING, "timer rest insert fail.\n");
		}

		ioctimer_node *lnext = lhead->_next;
		free(lhead);
		lhead = lnext;
	}
}

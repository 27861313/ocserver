#include "iqueue.h"
#include "iatom.h"
#include <assert.h>
#include <string.h>
#include <malloc.h> 

//---------------------------------------------------------------
//function: 
//          iocqueue_create 
//Access:
//           public  
//Parameter:
//          [in] unsigned int maxnum - 
//          [in] iocqueue_free_data * freecall - 
//Returns:
//          iocqueue* - 
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
iocqueue* iocqueue_create(uint32_i maxnum, iocqueue_free_data freecall)
{
	iocqueue*  lq = (iocqueue*)malloc(sizeof(iocqueue));
	assert(lq);
	lq->_num = 0;
	lq->_status = OC_QU_NOMAL;
	lq->_wt = 0;
	lq->_rt = 0;
	lq->_max = maxnum;
	lq->_head = malloc(sizeof(iocqueue_node));
	lq->_head->_data = NULL;
	lq->_head->_next = NULL;
	lq->_tail = lq->_head;
	lq->_freecall = freecall;

	return lq;
}

//---------------------------------------------------------------
//function: 
//          iocqueue_pop 
//Access:
//           public  
//Parameter:
//          [in] iocqueue * lq - 
//Returns:
//          void* - 
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
void* iocqueue_pop(iocqueue* lq)
{
	void* lpval = NULL;
#ifdef OC_MULTITHREADING
	if (CAS(&lq->_status, OC_QU_RELEASE, OC_QU_RELEASE))  //���ٱ��
	{
		return NULL;
	}


	iocqueue_node *p = NULL;
	for (;;)
	{
		if (CAS(&lq->_rt, 0, ithread_self()))
		{
			do
			{
				p = lq->_head;

				if (p->_next == NULL) {
					AtomicRelease(&lq->_rt);
					return NULL;
				}
			} while (!CAS(&lq->_head, p, p->_next));

			AtomicSub(&lq->_num, 1);
			lpval = p->_next->_data;
			p->_next->_data = NULL;
			free(p);
			AtomicRelease(&lq->_rt);
			break;
		}
		else
		{
			// this is a good place to yield the thread in case there are more
			// software threads than hardware processors and you have more
			// than 1 producer thread
			// have a look at sched_yield (POSIX.1b)
			sched_yield();
		}
	
	}

#else
	if (lq->_head->_next == NULL) return NULL;
	iocqueue_node *p = lq->_head->_next;
	lq->_head->_next = p->_next;
	--lq->_num;
	lpval = p->_data;
	free(p);
#endif
	return lpval;
}

//---------------------------------------------------------------
//function: 
//          iocqueue_push 
//Access:
//           public  
//Parameter:
//          [in] iocqueue * lq - 
//          [in] void * data - 
//Returns:
//          unsigned char - 
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
Boolean iocqueue_push(iocqueue* lq, void* data)
{
#ifdef OC_MULTITHREADING
	if (CAS(&lq->_status, OC_QU_RELEASE, OC_QU_RELEASE))
	{
		return FALSE;
	}
#endif

	if (lq->_max != 0)
	{
		if (lq->_num >= lq->_max) return FALSE;
	}

	iocqueue_node *lpnode = malloc(sizeof(iocqueue_node));
	assert(lpnode);
	lpnode->_data = data;
	lpnode->_next = NULL;
#ifdef OC_MULTITHREADING
	for (;;)
	{
		if (CAS(&lq->_wt, 0, ithread_self()))
		{
			iocqueue_node *p = lq->_tail;
			iocqueue_node *oldp = p;
			do {

				while (p->_next != NULL)
					p = p->_next;
			} while (!CAS(&p->_next, NULL, lpnode));


			AtomicAdd(&lq->_num, 1);
			CAS(&lq->_tail, oldp, lpnode);
			AtomicRelease(&lq->_wt);
			break;
		}
		else
		{
			// this is a good place to yield the thread in case there are more
			// software threads than hardware processors and you have more
			// than 1 producer thread
			// have a look at sched_yield (POSIX.1b)
			sched_yield();
		}
	}

#else
	lq->_tail->_next = lpnode;
	lq->_tail = lpnode;
	++lq->_num;
#endif

	return TRUE;
}

//---------------------------------------------------------------
//function: 
//          iocqueue_count ��ȡ���нڵ㣬��ǰ����
//Access:
//           public  
//Parameter:
//          [in] iocqueue * lq - 
//Returns:
//          uint32_i - ����
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
uint32_i iocqueue_count(iocqueue* lq)
{
	return lq->_num;
}


//---------------------------------------------------------------
//function: 
//          iocqueue_release 
//Access:
//           public  
//Parameter:
//          [in] iocqueue * lq - 
//Returns:
//          void - 
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
void iocqueue_release(iocqueue* lq)
{
#ifdef OC_MULTITHREADING
	CAS(&lq->_status, OC_QU_NOMAL, OC_QU_RELEASE);
#endif
	void* lfree = 0;
	while ((lfree = iocqueue_pop(lq)) != NULL)
	{
		if (lq->_freecall == NULL) continue;
		(*lq->_freecall)(lfree);
	}

	//�ͷ���Դ
	free(lq->_head);
	lq->_head = NULL;
	lq->_tail = NULL;
	free(lq);
}
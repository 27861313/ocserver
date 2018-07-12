#include "ihashmap.h"
#include "iatom.h"
#include "iconvert.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#ifndef IAVL_TREE_C
#include "iavltree.inc"
#endif
//---------------------------------------------------------------
//FUNCTION:
//          IOCHASHMAP_UINT
//ACCESS:
//           PUBLIC
//PARAMETER:
//          [IN] VOID * KEY -
//RETURNS:
//          UINT32_I -
//REMARKS:
//          ...
//AUTHOR:    ADMINISTRATOR[MIRLIANG]
//---------------------------------------------------------------
/*
UINT32_I IOCHASHMAP_UINT(VOID *KEY) 
{
	UINT32_I K = V_TOUINT32(KEY);
	RETURN K;
}
*/
//---------------------------------------------------------------
//FUNCTION:
//          IOCHASHMAP_INT
//ACCESS:
//           PUBLIC
//PARAMETER:
//          [IN] VOID * KEY -
//RETURNS:
//          UINT32_I -
//REMARKS:
//          ...
//AUTHOR:    ADMINISTRATOR[MIRLIANG]
//---------------------------------------------------------------
/*
UINT32_I IOCHASHMAP_INT(VOID *KEY) 
{
	UINT32_I K = V_TOINT32(KEY);
	RETURN K;
}
*/

//---------------------------------------------------------------
//FUNCTION:
//          IOCHASHMAP_STR
//ACCESS:
//           PUBLIC
//PARAMETER:
//          [IN] VOID * KEY -
//RETURNS:
//          UINT32_I -
//REMARKS:
//          ...
//AUTHOR:    ADMINISTRATOR[MIRLIANG]
//---------------------------------------------------------------
/*
UINT32_I IOCHASHMAP_STR(VOID *KEY) {

	UINT32_I HASHCODE = 0;
	CHAR *STR = (CHAR*)KEY;
	WHILE (*STR) {
		HASHCODE = (*STR++) + (HASHCODE << 6) + (HASHCODE << 16) - HASHCODE;
	}
	HASHCODE &= 0X7FFFFFFF;
	RETURN HASHCODE;
}
*/

//---------------------------------------------------------------
//function:
//          iochashmap_create
//Access:
//           public
//Parameter:
//          [in] uint32_i hashsize -
//          [in] uint32_i -
//          [in] * ihashcode -
//          [in] void * key -
//          [in] int32_i -
//          [in] * equals -
//          [in] void * key1 -
//          [in] void * key2 -
//Returns:
//          iochashmap* -
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------

iochashmap *iochashmap_create(uint32_i hashsize, uint32_i (*ihashcode)(void *key), int32_i (*equals)(void *key1, void *key2), void (*addref)(void *val), void (*delref)(void *val))
{
	if (hashsize <= 0 || equals == NULL) // 模长不能为0, equals比较函数不能为空
	{
		return NULL;
	}

	iochashmap *lmap = calloc(1, sizeof(iochashmap));
	if (lmap == NULL)
		return NULL;

	lmap->ihashcode = ihashcode;
	lmap->equals = equals;
	//lmap->getkey = getkey;
	lmap->addref = addref;
	lmap->delref = delref;
	lmap->_tbsize = hashsize;
	lmap->_size = 0;
	lmap->_tbarray = calloc(lmap->_tbsize, sizeof(iochashmap_entry));

	for (int i = 0; i < lmap->_tbsize; ++i) // 槽位初始化
	{
		(lmap->_tbarray + i)->_mark = 0;
		(lmap->_tbarray + i)->_tree = NULL;
	}

	if (lmap->_tbarray == NULL)
	{
		free(lmap);
		return NULL;
	}

	return lmap;
}

//---------------------------------------------------------------
//function:
//          iochashmap_put
//Access:
//           public
//Parameter:
//          [in] iochashmap * lmap -
//          [in] void * key -
//          [in] void * val -
//Returns:
//          Boolean -
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
Boolean iochashmap_put(iochashmap *lmap, void *key, void *val)
{
	if (lmap == NULL || key == NULL || val == NULL)
	{
		return FALSE;
	}

	uint32_i index = (*(lmap->ihashcode))(key) % (lmap->_tbsize);
	iochashmap_entry *ltb = lmap->_tbarray + index;
	if (ltb == NULL)
	{
		return FALSE;
	}

	while (!CAS(&(ltb->_mark), 0, 0x10000000)) // 加写锁
	{
	}

	Boolean issucc = FALSE;
	ltb->_tree = iocinsert_tree(key, val, ltb->_tree, lmap->equals, &issucc);

	if (issucc)
	{
			AtomicAddFetch(&(lmap->_size), 1); // 数量加1
	}

	if (lmap->addref != NULL)
	{
		lmap->addref(val); // 添加引用计数
	}

	ltb->_mark = 0; // 释放锁

	return TRUE;
}

//---------------------------------------------------------------
//function:
//          iochashmap_getentry
//Access:
//           public
//Parameter:
//          [in] iochashmap * lmap -
//          [in] void * key -
//Returns:
//          iochashmap_entry * -
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
void *iochashmap_get(iochashmap *lmap, void *key)
{
	uint32_i index = (*(lmap->ihashcode))(key) % (lmap->_tbsize);
	iochashmap_entry *ltb = lmap->_tbarray + index;

	uint32_i curr = 0;
	uint16_i try_cnt = 0;
	do // 加读锁
	{
		curr = ltb->_mark;
		while ((curr & 0x10000000) != 0)
		{
			++try_cnt;
			if (try_cnt > 10)
			{
				try_cnt = 0;
				sched_yield(); // 尝试次数超过10次 让出cpu时间片
			}
			curr = ltb->_mark;
		}
	} while (!CAS(&(ltb->_mark), curr, curr + 1));

	void *val = iocfind_val(key, ltb->_tree, lmap->equals);
	if (lmap->addref != NULL && val != NULL) // 添加引用计数
	{
		lmap->addref(val);
	}

	try_cnt = 0;
	do // 释放读锁
	{
		if (++try_cnt > 10)
		{
			try_cnt = 0;
			sched_yield(); // 让出cpu时间片
		}
		curr = ltb->_mark;
	} while (!CAS(&(ltb->_mark), curr, curr - 1));

	
	return val;
}

//---------------------------------------------------------------
//function:
//          iochashmap_removeentry
//Access:
//           public
//Parameter:
//          [in] iochashmap * lmap -
//          [in] iochashmap_entry * lentry -
//Returns:
//          iochashmap_entry * -
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
Boolean iochashmap_del(iochashmap *lmap, void *key)
{
	if (lmap == NULL || key == NULL)
	{
		return FALSE;
	}

	uint32_i index = (*(lmap->ihashcode))(key) % lmap->_tbsize;
	iochashmap_entry *ltb = lmap->_tbarray + index;

	Boolean issucc = FALSE;
	void* val = iocfind_val(key, ltb->_tree, lmap->equals);
	if (val != NULL)
	{
		while (!CAS(&(ltb->_mark), 0, 0x10000000)) // 加写锁
		{
		}

		ltb->_tree = iocdel_node(key, ltb->_tree, lmap->equals, &issucc);

		if (issucc)
		{
				AtomicSubFetch(&(lmap->_size), 1); // 数量减1
		}

		if (lmap->delref != NULL)
		{
				lmap->delref(val); // 引用计数减1
		}

		ltb->_mark = 0; // 释放写锁
	}
	return issucc;
}

//---------------------------------------------------------------
//function:
//          iochashmap_release
//Access:
//           public
//Parameter:
//          [in] iochashmap * lmap -
//Returns:
//          void -
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
void iochashmap_release(iochashmap *lmap)
{
	if (lmap == NULL)
	{
		return;
	}

	for (size_t i = 0; i < lmap->_tbsize; i++)
	{
		iocdelete_tree(&((lmap->_tbarray + i)->_tree), lmap->delref);
	}

	free(lmap->_tbarray);
	free(lmap);
}

void iochashmap_getkey(iochashmap *lmap, void *rev, void (*getkey)(void *key, void *rev))
{
	for (int i = 0; i < lmap->_tbsize; ++i)
	{
		iocque_tree((lmap->_tbarray + i)->_tree, rev, getkey);
	}
}

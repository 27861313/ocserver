#ifndef IAVL_TREE_C
#define IAVL_TREE_C

#include "iinc.h"
#include "iconvert.h"
#include <stdlib.h>
#include <stdio.h>

//void (*getkey)(void* key);
typedef struct AVL_TREE avl_tree;
struct AVL_TREE
{
	void *_key;
	void *_val;
	uint32_i _height;
	avl_tree *_pleft;
	avl_tree *_pright;
};

uint32_i iocmaxnode(uint32_i a, uint32_i b) 
{ 
	return (a > b ? a : b); 
}

uint32_i iocheight(avl_tree* tree)
{
	if (tree == NULL)
	{
		return 0;
	}
	return tree->_height;
}

avl_tree* iocfindmax(avl_tree* tree)
{
	if (tree != NULL)
	{
		while (tree->_pright != NULL)
		{
			tree = tree->_pright;
		}
	}
	return tree;
}

avl_tree* iocfindmin(avl_tree* tree)
{
	if (tree != NULL)
	{
		while (tree->_pleft != NULL)
		{
			tree = tree->_pleft;
		}
	}
	return tree;
}

avl_tree* iocsinglerotate_wl(avl_tree *tree)
{
	avl_tree* p;
	p = tree->_pleft;
	if (p == NULL)
	{
		return tree;
	}

	tree->_pleft = p->_pright;
	p->_pright = tree;

	tree->_height = iocmaxnode(iocheight(tree->_pleft), iocheight(tree->_pright)) + 1;
	p->_height = iocmaxnode(iocheight(p->_pleft), tree->_height) + 1;

	return p;
}

avl_tree* iocsinglerotate_wr(avl_tree* tree)
{
	avl_tree* p;

	p = tree->_pright;
	if (p == NULL)
	{
		return tree;
	}

	tree->_pright = p->_pleft;
	p->_pleft = tree;

	tree->_height = iocmaxnode(iocheight(tree->_pleft), iocheight(tree->_pright)) + 1;
	p->_height = iocmaxnode(iocheight(p->_pright), tree->_height) + 1;

	return p;
}

avl_tree* iocdoublerotate_wl(avl_tree *tree)
{
	tree->_pleft = iocsinglerotate_wr(tree->_pleft);

	return iocsinglerotate_wl(tree);
}

avl_tree* iocdoublerotate_wr(avl_tree *tree)
{
	tree->_pright = iocsinglerotate_wl(tree->_pright);

	return iocsinglerotate_wr(tree);
}

void iocque_tree(avl_tree* tree, void* rev, void (*getkey)(void* key, void* rev))
{
	if (tree != NULL)
	{
		iocque_tree(tree->_pleft, rev, getkey);
		//printf("%d   ", v_touint32(tree->_val));
		if (getkey != NULL)
		{
			getkey(tree->_key, rev);
		}
		iocque_tree(tree->_pright, rev, getkey);
	}
}

//---------------------------------------------------------------
//function: 
//          iochashmap_equals_str 
//Access:
//           public  
//Parameter:
//          [in] void * key1 - 
//          [in] void * key2 - 
//Returns:
//          int32_i - 
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
int32_i iochashmap_equals_str(void *key1, void *key2) 
{
	//return strcmp(*((uint32_i*)key1), *((uint32_i*)key2));
	if(v_touint32(key1) > v_touint32(key2))
	{
		return 1;
	}
	else if (v_touint32(key1) < v_touint32(key2))
	{
		return -1;
	}
	return 0;
}

void* iocfind_val(void* key, avl_tree* tree, int32_i(*equals)(void* key1, void* key2))
{
	if (tree == NULL || key == NULL)
	{
		return NULL;
	}
	else if ((*equals)(key, tree->_key) == 0)
	{
		return tree->_val;
	}
	else if ((*equals)(key, tree->_key) < 0)
	{
		return iocfind_val(key, tree->_pleft, equals);
	}
	else
	{
		return iocfind_val(key, tree->_pright, equals);
	}
}


avl_tree* iocinsert_tree(void* key, void* val, avl_tree* tree, int32_i(*equals)(void* key1, void* key2), Boolean* issucc)
{
	if (key == NULL || val == NULL) 
	{
		return tree;
	}

	if (iocfind_val(key, tree, equals) != NULL) // 重复的值 不能插入
	{
		return tree;
	}
	
	if (tree == NULL)
	{
		tree = (avl_tree *)malloc(sizeof(avl_tree));
		tree->_key = key;
		tree->_val = val;
		tree->_height = 0;
		tree->_pleft = NULL;
		tree->_pright = NULL;
		*issucc = TRUE;
	}
	else if ((*equals)(key, tree->_key) < 0)
	{
		tree->_pleft = iocinsert_tree(key, val, tree->_pleft, equals, issucc);
		if (iocheight(tree->_pleft) - iocheight(tree->_pright) == 2) // 不平衡
		{
			if ((*equals)(key, tree->_pleft->_key) < 0)
			{
				tree = iocsinglerotate_wl(tree);
			}
			else
			{
				tree = iocdoublerotate_wl(tree);
			}
		}
	}
	else if ((*equals)(key, tree->_key) > 0)
	{
		tree->_pright = iocinsert_tree(key, val, tree->_pright, equals, issucc);
		if (iocheight(tree->_pright) - iocheight(tree->_pleft) == 2) // 不平衡
		{
			if ((*equals)(key, tree->_pright->_key) > 0)
			{
				tree = iocsinglerotate_wr(tree);
			}
			else
			{
				tree = iocdoublerotate_wr(tree);
			}
		}
	}

	tree->_height = iocmaxnode(iocheight(tree->_pleft), iocheight(tree->_pright)) + 1;
	return tree;
}

avl_tree* iocdel_node(void* key, avl_tree* tree, int32_i(*equals)(void* key1, void* key2), Boolean* issucc)
{
	if (key == NULL || tree == NULL || equals == NULL)
	{
		return tree;
	}

	void* node = iocfind_val(key, tree, equals);
	if (node == NULL)
	{
		return tree;
	}

	if ((*equals)(key, tree->_key) < 0)
	{
		tree->_pleft = iocdel_node(key, tree->_pleft, equals, issucc);

		if (iocheight(tree->_pright) - iocheight(tree->_pleft) == 2) // 不平衡 
		{
			if ((*equals)(key, tree->_pright->_key) < 0)
			{
				tree = iocdoublerotate_wr(tree);
			}
			else
			{
				tree = iocsinglerotate_wr(tree);
			}
		}
	}
	else if ((*equals)(key, tree->_key) > 0)
	{
		tree->_pright = iocdel_node(key, tree->_pright, equals, issucc);
		if (iocheight(tree->_pleft) - iocheight(tree->_pright) == 2)
		{
			if ((*equals)(key, tree->_pleft->_key) < 0)
			{
				tree = iocdoublerotate_wl(tree);
			}
			else
			{
				tree = iocsinglerotate_wl(tree);
			}
		}
	}
	else
	{
		if (tree->_pleft && tree->_pright)
		{
			if (iocheight(tree->_pright) > iocheight(tree->_pleft))
			{
				avl_tree* nodemin = iocfindmin(tree->_pright);
				tree->_key = nodemin->_key;
				tree->_val = nodemin->_val;
				tree->_pright = iocdel_node(tree->_key, tree->_pright, equals, issucc);
			}
			else
			{
				avl_tree* nodemax = iocfindmax(tree->_pleft);
				tree->_key = nodemax->_key;
				tree->_val = nodemax->_val;
				tree->_pleft = iocdel_node(tree->_key, tree->_pleft, equals, issucc);
			}
		}
		else
		{
			avl_tree* node = tree;
			tree = (tree->_pleft == NULL ? tree->_pright : tree->_pleft);
			free(node);
		}
		*issucc = TRUE;
	}
	return tree;
}

void iocdelete_tree(avl_tree** ptree, void (*delref)(void* val))
{
	if (ptree == NULL || *ptree == NULL)
	{
		return;
	}

	iocdelete_tree(&((*ptree)->_pleft), delref);
	iocdelete_tree(&((*ptree)->_pright), delref);
	delref((*ptree)->_val);
	free(*ptree);
	*ptree = NULL;
}

/*
uint32_i tree_num(avl_tree* tree)
{
	if (tree == NULL)
	{
		return 0;
	}
	else
	{
		return 1 + tree_num(tree->_pleft) + tree_num(tree->_pright);
	}
}
*/

//---------------------------------------------------------------
//function: 
//          iochashmap_uint 
//Access:
//           public  
//Parameter:
//          [in] void * key - 
//Returns:
//          uint32_i - 
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
uint32_i iochashmap_uint(void *key) 
{
	return  v_touint32(key);
}

//---------------------------------------------------------------
//function: 
//          iochashmap_int 
//Access:
//           public  
//Parameter:
//          [in] void * key - 
//Returns:
//          uint32_i - 
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
uint32_i iochashmap_int(void *key) 
{
	return  v_toint32(key);
}

//---------------------------------------------------------------
//function: 
//          iochashmap_str 
//Access:
//           public  
//Parameter:
//          [in] void * key - 
//Returns:
//          uint32_i - 
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
uint32_i iochashmap_str(void *key) 
{

	uint32_i hashcode = 0;
	char *str = (char*)key;
	while (*str) 
	{
		hashcode = (*str++) + (hashcode << 6) + (hashcode << 16) - hashcode;
	}
	hashcode &= 0x7FFFFFFF;
	return hashcode;
}

//---------------------------------------------------------------
//function: 
//          iochashmap_equals_int 
//Access:
//           public  
//Parameter:
//          [in] void * key1 - 
//          [in] void * key2 - 
//Returns:
//          int32_i - 
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
int32_i iochashmap_equals_int(void *key1, void *key2) 
{
	int32_i k1 = v_toint32(key1);
	int32_i k2 = v_toint32(key2);
	return k1 == k2 ? 1 : 0;
}

//---------------------------------------------------------------
//function: 
//          iochashmap_equals_uint 
//Access:
//           public  
//Parameter:
//          [in] void * key1 - 
//          [in] void * key2 - 
//Returns:
//          int32_i - 
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
int32_i iochashmap_equals_uint(void *key1, void *key2) 
{
	//uint32_i k1 = *((uint32_i*)v_touint32(key1));
	//uint32_i k2 = *((uint32_i*)v_touint32(key2));
	uint32_i k1 = v_touint32(key1);
	uint32_i k2 = v_touint32(key2);
	return k1 == k2 ? 1 : 0;
}


//---------------------------------------------------------------
//function: 
//          iochashmap_less_uint 
//Access:
//           public  
//Parameter:
//          [in] void * key1 - 
//          [in] void * key2 - 
//Returns:
//          int32_i - 
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
Boolean iochashmap_less_uint(void *key1, void *key2)
{
	uint32_i k1 = v_touint32(key1);
	uint32_i k2 = v_touint32(key2);
	return k1 < k2 ? TRUE : FALSE;
}


//---------------------------------------------------------------
//function: 
//          iochashmap_less_int 
//Access:
//           public  
//Parameter:
//          [in] void * key1 - 
//          [in] void * key2 - 
//Returns:
//          int32_i - 
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
Boolean iochashmap_less_int(void *key1, void *key2)
{
	int32_i k1 = v_toint32(key1);
	int32_i k2 = v_toint32(key2);
	return k1 < k2 ? TRUE : FALSE;
}

#endif

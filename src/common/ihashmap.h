#ifndef OC_IHASHMAP_H
#define OC_IHASHMAP_H

#include "iinc.h"
//#include "iavltree.h"

#ifdef __cplusplus 
extern "C" {
#endif

/*
///////////////////////////////////
开发机测试：
插入2线程1000000数据平均用时：2.2s
删除2线程1000000数据平均用时：1.9s
查找2线程1000000数据平均用时：1.4s
//////////////////////////////////
*/


typedef struct IOCHASHMAP_ENTRY  iochashmap_entry;
typedef struct IOCHASHMAP        iochashmap;
typedef struct AVL_TREE avl_tree;

struct IOCHASHMAP_ENTRY
{
	uint32_i _mark;   // 第一位标记写入,其余位标记读的次数
	avl_tree* _tree;  // 平衡二叉树,存放冲突数据
};

struct IOCHASHMAP
{
	int32_i   _size;
	int32_i   _tbsize;
	iochashmap_entry *_tbarray;
	//void (*getkey)(void* key, void* rev); // 返回用户层key
	void (*addref)(void* val); // 添加引用计数 (注意用原子操作)
	void (*delref)(void* val); // 删除引用计数 (注意用原子操作)
	uint32_i(*ihashcode)(void* key); // 计算hash模值
	int32_i(*equals)(void* key1, void* key2); // 比较函数
};


// @function 创建HashMap
// @param    uint32_i       -  hash table size
// @param    ihashcode      -  key作为索引比较
// @param    equals         -  key比较函数
// @return   iochashmap*    -  hashmap
iochashmap* iochashmap_create(uint32_i hashsize, uint32_i(*ihashcode)(void* key), int32_i(*equals)(void* key1, void* key2), void (*addref)(void* val), void (*delref)(void* val));

// @function 插入 
// @param    iochashmap*    -  hashmap
// @param    void*          -  key
// @param    void*          -  data
// @return   Boolean        -  TRUE成功FALSE失败
Boolean iochashmap_put(iochashmap *lmap, void* key, void* val);

// @function 查找
// @param    iochashmap*    -  hashmap
// @param    void*          -  key
// @param    void*          -  data
// @return   Boolean        -  TRUE找到FALSE没找到
void* iochashmap_get(iochashmap *lmap, void* key);

// @function 删除节点 
// @param    iochashmap*    -  hashmap
// @param    void*          -  key 
// @return   avl_tree*      -  最后的根节点
Boolean iochashmap_del(iochashmap *lmap, void* key);

// @function iochashmap  销毁
// @param    iochashmap*    -  hashmap
// @return   void           -
void iochashmap_release(iochashmap *lmap);

// @function 获取key值
// @param    iochashmap*    -  hashmap
// @param    *getkey        -  回调函数
// @return   void           -
void iochashmap_getkey(iochashmap *lmap, void* rev, void (*getkey)(void* key, void* rev));
//void printf_tree(iochashmap *lmap);

#ifdef __cplusplus 
}
#endif

#endif

/*
 * Created on Thu Jun 27 2018
 *
 * The MIT License (MIT)
 * Copyright (c) 2018 korialuo
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef OC_ILIST_H
#define OC_ILIST_H

#include "iinc.h"

#ifdef __cplusplus
extern "C" {
#endif

    // 节点
    typedef struct IOC_LISTNODE {
        void*                    _data;
        struct IOC_LISTNODE*     _prev;
        struct IOC_LISTNODE*     _next;
    } ioclistnode;

    // 链表
    typedef struct IOC_LIST {
        int32_i          _len;
        ioclistnode*     _head;
        ioclistnode*     _tail;
    } ioclist;

    // @function 创建节点
    // @return   新节点
    ioclistnode* ioclistnode_create(void *data);

    // @function 释放节点
    // @param    lnode 被释放的节点
    void ioclistnode_release(ioclistnode* lnode);

    // @function 创建链表
    // @return   ioclist*  随机数
    ioclist* ioclist_create();

    // @function 插入节点到链表
    // @param    llst      链表
    // @param    index     插入位置
    // @param    lnode     插入的节点
    // @return   ioclistnode* 返回插入的节点
    ioclistnode* ioclist_insert(ioclist* llst, int32_i index, ioclistnode* lnode);

    // @function 插入节点到链表尾
    // @param    llst      链表
    // @param    lnode     插入的节点
    // @return   ioclistnode* 返回插入的节点
    ioclistnode* ioclist_pushback(ioclist* llst, ioclistnode* lnode);

    // @function 移除链表某个位置的节点
    // @param    llst      链表
    // @param    index     位置
    // @return   ioclistnode* 返回被移除的节点
    ioclistnode* ioclist_remove(ioclist* llst, int32_i index);

    // @function 获取链表某个位置的节点
    // @param    llst      链表
    // @param    index     位置
    // @return   ioclistnode* 返回指定位置上的节点
    ioclistnode* ioclist_at(ioclist* llst, int32_i index);

    // @function 获取链表头节点
    // @param    llst      链表
    // @return   ioclistnode* 返回头节点
    ioclistnode* ioclist_head(ioclist* llst);

    // @function 获取链表尾节点
    // @param    llst      链表
    // @return   ioclistnode* 返回尾节点
    ioclistnode* ioclist_tail(ioclist* llst);

    // @function 清空链表
    // @param    llst      链表
    // @return   void
    void ioclist_clear(ioclist* llist);

    // @function 遍历链表
    // @param    llst        链表
    // @param    arg         传给回调方法的自定义参数
    // @param    foreach_cb    回调方法
    //           @param lnode  当前节点
    //           @param arg    自定义参数
    //           @return bool  返回true停止遍历, 返回false继续遍历
    // @return   void
    void ioclist_foreach(ioclist* llst, void* arg, Boolean (*foreach_cb)(ioclistnode* lnode, void *arg));

    // @function 释放链表
    // @param    llst       链表
    // @return   void
    void ioclist_release(ioclist* llst);

#ifdef __cplusplus
}
#endif

#endif // OC_ILIST_H





    
#include "ilist.h"
#include <stdlib.h>
#include <memory.h>

ioclistnode *ioclistnode_create(void *data)
{
    ioclistnode *node = NULL;
    if ((node = (ioclistnode *)malloc(sizeof(ioclistnode))) != NULL)
    {
        node->_data = data;
    }
    return node;
}

void ioclistnode_release(ioclistnode *lnode)
{
    if (lnode != NULL)
    {
        free(lnode);
    }
}

ioclist *ioclist_create()
{
    ioclist *llst = (ioclist *)malloc(sizeof(ioclist));
    if (llst == NULL)
    {
        return NULL;
    }
    memset(llst, 0, sizeof(ioclist));
    return llst;
}

ioclistnode *ioclist_insert(ioclist *llst, int32_i index, ioclistnode *lnode)
{
    if (lnode == NULL || index >= llst->_len)
    {
        return NULL;
    }
    ioclistnode *insertnode = llst->_head;
    int32_i i = 0;
    while (1)
    {
        if (!insertnode || i == index)
            break;
        insertnode = insertnode->_next;
        ++i;
    }
    if (!insertnode)
    {
        return NULL;
    }
    lnode->_prev = insertnode->_prev;
    lnode->_next = insertnode;
    insertnode->_prev = lnode;
    if (llst->_head == insertnode)
    {
        llst->_head = lnode;
    }
    llst->_len++;
    return lnode;
}

ioclistnode *ioclist_pushback(ioclist *llst, ioclistnode *lnode)
{
    if (lnode == NULL)
    {
        return NULL;
    }
    if (llst->_len == 0)
    {
        llst->_head = lnode;
        llst->_tail = lnode;
        lnode->_prev = NULL;
        lnode->_next = NULL;
    }
    else
    {
        lnode->_prev = llst->_tail;
        lnode->_next = NULL;
        llst->_tail->_next = lnode;
        llst->_tail = lnode;
    }
    llst->_len++;
    return lnode;
}

ioclistnode *ioclist_remove(ioclist *llst, int32_i index)
{
    if (index >= llst->_len)
    {
        return NULL;
    }
    ioclistnode *removenode = llst->_head;
    int32_i i = 0;
    while (1)
    {
        if (!removenode || i == index)
            break;
        removenode = removenode->_next;
        ++i;
    }
    if (!removenode)
    {
        return NULL;
    }
    if (removenode->_prev)
    {
        // 有前节点, 将前节点的后指针设为当前节点后一节点
        removenode->_prev->_next = removenode->_next;
    }
    else
    {
        // 没有前节点, 将链表头指针设为当前节点后一节点
        llst->_head = removenode->_next;
    }

    if (removenode->_next)
    {
        // 有后节点, 将后节点的前指针设为当前节点前一节点
        removenode->_next->_prev = removenode->_prev;
    }
    else
    {
        // 没有后节点, 将链表尾指针设为当前节点前一节点
        llst->_tail = removenode->_prev;
    }
    removenode->_prev = NULL;
    removenode->_next = NULL;
    llst->_len--;
    return removenode;
}

ioclistnode *ioclist_at(ioclist *llst, int32_i index)
{
    if (index >= llst->_len)
    {
        return NULL;
    }

    int32_i i = 0;
    ioclistnode *node = llst->_head;
    while (1)
    {
        if (!node || i == index)
            break;
        node = node->_next;
        ++i;
    }
    return node;
}

ioclistnode *ioclist_head(ioclist *llst)
{
    return llst->_head;
}

ioclistnode *ioclist_tail(ioclist *llst)
{
    return llst->_tail;
}

void ioclist_clear(ioclist *llist)
{
    ioclistnode *node = ioclist_head(llist);
    while (node != NULL)
    {
        ioclistnode *next = node->_next;
        ioclistnode_release(node);
        node = next;
    }
    llist->_head = NULL;
    llist->_tail = NULL;
    llist->_len = 0;
}

void ioclist_foreach(ioclist *llst, void *arg, Boolean (*foreach_cb)(ioclistnode *lnode, void *arg))
{
    /*if (foreach_cb == NULL)
    {
        return;
    }*/
    ioclistnode *node = llst->_head;
    while (1)
    {
        if (!node)
            break;
        if (foreach_cb(node, arg))
            break;
        node = node->_next;
    }
}

void ioclist_release(ioclist *llst)
{
    if (llst == NULL)
    {
        return;
    }

    ioclistnode *node = llst->_head;
    while (1)
    {
        if (!node)
            break;
        ioclistnode *releasenode = node;
        node = node->_next;
        ioclistnode_release(releasenode);
    }
    free(llst);
}
/*
 * Created on Thu Jun 14 2018
 *
 * The MIT License (MIT)
 * Copyright (c) 2018 mr liang
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

#ifndef MAP_TREE_H
#define MAP_TREE_H

#include "iinc.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef OC_MULTITHREADING
/*
////////////////////////////////////////////////////
one treenode is 46 char
when depth is 4, all tree is 85*46 char
///////////////////////////////////////////////////
*/


	typedef struct IOCREGION_DATA iocregion_data;
	typedef struct IOCMAP_TREE_NODE iocmap_tree_node;

	struct IOCREGION_DATA
	{
		uint32_i _x;    // x
		uint32_i _y;    // y
		uint32_i _long; // 长
		uint32_i _width;// 宽
	};

	struct IOCMAP_TREE_NODE
	{
		int32_i _depth;	 // 树的深度 
		Boolean _ishead; // 是否头节点 
		Boolean _isleaf; // 是否叶子节点 
		iocmap_tree_node* _father; // 父亲节点
		iocmap_tree_node* _lu;
		iocmap_tree_node* _lb;
		iocmap_tree_node* _ru;
		iocmap_tree_node* _rb;
		iocregion_data _region;
		void *datap; // 数据信息
	};


	// @function 创建节点
	// @param   x_max     x最大范围
	// @param   y_max     y最大范围
	// @param   depth     最大高度 
	// @return  map_tree_node* 
	iocmap_tree_node* ioccreate_map_tree(uint32_i x_max, uint32_i y_max, uint32_i depth);

	// @function 销毁树
	void iocdestroy_map_tree(iocmap_tree_node *head);

	// @function 分支处理 
	// @param   p_old_node 父亲节点 
	// @param   depth   深度
	void iocsplit_node(iocmap_tree_node *p_old_node, uint32_i depth);

	// @function
	// @param   x     x
	// @param   y     y
	// @param   longth  长
	// @param   width   宽
	// @param   father   父亲节点 
	// @param   dep      深度
	// @param   isleaf   是否叶子节点       
	// @return  map_tree_node*
	iocmap_tree_node* ioccreate_node(uint32_i x, uint32_i y, uint32_i longth,
							   uint32_i width, iocmap_tree_node *father, uint32_i dep, Boolean isleaf);

	// @function        区域检测ཻ
	// @param   one     
	// @param   two     
	// @return  Boolean 
	//Boolean is_rect_coin(region_data one, region_data two);

#ifdef __cplusplus
}
#endif
#endif
#endif
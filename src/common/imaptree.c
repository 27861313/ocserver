#include "imaptree.h"
#include "malloc.h"


iocmap_tree_node* ioccreate_map_tree(uint32_i x_max, uint32_i y_max, uint32_i depth)
{
	iocmap_tree_node *head = ioccreate_node(0, x_max, 0, y_max, NULL, 0, FALSE); // 创建节点 
	if (head == NULL)
	{
		return NULL;
	}

	iocsplit_node(head, depth);
	return NULL;
}

void iocdestroy_map_tree(iocmap_tree_node *head)
{
	if (head == NULL)
	{
		return;
	}

	iocdestroy_map_tree(head->_lb);
	iocdestroy_map_tree(head->_lu);
	iocdestroy_map_tree(head->_rb);
	iocdestroy_map_tree(head->_rb);
	free(head);
}

void iocsplit_node(iocmap_tree_node *p_old_node, uint32_i depth)
{
	if (p_old_node == NULL || p_old_node->_depth >= depth)
	{
		return;
	}

	Boolean isleaf = FALSE;
	uint32_i dep = p_old_node->_depth + 1;
	if (dep >= depth)
	{
		isleaf = TRUE;
	}

	uint32_i x = p_old_node->_region._x;
	uint32_i y = p_old_node->_region._y;
	uint32_i longth = p_old_node->_region._long / 2;
	uint32_i width = p_old_node->_region._width / 2;
	p_old_node->_lu = ioccreate_node(x, y, longth, width, p_old_node, dep, isleaf);
	p_old_node->_lb = ioccreate_node(x, y + width, longth, width, p_old_node, dep, isleaf);
	p_old_node->_ru = ioccreate_node(x + longth, y, longth, width, p_old_node, dep, isleaf);
	p_old_node->_rb = ioccreate_node(x + longth, y + width, longth, width, p_old_node, dep, isleaf);

	iocsplit_node(p_old_node->_lu, depth);
	iocsplit_node(p_old_node->_lb, depth);
	iocsplit_node(p_old_node->_ru, depth);
	iocsplit_node(p_old_node->_ru, depth);
}

iocmap_tree_node* ioccreate_node(uint32_i x, uint32_i y, uint32_i longth,
							   uint32_i width, iocmap_tree_node *father, uint32_i dep, Boolean isleaf)
{
	iocmap_tree_node* new_node = (iocmap_tree_node *)malloc(sizeof(iocmap_tree_node));

	if (dep == 0) // 头结点
	{
		new_node->_ishead = TRUE;
	}

	new_node->_isleaf = isleaf;
	new_node->_father = father;
	new_node->_region._x = x;
	new_node->_region._y = y;
	new_node->_region._long = longth;
	new_node->_region._width = width;
	new_node->_depth = dep;

	new_node->_lu = NULL;
	new_node->_lb = NULL;
	new_node->_rb = NULL;
	new_node->_ru = NULL;

	return new_node;
}

/*
Boolean is_rect_coin(region_data one, region_data two)
{
	if (((one.x_min > two.x_min && one.x_min < two.x_max) || (one.x_max > two.x_min && one.x_max < two.x_max)) && ((one.y_min > two.y_min && one.y_min < two.y_max) || (one.y_max > two.y_min && one.y_max < two.y_max)))
	{
		return TRUE;
	}
	return FALSE;
}
*/
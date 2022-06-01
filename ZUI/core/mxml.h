/**
* @file		mxml.h
* @brief	xml解析器.
* @author	[Zero](22249030)
* @version	1.0
* @date		$date
* @par History:
*
* [2016-10-25] <Zero> v1.0
*
* + v1.0版发布.
*
*/
#ifndef _mxml_h_
#define _mxml_h_

#include <ZUI.h>

#  define MXML_DESCEND		1	/* Descend when finding/walking */
#  define MXML_NO_DESCEND	0	/* Don't descend when finding/walking */
#  define MXML_DESCEND_FIRST	-1	/* Descend for first find */

#  define MXML_ADD_BEFORE	0	/* Add node before specified node */
#  define MXML_ADD_AFTER	1	/* Add node after specified node */
#  define MXML_ADD_TO_PARENT	NULL	/* Add node relative to parent */

typedef struct mxml_attr_s		/**** XML元素节点的属性值 ****/
{
	ZuiText			name;		/* 属性名 */
	ZuiText			value;		/* 属性值 */
} mxml_attr_t;

typedef struct mxml_value_u		/**** XML元素值 ****/
{
	ZuiText			name;		/* Name of element */
	int			num_attrs;	/* Number of attributes */
	mxml_attr_t		*attrs;		/* Attributes */
} mxml_value_t;

typedef struct mxml_node_s			/**** XML 节点 ****/
{
	struct mxml_node_s	*next;			/* 同级的下一个节点（在同一个父节点下）*/
	struct mxml_node_s	*prev;			/* 同级的上一个节点（在同一个父节点下 */
	struct mxml_node_s	*parent;		/* 父节点 */
	struct mxml_node_s	*child;			/* 第一个子节点 */
	struct mxml_node_s	*last_child;	/* 最后一个子节点 */
	mxml_value_t		value;			/* Node value */
	void				*user_data;		/* 用户关联数据 */
}mxml_node_t ;

typedef struct mxml_buf			/**** XML 节点 ****/
{
	wchar_t *buf;
	int len;
	int pos;
}mxml_buf_t;

mxml_node_t *mxmlLoadString(mxml_node_t *top, ZuiAny s, int len);
void mxmlAdd(mxml_node_t *parent, int where, mxml_node_t *child, mxml_node_t *node);
mxml_node_t *mxmlFindElement(mxml_node_t *node, mxml_node_t *top, const wchar_t *name, const wchar_t *attr, const wchar_t *value, int descend);
mxml_node_t *mxmlFindPath(mxml_node_t *top, const wchar_t *path);
mxml_node_t *mxmlWalkNext(mxml_node_t *node, mxml_node_t *top, int descend);
mxml_node_t *mxmlWalkPrev(mxml_node_t *node, mxml_node_t *top, int descend);
mxml_node_t *mxmlClone(mxml_node_t *node, mxml_node_t *parent);
void        mxmlDelete(mxml_node_t *node);

#endif /* !_mxml_h_ */

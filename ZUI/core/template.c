#include <ZUI.h>
#include "control.h"
#include "template.h"

#include <layout/HorizontalLayout.h>
#include <layout/Layout.h>
#include <layout/TabLayout.h>
#include <layout/TileLayout.h>
#include <layout/VerticalLayout.h>
#include <control/Window.h>
#include <core/function.h>

static int ZTemplate_Compare(struct _ZTemplate *e1, struct _ZTemplate *e2)
{
    return (e1->key < e2->key ? -1 : e1->key > e2->key);
}
RB_GENERATE(_ZTemplate_Tree, _ZTemplate, entry, ZTemplate_Compare);


struct _ZTemplate_Tree *Global_TemplateClass = NULL;
ZuiBool ZuiTemplateInit()
{
    Global_TemplateClass = (struct _ZTemplate_Tree *)malloc(sizeof(struct _ZTemplate_Tree));
    memset(Global_TemplateClass, 0, sizeof(struct _ZTemplate_Tree));
    return TRUE;
}
ZuiVoid ZuiTemplateUnInit() {
    struct _ZTemplate * c = NULL;
    struct _ZTemplate * cc = NULL;
    RB_FOREACH_SAFE(c, _ZTemplate_Tree, Global_TemplateClass, cc) {
        mxmlDelete((mxml_node_t*)c->node);
        RB_REMOVE(_ZTemplate_Tree, Global_TemplateClass, c);
        free(c);
    }
    free(Global_TemplateClass);
    Global_TemplateClass = NULL;
}
ZuiVoid ZuiAddTemplate(mxml_node_t *node)
{
    ZuiText classname = NULL;
    /*解析属性*/
    for (int i = 0; i < node->value.num_attrs; i++)
    {
        if (_tcsicmp(node->value.attrs[i].name, L"class") == 0)
        {
            classname = node->value.attrs[i].value;
        }
    }
    if (classname)
    {
		ZTemplate tmp = { 0 };
		tmp.key = Zui_Hash(classname);
		if (RB_FIND(_ZTemplate_Tree, Global_TemplateClass, &tmp))
		{
			return;
		}
        mxml_node_t *new_node = mxmlClone(node, NULL);
        if (new_node)
        {
            _wcslwr(classname);
            struct _ZTemplate *n = malloc(sizeof(struct _ZTemplate));
            memset(n, 0, sizeof(struct _ZTemplate));
            n->key = Zui_Hash(classname);
            n->node = new_node;
            RB_INSERT(_ZTemplate_Tree, Global_TemplateClass, n);
        }
    }
}

ZuiVoid ZuiLoadTemplate(mxml_node_t *n, ZuiControl p, ZuiAny Param1, ZuiAny Param2) {
    ZuiText Layout = _T("Layout");
    ZuiText ClassName = NULL;
    ZuiControl Control;
    mxml_node_t *node = mxmlClone(n, NULL);//先把节点克隆出来
    mxml_node_t *node_root = node;//用来销毁这个xml树
    for (int i = 0; i < node->value.num_attrs; i++)
    {
        if (_tcsicmp(node->value.attrs[i].name, L"layout") == 0) {
            Layout = node->value.attrs[i].value;
            if (_tcsicmp(Layout, L"layout") == 0)
            {
                p->m_sUserData = ZuiLayoutProc(Proc_OnCreate, p, 0, Param1, Param2);
                p->call = (ZCtlProc)&ZuiLayoutProc;
            }
            else if (_tcsicmp(Layout, L"horizontal") == 0)
            {
                p->m_sUserData = ZuiHorizontalLayoutProc(Proc_OnCreate, p, 0, Param1, Param2);
                p->call = (ZCtlProc)&ZuiHorizontalLayoutProc;
            }
            else if (_tcsicmp(Layout, L"vertical") == 0)
            {
                p->m_sUserData = ZuiVerticalLayoutProc(Proc_OnCreate, p, 0, Param1, Param2);
                p->call = (ZCtlProc)&ZuiVerticalLayoutProc;
            }
            else if (_tcsicmp(Layout, L"tile") == 0)
            {
                p->m_sUserData = ZuiTileLayoutProc(Proc_OnCreate, p, 0, Param1, Param2);
                p->call = (ZCtlProc)&ZuiTileLayoutProc;
            }
            else if (_tcsicmp(Layout, L"window") == 0)
            {
                p->m_sUserData = ZuiWindowProc(Proc_OnCreate, p, 0, Param1, Param2);
                p->call = (ZCtlProc)&ZuiWindowProc;
            }

        }
        else
        {
            ZuiControlCall(Proc_SetAttribute, p, node->value.attrs[i].name, node->value.attrs[i].value);
        }
    }

    node->user_data = p;
    for (node = mxmlFindElement(node, node, NULL, NULL, NULL, MXML_DESCEND); node != NULL; node = mxmlWalkNext(node, NULL, MXML_DESCEND))
    {
        ClassName = node->value.name;
        
        Control = NewZuiControl(ClassName, NULL, NULL);
        if (Control) {
            node->user_data = Control;//保存控件到节点
                                      /*添加到容器*/
            ZuiControlCall(Proc_Layout_Add, node->parent->user_data, Control, NULL);
            /*解析属性*/
            for (int i = 0; i < node->value.num_attrs; i++)
            {
                ZuiControlCall(Proc_SetAttribute, Control, node->value.attrs[i].name, node->value.attrs[i].value);
            }
        }
    }
    mxmlDelete(node_root);
}

#include "builder.h"
#include <core/control.h>
#include <core/carray.h>
#include <core/resdb.h>
#include <core/template.h>

static  ZuiControl ZuiLayoutLoadNode(mxml_node_t *tree, ZuiControl win) {
    mxml_node_t *node;
    ZuiText ClassName = NULL;
    ZuiBool Visible = FALSE, Enabled = TRUE;
    ZuiControl Control;
    for (node = mxmlFindElement(tree, tree, NULL, NULL, NULL, MXML_DESCEND); node != NULL; node = mxmlWalkNext(node, NULL, MXML_DESCEND)/*node = mxmlFindElement(node, tree, NULL,NULL,NULL,MXML_DESCEND)*/) {
        {
        LoadNodeBedin:
            ClassName = node->value.name;
            
            if (_tcsicmp(ClassName, L"Template") == 0) {//模版类
                ZuiAddTemplate(node);
                if (node->next) {
					node = node->next;
                    ClassName = node->value.name;
                    goto LoadNodeBedin;
                }
				else if (node->parent) {
					node = node->parent->next;
					if (node) {
						ClassName = node->value.name;
						goto LoadNodeBedin;
					}
					else
						continue;
				}
				else {
					node = NULL;//模板节点既没有兄弟节点也没有父节点，设置为空。
					continue;
				}
            }
            /*
            if (_tcsicmp(ClassName, L"Menu") == 0) {//菜单类
                ZuiAddMenu(node, NULL);
                node = node->next;
                if (node) {
                    ClassName = node->value.name;
                    goto LoadNodeBedin;
                }
                else
                    continue;
            }
            */
			if (_wcsnicmp(ClassName, L"?", 1) == 0) {//跳过<? xxx ?>节点
				node->user_data = node->parent->user_data;
				continue;
			}
            if (_tcsicmp(ClassName, L"Include") == 0) {//包含文件
                ZuiText src = NULL;
                for (int i = 0; i < node->value.num_attrs; i++)
                {
                    if (_tcsicmp(node->value.attrs[i].name, L"src") == 0) {
                        src = node->value.attrs[i].value;
                    }
                }
                ZuiRes res = ZuiResDBGetRes(src, ZREST_STREAM);
                if (res) {
                    mxml_node_t *new_node = mxmlLoadString(NULL,res->p, res->plen);
                    mxmlAdd(node->parent ? node->parent : node, MXML_ADD_AFTER, node, new_node);
                    ZuiResDBDelRes(res);
                }
            }
            else if (!node->user_data) {//当前节点还未创建
                Control = NewZuiControl(ClassName, NULL, NULL);
                if (node->parent->user_data && _tcsicmp(ClassName, L"window") != 0) {
                    //上级控件已存在且当前欲创建的子窗口不为窗口对象
                    if (Control) {
                        node->user_data = Control;//保存控件到节点
                                                  /*添加到容器*/
                        ZuiControlCall(Proc_Layout_Add, node->parent->user_data, Control, NULL);
                    }
                    else {
                        /*当前控件创建失败 子控件肯定创建不了 删除节点*/
                        mxmlDelete(node);
                        /*再次从头处理*/
                        node = tree;
                        continue;//窗口创建失败就没必要继续下去了
                    }
                }
                else if (!node->parent->user_data && _tcsicmp(ClassName, L"window") == 0) {
                    //上级控件已存在且当前欲创建的子窗口为窗口对象
                    if (Control) {
                        node->user_data = Control;//保存控件到节点
                        win = Control;
                    }
                    else
                        break;//窗口创建失败就没必要继续下去了
                }
                /*解析属性*/
                for (int i = 0; i < node->value.num_attrs; i++)
                {
                    ZuiControlCall(Proc_SetAttribute, Control, node->value.attrs[i].name, node->value.attrs[i].value);
                }
            }
        }
    }
    return win;
}
ZEXPORT ZuiControl ZCALL ZuiLayoutLoad(ZuiAny xml, int len) {
    mxml_node_t *tree;
    tree = mxmlLoadString(NULL, xml, len);
    ZuiControl win = ZuiLayoutLoadNode(tree, NULL);
    /*解析完成后释放xml树*/
    mxmlDelete(tree);
    return win;
}
ZuiBool ZuiBuilderInit() {
    return TRUE;
}
ZuiVoid ZuiBuilderUnInit() {

}


#include "Register.h"
#include <core/function.h>
#include <layout/Layout.h>
#include <layout/VerticalLayout.h>
#include <layout/HorizontalLayout.h>
#include <layout/TabLayout.h>
#include <layout/TileLayout.h>

#include <control/Window.h>
#include <control/ScrollBar.h>
#include <control/Label.h>
#include <control/Button.h>
#include <control/Option.h>
#include <control/CheckBox.h>
#include <control/ProgressBar.h>
#include <control/SplitterBar.h>
#include <control/List.h>
#include <control/TreeView.h>

static int ZClass_Compare(struct _ZClass *e1, struct _ZClass *e2)
{
    return (e1->key < e2->key ? -1 : e1->key > e2->key);
}
RB_GENERATE(_ZClass_Tree, _ZClass, entry, ZClass_Compare);

struct _ZClass_Tree *Global_ControlClass = NULL;

static ZuiBool ZuiCoreInit(void *data) {
    return (ZuiBool)((ZCtlProc)data)(Proc_CoreInit, NULL, NULL, NULL, NULL);
}
ZuiBool ZuiClassInit()
{
    Global_ControlClass = (struct _ZClass_Tree *)malloc(sizeof(struct _ZClass_Tree));
    memset(Global_ControlClass, 0, sizeof(struct _ZClass_Tree));
    /*核心组件 不可卸载*/
    ZuiControlRegisterAdd(Type_Layout, (ZCtlProc)&ZuiLayoutProc);
    ZuiControlRegisterAdd(Type_VerticalLayout, (ZCtlProc)&ZuiVerticalLayoutProc);
    ZuiControlRegisterAdd(Type_HorizontalLayout, (ZCtlProc)&ZuiHorizontalLayoutProc);
    ZuiControlRegisterAdd(Type_TabLayout, (ZCtlProc)&ZuiTabLayoutProc);
    ZuiControlRegisterAdd(Type_TileLayout, (ZCtlProc)&ZuiTileLayoutProc);
    ZuiControlRegisterAdd(Type_Window, (ZCtlProc)&ZuiWindowProc);
    ZuiControlRegisterAdd(Type_ScrollBar, (ZCtlProc)&ZuiScrollBarProc);
    ZuiControlRegisterAdd(Type_SplitterBar, (ZCtlProc)&ZuiSplitterBarProc);
    ZuiControlRegisterAdd(Type_Label, (ZCtlProc)&ZuiLabelProc);
    ZuiControlRegisterAdd(Type_ProgressBar, (ZCtlProc)&ZuiProgressBarProc);
    ZuiControlRegisterAdd(Type_Button, (ZCtlProc)&ZuiButtonProc);
    ZuiControlRegisterAdd(Type_Option, (ZCtlProc)&ZuiOptionProc);
    ZuiControlRegisterAdd(Type_CheckBox, (ZCtlProc)&ZuiCheckBoxProc);
    ZuiControlRegisterAdd(Type_List, (ZCtlProc)&ZuiListProc);
    ZuiControlRegisterAdd(Type_TreeView, (ZCtlProc)&ZuiTreeViewProc);
    
    return TRUE;
}
ZuiVoid ZuiClassUnInit() {
    struct _ZClass * c = NULL;
    struct _ZClass * cc = NULL;
    RB_FOREACH_SAFE(c, _ZClass_Tree, Global_ControlClass, cc) {
        c->cb(Proc_CoreUnInit, NULL, NULL, NULL, NULL);
        RB_REMOVE(_ZClass_Tree, Global_ControlClass, c);
        free(c);
    }
    free(Global_ControlClass);
}
ZuiBool ZuiControlRegisterAdd(ZuiText name, ZCtlProc Proc)
{
    if (ZuiCoreInit(Proc))
    {
        struct _ZClass *n = (struct _ZClass *)malloc(sizeof(struct _ZClass));
        memset(n, 0, sizeof(struct _ZClass));
        ZText _name[256];
        memset(_name, 0, sizeof(_name));
        wcsncpy(_name, name, 256);
        _wcslwr(_name);
        n->key = Zui_Hash(_name);
        n->cb = Proc;
        RB_INSERT(_ZClass_Tree, Global_ControlClass, n);
        return TRUE;
    }
    return FALSE;
}
ZuiBool ZuiControlRegisterDel(ZuiText name)
{
    ZClass theNode = { 0 };
    ZClass *c;
    theNode.key = Zui_Hash(name);
    c = RB_FIND(_ZClass_Tree, Global_ControlClass, &theNode);
    if (c) {
        RB_REMOVE(_ZClass_Tree, Global_ControlClass, c);
        free(c);
    }
    return 0;
}

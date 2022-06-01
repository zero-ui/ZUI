#ifndef __Zui_CONTROLS_REGISTER_H__
#define __Zui_CONTROLS_REGISTER_H__
#include <ZUI.h>
#include <core/tree.h>
typedef struct _ZClass
{
    RB_ENTRY(_ZClass) entry;
    ZCtlProc cb;
    unsigned int key;
}*ZuiClass, ZClass;
RB_HEAD(_ZClass_Tree, _ZClass);
RB_PROTOTYPE(_ZClass_Tree, _ZClass,,);
extern struct _ZClass_Tree *Global_ControlClass;

/**
* 此函数用作注册系统默认控件.
* @return 此函数没有返回值.
*/
ZuiBool ZuiClassInit();
ZuiVoid ZuiClassUnInit();
/**
* 此函数用作向系统注册控件.
* @param name 控件类名 请使用全小写注册
* @param Proc 回调地址
* @return 此函数没有返回值.
*/
ZuiBool ZuiControlRegisterAdd(ZuiText name, ZCtlProc Proc);
/**
* 此函数用作删除系统中注册的控件.
* @param name 控件类名 请使用全小写注册
* @return 此函数没有返回值.
*/
ZuiBool ZuiControlRegisterDel(ZuiText name);

#endif	//__Zui_CONTROLS_REGISTER_H__
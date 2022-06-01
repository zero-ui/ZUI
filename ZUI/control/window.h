#ifndef __UIWINDOW_H__
#define __UIWINDOW_H__
#include <ZUI.h>
#include <platform/platform.h>

/**窗口控件结构*/
typedef struct _ZWindow
{    
    ZuiOsWindow m_osWindow;    
    
    ZCtlProc old_call;
    ZuiAny old_udata;
}*ZuiWindow, ZWindow;
ZEXPORT ZuiAny ZCALL ZuiWindowProc(int ProcId, ZuiControl cp, ZuiWindow p, ZuiAny Param1, ZuiAny Param2);
//查找窗口
ZEXPORT ZuiControl ZCALL ZuiWindowFindName(ZuiText Name);

#endif	//__UIWINDOW_H__
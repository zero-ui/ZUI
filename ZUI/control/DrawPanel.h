#ifndef __DrawPanel_H__
#define __DrawPanel_H__
#include <ZUI.h>

/**按钮控件结构*/
typedef struct _ZDrawPanel
{
    ZCtlProc old_call;
}*ZuiDrawPanel, ZDrawPanel;
ZEXPORT ZuiAny ZCALL ZuiDrawPanelProc(int ProcId, ZuiControl cp, ZuiDrawPanel p, ZuiAny Param1, ZuiAny Param2);


#endif	//__LABEL_H__

#ifndef __SplitterBar_H__
#define __SplitterBar_H__
#include <ZUI.h>
/**分割条结构*/
typedef struct _ZSplitterBar
{
    ZuiBool mousedown;
    int type;
    ZCtlProc old_call;
}*ZuiSplitterBar, ZSplitterBar;
ZEXPORT ZuiAny ZCALL ZuiSplitterBarProc(int ProcId, ZuiControl cp, ZuiSplitterBar p, ZuiAny Param1, ZuiAny Param2);


#endif	//__SplitterBar_H__

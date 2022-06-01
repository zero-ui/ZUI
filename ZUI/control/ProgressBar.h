#ifndef __ProgressBar_H__
#define __ProgressBar_H__
#include <ZUI.h>

//  进度条控件结构
#define PBSIZE 6
typedef struct _ZProgressBar
{
    ZuiColor pbColor;         //进度条颜色
    ZuiColor pbBkColor;       //进度条背景颜色

    int pbHeight;
    int pbPercent;

    ZCtlProc old_call;
}*ZuiProgressBar, ZProgressBar;
ZEXPORT ZuiAny ZCALL ZuiProgressBarProc(int ProcId, ZuiControl cp, ZuiProgressBar p, ZuiAny Param1, ZuiAny Param2);


#endif	//__ProgressBar_H__

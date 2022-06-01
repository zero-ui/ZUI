#ifndef __CheckBox_H__
#define __CheckBox_H__
#include <ZUI.h>



/**按钮控件结构*/
typedef struct _ZCheckBox
{

    ZCtlProc old_call;
}*ZuiCheckBox, ZCheckBox;
ZEXPORT ZuiAny ZCALL ZuiCheckBoxProc(int ProcId, ZuiControl cp, ZuiCheckBox p, ZuiAny Param1, ZuiAny Param2);


#endif	//__CheckBox_H__

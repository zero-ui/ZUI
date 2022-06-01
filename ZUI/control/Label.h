#ifndef __LABEL_H__
#define __LABEL_H__
#include <ZUI.h>

/**按钮控件结构*/
typedef struct _ZLabel
{
	ZRect		m_rcPadding;
    ZuiRes      m_rFont;         //字体
    unsigned int     m_uTextStyle;    //字体控制
    ZuiColor    m_cTextColor;   //字体颜色
    ZuiColor    m_cTextColorDisabled;
    ZCtlProc old_call;
}*ZuiLabel, ZLabel;
ZEXPORT ZuiAny ZCALL ZuiLabelProc(int ProcId, ZuiControl cp, ZuiLabel p, ZuiAny Param1, ZuiAny Param2);


#endif	//__LABEL_H__

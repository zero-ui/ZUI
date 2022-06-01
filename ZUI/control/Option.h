#ifndef __OPTION_H__
#define __OPTION_H__
#include <ZUI.h>


/**按钮控件结构*/
typedef struct _ZOption
{
    ZuiBool m_bSelected;        //是否选择
    ZuiBool m_bGroup;           //
    //控件继承自按钮控件
    ZuiRes      m_ResSelected;          //选中的普通状态
    ZuiRes      m_ResSelectedHot;       //选中的点燃状态
    ZuiRes      m_ResSelectedPushed;    //选中的按下状态
    ZuiRes      m_ResSelectedFocused;
    ZuiRes      m_ResSelectedDisabled;

    ZuiColor    m_ColorSelected;        //选中的普通状态
    ZuiColor    m_ColorSelectedHot;     //选中的点燃状态
    ZuiColor    m_ColorSelectedPushed;  //选中的按下状态
    ZuiColor    m_ColorSelectedFocused;
    ZuiColor    m_ColorSelectedDisabled;

    ZCtlProc old_call;
    ZuiAny old_udata;
}*ZuiOption, ZOption;
ZEXPORT ZuiAny ZCALL ZuiOptionProc(int ProcId, ZuiControl cp, ZuiOption p, ZuiAny Param1, ZuiAny Param2);


#endif  //__OPTION_H__

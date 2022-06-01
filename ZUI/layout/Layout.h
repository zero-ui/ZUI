#ifndef __LAYOUT_H__
#define __LAYOUT_H__
#include <ZUI.h>
#include <core/carray.h>


/**容器基类结构*/
typedef struct _ZuiLayout
{
    DArray *m_items;                /// 控件数组
    ZRect m_rcInset;                /// 内边距
    ZuiBool m_bMouseChildEnabled;
    int m_iChildPadding;
    unsigned int m_iChildAlign;
    unsigned int m_iChildVAlign;
    ZuiBool m_bFocused;             //是焦点状态
    ZuiBool m_bScrollProcess;       //  防止SetPos循环调用
    int  m_nScrollStepSize;

    ZuiControl m_pVerticalScrollBar;    //纵向滚动条
    ZuiControl m_pHorizontalScrollBar;  //横向滚动条

    ZCtlProc old_call;
}*ZuiLayout, ZLayout;

void* ZCALL ZuiLayoutProc(int ProcId, ZuiControl cp, ZuiLayout p, ZuiAny Param1, ZuiAny Param2);


#endif // __LAYOUT_H__

#ifndef __UISCROLLBAR_H__
#define __UISCROLLBAR_H__
#include <ZUI.h>


#define DEFAULT_SCROLLBAR_SIZE 16 //默认滚动条大小
#define DEFAULT_TIMERID 10 //时钟ID
#define SB_INSET   3 //滚动条控件内边距
#define COL_RES     4 //图片资源每行图片数量
#define ROW_RES     5 //图片资源行数
#define RES_SEP     2 //图片间隔

typedef struct _ZScrollBar
{
    ZuiBool m_bHorizontal; //是否横向
    int m_nRange;
    int m_nScrollPos;
    int m_nLineSize;
    ZuiControl m_pOwner;
    ZPoint ptLastMouse;
    int m_nLastScrollPos;
    int m_nLastScrollOffset;
    int m_nScrollRepeatDelay;
    ZuiRes m_sImageRes;         //控件图片资源
    ZRect m_rcArray[ROW_RES * COL_RES];        //资源坐标
    //颜色
    ZuiColor m_tNormalColor;    //滑块
    ZuiColor m_tHotColor;
    ZuiColor m_tPushColor;
    ZuiColor m_DisableColor;
    ZuiColor m_bNormalColor;    //按钮
    ZuiColor m_bHotColor;
    ZuiColor m_bPushColor;

    //头按钮
    ZuiBool m_bShowButton1;
    ZRect m_rcButton1;
    unsigned int m_uButton1State;

    //尾按钮
    ZuiBool m_bShowButton2;
    ZRect m_rcButton2;
    unsigned int m_uButton2State;

    //滑块按钮
    ZRect m_rcThumb;
    unsigned int m_uThumbState;

    ZCtlProc old_call;
}*ZuiScrollBar, ZScrollBar;
ZEXPORT ZuiAny ZCALL ZuiScrollBarProc(int ProcId, ZuiControl cp, ZuiScrollBar p, ZuiAny Param1, ZuiAny Param2);

#endif // __UISCROLLBAR_H__

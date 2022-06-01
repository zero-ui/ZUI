#ifndef __ZUI_PLATFORM_OS_LINUX_H__
#define __ZUI_PLATFORM_OS_LINUX_H__

#include <ZUI.h>
#include <core/carray.h>
#include <platform/platform.h>

#if (defined PLATFORM_OS_LINUX)

#define ZDC_ARROW 0

typedef struct _ZOsWindow
{
    ZRect m_rect;               //窗口矩形
    ZuiBool m_nobox;            //无边框
    ZuiBool m_bMax;             //是否最大化
    ZuiBool m_combo;            //组合窗口 失焦隐藏

    //下面参数用于事件调度

    ZuiGraphics m_hDcOffscreen;     //内存缓冲区绘图DC

    char m_nOpacity;                //窗口透明度
    ZuiBool m_bLayered;             //是否分层
    ZRect m_rcLayeredInset;         //
    ZuiBool m_bLayeredChanged;      //
    ZRect m_rcLayeredUpdate;        //
    ZuiBool m_bShowUpdateRect;      //是否显示更新区域 调试使用
    //

    //
    ZuiControl m_pRoot;             //控件树根节点
    ZuiControl m_pFocus;            //处于获得焦点状态的控件
    ZuiControl m_pEventHover;       //处于鼠标悬停状态的控件
    ZuiControl m_pEventClick;       //被鼠标点击的控件
    ZuiControl m_pEventKey;         //接收键盘输入的控件
    //
    ZuiBool m_bUnfocusPaintWindow;  //开启无焦点窗口
    ZuiBool m_bMouseTracking;       //是否需要支持鼠标追踪
    ZuiBool m_bMouseCapture;        //是否需要支持鼠标捕获
    ZuiBool m_bIsPainting;          //是否正在绘制
    //
    ZPoint m_ptLastMousePos;        //鼠标位置
    ZSize m_szMinWindow;             //窗口最小大小
    ZSize m_szMaxWindow;             //窗口最大大小
    ZSize m_szInitWindowSize;        //窗体初始化时的大小
    ZuiBool m_bFirstLayout;         //是否是第一次布局 相当于窗口初始化
    ZuiBool m_bUpdateNeeded;        //是否需要更新界面布局
    ZuiBool m_bFocusNeeded;         //是否需要焦点
    ZuiBool m_bOffscreenPaint;      //是否需要开双缓存绘图

    DArray *m_aTimers;              //时钟句柄数组
    DArray *m_aPostPaintControls;   //在绘制完成后发送绘制请求的控件集合
    DArray *m_aFoundControls;       //
    DArray *m_aDelayedCleanup;      //延时销毁的控件
} *ZuiOsWindow, ZOsWindow;

#endif //PLATFORM_OS_LINUX
#endif //__ZUI_PLATFORM_OS_LINUX_H__

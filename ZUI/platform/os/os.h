#ifndef __ZUI_PLATFORM_OS_H__
#define __ZUI_PLATFORM_OS_H__

#include <ZUI.h>

//事件类型定义
enum EVENTTYPE_UI
{
    ZEVENT__FIRST = 1,
    ZEVENT__KEYBEGIN,
    ZEVENT_KEYDOWN,
    ZEVENT_KEYUP,
    ZEVENT_CHAR,
    ZEVENT_SYSKEY,
    ZEVENT__KEYEND,
    ZEVENT__MOUSEBEGIN,
    ZEVENT_MOUSEMOVE,       //鼠标移动
    ZEVENT_MOUSELEAVE,      //鼠标离开
    ZEVENT_MOUSEENTER,      //鼠标进入
    ZEVENT_MOUSEHOVER,      //鼠标悬停
    ZEVENT_LBUTTONDOWN,     //
    ZEVENT_LBUTTONUP,       
    ZEVENT_RBUTTONDOWN,
    ZEVENT_LDBLCLICK,
    ZEVENT_CONTEXTMENU,
    ZEVENT_SCROLLWHEEL,
    ZEVENT__MOUSEEND,
    ZEVENT_KILLFOCUS,
    ZEVENT_SETFOCUS,
    ZEVENT_WINDOWSIZE,      
    ZEVENT_SETCURSOR,       //即将设置光标
    ZEVENT_TIMER,           //定时器
    ZEVENT_NOTIFY,
    ZEVENT_COMMAND,
    ZEVENT__LAST
};

//事件结构,由os层发出
typedef struct tagTEventUI
{
    enum EVENTTYPE_UI Type; //事件类型
    ZuiControl pSender;     //事件对应的控件
    unsigned long long dwTimestamp;     //时间戳
    ZPoint ptMouse;         //鼠标位置
    ZText chKey;            //
    int wKeyState;       //
    unsigned int wParam;         //
    unsigned int lParam;         //
} TEventUI;


//此函数用作初始化Os.
ZuiBool ZuiOsInitialize();
//此函数用作反初始化Os.
ZuiBool ZuiOsUnInitialize();
//此函数用作创建ZuiOsWindow对象.
ZuiOsWindow ZuiOsCreateWindow(ZuiControl root,ZuiBool show, ZuiAny pcontrol);
// 此函数用作销毁由ZuiOsCreateWindow创建的对象.
ZuiVoid ZuiOsDestroyWindow(ZuiOsWindow OsWindow);
//此函数用作设置窗口标题.
ZuiBool ZuiOsSetWindowTitle(ZuiOsWindow OsWindow, ZuiText Title);
//此函数用做最小化窗口
ZuiBool ZuiOsSetWindowMin(ZuiOsWindow OsWindow);
//此函数用做最大化窗口
ZuiBool ZuiOsSetWindowMax(ZuiOsWindow OsWindow);
// 此函数用做向下还原窗口
ZuiBool ZuiOsSetWindowRestor(ZuiOsWindow OsWindow);
ZuiBool ZuiOsSetWindowSize(ZuiOsWindow OsWindow, unsigned int w, unsigned int h);
ZuiBool ZuiOsSetWindowNoBox(ZuiOsWindow OsWindow, ZuiBool b);
ZuiBool ZuiOsSetWindowComBo(ZuiOsWindow OsWindow, ZuiBool b);
ZuiBool ZuiOsSetWindowTool(ZuiOsWindow OsWindow, ZuiBool b);

//此函数用作设置窗口可视状态.
ZuiVoid ZuiOsSetWindowVisible(ZuiOsWindow OsWindow, ZuiBool Visible);
ZuiVoid ZuiOsWindowPopup(ZuiOsWindow OsWindow, ZuiPoint pt);
ZuiVoid ZuiOsSetWindowCenter(ZuiOsWindow OsWindow);

//设置光标
ZuiVoid ZuiOsSetCursor(unsigned int type);
//俘获输入
ZuiVoid ZuiOsSetCapture(ZuiOsWindow OsWindow);
//释放俘获
ZuiVoid ZuiOsReleaseCapture(ZuiOsWindow OsWindow);
//指定区域失效
ZuiVoid ZuiOsInvalidate(ZuiOsWindow p);
//指定区域失效
ZuiVoid ZuiOsInvalidateRect(ZuiOsWindow p, ZRect *rcItem);

//创建时钟
ZuiBool ZuiOsSetTimer(ZuiControl pControl, unsigned int nTimerID, unsigned int uElapse);
//销毁时钟
ZuiBool ZuiOsKillTimer_Id(ZuiControl pControl, unsigned int nTimerID);
//销毁时钟
ZuiVoid ZuiOsKillTimer(ZuiControl pControl);
//销毁全部时钟
ZuiVoid ZuiOsRemoveAllTimers(ZuiOsWindow p);
//设置焦点控件
ZuiVoid ZuiOsSetFocus(ZuiOsWindow p, ZuiControl pControl, ZuiBool bFocusWnd);
//移除控件在系统层的对象
ZuiVoid ZuiOsReapObjects(ZuiOsWindow p, ZuiControl pControl);
//延迟销毁控件
ZuiVoid ZuiOsAddDelayedCleanup(ZuiControl pControl,ZuiAny Param1,ZuiAny Param2);
//系统消息循环
int ZuiOsMsgLoop();
ZEXPORT ZuiVoid ZuiOsPostMessage(ZuiControl cp, ZuiAny Msg, ZuiAny Param1, ZuiAny Param2);
ZEXPORT int ZuiDoModel(ZuiControl cp);
//退出系统消息循环
ZuiVoid ZuiOsMsgLoopExit(int nRet);
//投递一个任务到UI线程

int ZuiOsUtf8ToUnicode(ZuiAny str, int slen, ZuiText out, int olen);
int ZuiOsAsciiToUnicode(ZuiAny str, int slen, ZuiText out, int olen);
int ZuiOsUnicodeToAscii(ZuiText str, int slen, ZuiAny out, int olen);
int ZuiOsUnicodeToUtf8(ZuiText str, int slen, ZuiAny out, int olen);

ZuiVoid ZuiOsClientToScreen(ZuiControl p, ZuiPoint pt);
ZuiVoid ZuiOsScreenToClient(ZuiControl p, ZuiPoint pt);
#endif //__ZUI_PLATFORM_OS_H__
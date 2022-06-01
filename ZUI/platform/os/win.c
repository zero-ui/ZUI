#include <core/control.h>
#include <core/builder.h>
#include <core/function.h>
#include "win.h"
#if (defined PLATFORM_OS_WIN)
#pragma comment(lib, "Imm32.lib")
#pragma comment(lib, "Winmm.lib")

typedef BOOL(__stdcall *PFUNCUPDATELAYEREDWINDOW)(HWND, HDC, POINT*, SIZE*, HDC, POINT*, COLORREF, BLENDFUNCTION*, DWORD);

//分层刷新timerid
#define LAYEREDUPDATE_TIMERID   0x2000
static HPEN m_hUpdateRectPen = NULL;
HINSTANCE m_hInstance = NULL;           //模块句柄
static DWORD m_hMainThreadId = 0;    //主线程ID
static PFUNCUPDATELAYEREDWINDOW g_fUpdateLayeredWindow = NULL;	//UpdateLayeredWindow函数指针

//定时器结构
typedef struct tagTIMERINFO
{
    ZuiControl pSender; //接收时钟消息的控件
    int nLocalID;    //时钟ID
    HWND hWnd;          //窗口句柄
    int uWinTimer;
    ZuiBool bKilled;
} TIMERINFO;
#ifdef BUILDING_ZUI_SHARED
//动态库入口函数
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
#endif
//查找桌面句柄
static BOOL __stdcall enumUserWindowsCB(HWND hwnd, LPARAM lParam)
{
    long wflags = GetWindowLong(hwnd, GWL_STYLE);
    if (!(wflags & WS_VISIBLE)) return TRUE;

    HWND sndWnd;
    if (!(sndWnd = FindWindowEx(hwnd, NULL, _T("SHELLDLL_DefView"), NULL))) return TRUE;
    HWND targetWnd;
    if (!(targetWnd = FindWindowEx(sndWnd, NULL, _T("SysListView32"), _T("FolderView")))) return TRUE;

    HWND* resultHwnd = (HWND*)lParam;
    *resultHwnd = targetWnd;
    return FALSE;
}
static  HWND findDesktopIconWnd()
{
    HWND resultHwnd = NULL;
    EnumWindows((WNDENUMPROC)enumUserWindowsCB, (LPARAM)&resultHwnd);
    return resultHwnd;
}
//功能键检查
static int MapKeyState()
{
    int uState = 0;
    if (GetKeyState(VK_CONTROL) < 0) uState |= MK_CONTROL;
    if (GetKeyState(VK_RBUTTON) < 0) uState |= MK_RBUTTON;
    if (GetKeyState(VK_LBUTTON) < 0) uState |= MK_LBUTTON;
    if (GetKeyState(VK_SHIFT) < 0) uState |= MK_SHIFT;
    if (GetKeyState(VK_MENU) < 0) uState |= MK_ALT;
    return uState;
}
//窗口消息回调
static LRESULT WINAPI __WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    ZuiOsWindow p = NULL;
    if (uMsg == WM_NCCREATE) {
        LPCREATESTRUCT lpcs = (LPCREATESTRUCT)(lParam);
        p = (ZuiOsWindow)(lpcs->lpCreateParams);
        p->m_hWnd = hWnd;
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LPARAM)p);
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    else {
        p = (ZuiOsWindow)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        if (uMsg == WM_NCDESTROY && p != NULL) {
            LRESULT lRes = DefWindowProc(hWnd, uMsg, wParam, lParam);
            SetWindowLongPtr(p->m_hWnd, GWLP_USERDATA, 0L);
            p->m_hWnd = NULL;
            return lRes;
        }
    }

    // 事件的自定义处理
    switch (uMsg) {
    case WM_APP + 1:
    {
        while (darray_len(p->m_aDelayedCleanup)) {
            ZuiControl cp = p->m_aDelayedCleanup->data[0];
            darray_delete(p->m_aDelayedCleanup, darray_find(p->m_aDelayedCleanup, cp));
            if (cp == p->m_pRoot) {
                //无焦点窗口不做任何处理
                if (!p->m_bUnfocusPaintWindow)
                {
                    SetFocus(NULL);
                }

                if (GetActiveWindow() == p->m_hWnd) {
                    HWND hwndParent = GetWindowOwner(p->m_hWnd);
                    //无焦点窗口不做任何处理
                    if (!p->m_bUnfocusPaintWindow)
                    {
                        if (hwndParent != NULL) SetFocus(hwndParent);
                    }
                }
            }
            if (darray_len(p->m_aDelayedCleanup) == 0) {
                ZuiControlCall(Proc_OnDestroy, cp, (ZuiAny)wParam, (ZuiAny)lParam);
                break;
            }
            else
                ZuiControlCall(Proc_OnDestroy, cp, (ZuiAny)wParam, (ZuiAny)lParam);
        };
        return 0;
    }
    case WM_CLOSE:  //关闭窗口
    {
        // Make sure all matching "closing" events are sent
        TEventUI event = { 0 };
        event.ptMouse = p->m_ptLastMousePos;
        event.wKeyState = MapKeyState();
        event.dwTimestamp = GetTickCount64();
        if (p->m_pEventHover != NULL) {
            event.Type = ZEVENT_MOUSELEAVE;
            event.pSender = p->m_pEventHover;
            ZuiControlEvent(p->m_pEventHover, &event);
        }
        if (p->m_pEventClick != NULL) {
            event.Type = ZEVENT_LBUTTONUP;
            event.pSender = p->m_pEventClick;
            ZuiControlEvent(p->m_pEventClick, &event);
        }

        ZuiControlCall(Proc_OnClose, p->m_pRoot, 0, 0);
        return 0;
    }
    case WM_ERASEBKGND:
    {
        //不允许背景擦除,防止闪屏
        return 1;
    }
    case WM_NCCALCSIZE:
    {
        if (wParam) {
            NCCALCSIZE_PARAMS* ncP = (NCCALCSIZE_PARAMS*)lParam;
            RECT aRect;
            RECT bRect;

            CopyRect(&aRect, &ncP->rgrc[1]);
            CopyRect(&bRect, &ncP->rgrc[0]);
            if (IsZoomed(hWnd)) {
                bRect.left += GetSystemMetrics(SM_CXSIZEFRAME);
                bRect.right -= GetSystemMetrics(SM_CXSIZEFRAME);
                bRect.top += GetSystemMetrics(SM_CYSIZEFRAME);
                bRect.bottom -= GetSystemMetrics(SM_CYSIZEFRAME);
            }
            CopyRect(&ncP->rgrc[0], &bRect);
            CopyRect(&ncP->rgrc[1], &bRect);
            CopyRect(&ncP->rgrc[2], &aRect);
        }
        return TRUE;
    }
    case WM_NCACTIVATE:
    case WM_NCPAINT:
    {
        ZuiOsInvalidate(p);
        return 1;
    }
    case WM_PAINT:  //绘制
    {
        //wprintf(L"paint...");
        PAINTSTRUCT ps = { 0 };
        if (p->m_pRoot == NULL) {	//没有控件树
            BeginPaint(p->m_hWnd, &ps);
            //CRenderEngine::DrawColor(p->m_hDcPaint, ps.rcPaint, 0xFFFF0000);
            EndPaint(p->m_hWnd, &ps);
            return 0;
        }

        RECT rcClient = { 0 };
        GetClientRect(p->m_hWnd, &rcClient);
        if (rcClient.right - rcClient.left == 0 || rcClient.bottom - rcClient.top == 0)
        {
            BeginPaint(p->m_hWnd, &ps);
            EndPaint(p->m_hWnd, &ps);
            return 0;
        }
        RECT rcPaint = { 0 };
        if (p->m_bLayered) {
            p->m_bOffscreenPaint = TRUE;
            rcPaint.bottom = p->m_rcLayeredUpdate.bottom;
            rcPaint.left = p->m_rcLayeredUpdate.left;
            rcPaint.right = p->m_rcLayeredUpdate.right;
            rcPaint.top = p->m_rcLayeredUpdate.top;
            if (IsRectEmpty((LPRECT)&p->m_rcLayeredUpdate)) {
                BeginPaint(p->m_hWnd, &ps);
                EndPaint(p->m_hWnd, &ps);
                return TRUE;
            }
            if (rcPaint.right > rcClient.right) rcPaint.right = rcClient.right;
            if (rcPaint.bottom > rcClient.bottom) rcPaint.bottom = rcClient.bottom;
            memset(&p->m_rcLayeredUpdate, 0, sizeof(p->m_rcLayeredUpdate));
        }
        else {
            if (!GetUpdateRect(p->m_hWnd, &rcPaint, FALSE)) return TRUE;
        }
        p->m_bIsPainting = TRUE;
        //是否需要更新控件布局
        if (p->m_bUpdateNeeded) {	//更新控件布局
            p->m_bUpdateNeeded = FALSE;
            if (!IsRectEmpty(&rcClient)) {
                if (p->m_pRoot->m_bUpdateNeeded) {
                    RECT rcRoot = rcClient;
                    //if (p->m_bOffscreenPaint)
                    //    ZuiDestroyGraphics(p->m_hDcOffscreen);
                    //p->m_hDcOffscreen = NULL;
                    if (p->m_bLayered) {
                        rcRoot.left += p->m_rcLayeredInset.left;
                        rcRoot.top += p->m_rcLayeredInset.top;
                        rcRoot.right -= p->m_rcLayeredInset.right;
                        rcRoot.bottom -= p->m_rcLayeredInset.bottom;
                    }
                    p->m_pRoot->m_bUpdateNeeded = FALSE;
                    ZuiControlCall(Proc_SetPos, p->m_pRoot, &rcRoot, (void *)TRUE);
                }
                else {
                    ZuiControl pControl = NULL;
                    darray_empty(p->m_aFoundControls);
                    ZuiControlCall(Proc_FindControl, p->m_pRoot, NULL, (void *)(ZFIND_FROM_UPDATE | ZFIND_VISIBLE | ZFIND_ME_FIRST | ZFIND_UPDATETEST));
                    for (int it = 0; it < darray_len(p->m_aFoundControls); it++) {
                        pControl = (ZuiControl)(p->m_aFoundControls->data[it]);
                        if (!pControl->m_bFloat)
                            ZuiControlCall(Proc_SetPos, pControl, (ZRect *)ZuiControlCall(Proc_GetPos, pControl, NULL, NULL), (void *)TRUE);
                        else {
                            RECT rcP;
                            ZuiControlCall(Proc_GetRelativePos, pControl, &rcP, NULL);
                            ZuiControlCall(Proc_SetPos, pControl, &rcP, (void *)TRUE);
                        }
                    }
                }
                // We'll want to notify the window when it is first initialized
                // with the correct layout. The window form would take the time
                // to submit swipes/animations.
                if (p->m_bFirstLayout) {
                    p->m_bFirstLayout = FALSE;
                    //第一次更新布局完成 相当于窗口初始化完成
                    if (p->m_bLayered && p->m_bLayeredChanged) {
                        ZuiOsInvalidate(p);
                        p->m_bIsPainting = FALSE;
                        return TRUE;
                    }
                }
            }
        }
        else if (p->m_bLayered && p->m_bLayeredChanged) {
            RECT rcRoot = rcClient;
            rcRoot.left += p->m_rcLayeredInset.left;
            rcRoot.top += p->m_rcLayeredInset.top;
            rcRoot.right -= p->m_rcLayeredInset.right;
            rcRoot.bottom -= p->m_rcLayeredInset.bottom;
            ZuiControlCall(Proc_SetPos, p->m_pRoot, &rcRoot, (void *)TRUE);
        }
        // Set focus to first control?
        if (p->m_bFocusNeeded) {
            // If we're in the process of restructuring the layout we can delay the
            // focus calulation until the next repaint.
            if (p->m_bUpdateNeeded) {
                p->m_bFocusNeeded = TRUE;
                InvalidateRect(p->m_hWnd, NULL, FALSE);
                return TRUE;
            }
            // Find next/previous tabbable control
            FINDTABINFO info1 = { 0 };
            info1.pFocus = p->m_pFocus;
            info1.bForward = TRUE;
            ZuiControl pControl = (ZuiControl)ZuiControlCall(Proc_FindControl, p->m_pRoot, &info1, (void *)(ZFIND_FROM_TAB | ZFIND_VISIBLE | ZFIND_ENABLED | ZFIND_ME_FIRST));
            if (pControl == NULL) {
                // Wrap around
                FINDTABINFO info2 = { 0 };
                info2.pFocus = NULL;
                info2.bForward = TRUE;
                pControl = (ZuiControl)ZuiControlCall(Proc_FindControl, p->m_pRoot, &info2, (void *)(ZFIND_FROM_TAB | ZFIND_VISIBLE | ZFIND_ENABLED | ZFIND_ME_FIRST));
            }
            if (pControl != NULL) ZuiOsSetFocus(p, pControl, TRUE);
            p->m_bFocusNeeded = FALSE;
            return TRUE;
        }
        //
        // 渲染屏幕
        //

        // 开始窗口绘制
        BeginPaint(p->m_hWnd, &ps);
        //是否双缓存绘图
        if (p->m_bOffscreenPaint)
        {
            ZuiControlCall(Proc_OnPaint, p->m_pRoot, p->m_hDcOffscreen, &rcPaint);

            for (int i = 0; i < darray_len(p->m_aPostPaintControls); i++) {
                ZuiControl pPostPaintControl = (ZuiControl)(p->m_aPostPaintControls->data[i]);
                ZuiControlCall(Proc_OnPostPaint, pPostPaintControl, p->m_hDcOffscreen, &rcPaint);
            }
            //RestoreDC(p->m_hDcOffscreen->hdc, iSaveDC);
            if (p->m_bLayered) {
                RECT rcWnd = { 0 };
                GetWindowRect(p->m_hWnd, &rcWnd);
                DWORD dwWidth = rcClient.right - rcClient.left;
                DWORD dwHeight = rcClient.bottom - rcClient.top;
                RECT rcLayeredClient = rcClient;
                rcLayeredClient.left += p->m_rcLayeredInset.left;
                rcLayeredClient.top += p->m_rcLayeredInset.top;
                rcLayeredClient.right -= p->m_rcLayeredInset.right;
                rcLayeredClient.bottom -= p->m_rcLayeredInset.bottom;


                BLENDFUNCTION bf = { AC_SRC_OVER, 0, p->m_nOpacity, AC_SRC_ALPHA };
                POINT ptPos = { rcWnd.left, rcWnd.top };
                SIZE sizeWnd = { dwWidth, dwHeight };
                POINT ptSrc = { 0, 0 };
                g_fUpdateLayeredWindow(p->m_hWnd, p->m_hDcPaint, &ptPos, &sizeWnd, p->m_hDcOffscreen->hdc, &ptSrc, 0, &bf, ULW_ALPHA);
            }
            else
                BitBlt(p->m_hDcPaint, rcPaint.left, rcPaint.top, rcPaint.right - rcPaint.left, rcPaint.bottom - rcPaint.top, p->m_hDcOffscreen->hdc, rcPaint.left, rcPaint.top, SRCCOPY);

            if (p->m_bShowUpdateRect && !p->m_bLayered) { //绘制更新矩形
                HPEN hOldPen = (HPEN)SelectObject(p->m_hDcPaint, m_hUpdateRectPen);
                SelectObject(p->m_hDcPaint, GetStockObject(HOLLOW_BRUSH));
                Rectangle(p->m_hDcPaint, rcPaint.left, rcPaint.top, rcPaint.right, rcPaint.bottom);
                SelectObject(p->m_hDcPaint, hOldPen);
            }
        }
        else
        {
            // A standard paint job
            p->m_hDcOffscreen->hdc = GetDC(hWnd);
            ZuiControlCall(Proc_OnPaint, p->m_pRoot, p->m_hDcOffscreen, &rcPaint);
        }
        // 全部完毕!
        EndPaint(p->m_hWnd, &ps);

        p->m_bIsPainting = FALSE;
        p->m_bLayeredChanged = FALSE;
        if (p->m_bUpdateNeeded) ZuiOsInvalidate(p);
        return TRUE;
    }
    case WM_PRINTCLIENT:
    {
        if (p->m_pRoot == NULL) break;
        RECT rcClient;
        GetClientRect(p->m_hWnd, &rcClient);
        HDC hDC = (HDC)wParam;
        int save = SaveDC(hDC);
        ZuiControlCall(Proc_OnPaint, p->m_pRoot, hDC, &rcClient);
        // Check for traversing children. The crux is that WM_PRINT will assume
        // that the DC is positioned at frame coordinates and will paint the child
        // control at the wrong position. We'll simulate the entire thing instead.
        if ((lParam & PRF_CHILDREN) != 0) {
            HWND hWndChild = GetWindow(p->m_hWnd, GW_CHILD);
            while (hWndChild != NULL) {
                RECT rcPos = { 0 };
                GetWindowRect(hWndChild, &rcPos);
                MapWindowPoints(HWND_DESKTOP, p->m_hWnd, (LPPOINT)(&rcPos), 2);
                SetWindowOrgEx(hDC, -rcPos.left, -rcPos.top, NULL);
                // NOTE: We use WM_PRINT here rather than the expected WM_PRINTCLIENT
                //       since the latter will not print the nonclient correctly for
                //       EDIT controls.
                SendMessage(hWndChild, WM_PRINT, wParam, lParam | PRF_NONCLIENT);
                hWndChild = GetWindow(hWndChild, GW_HWNDNEXT);
            }
        }
        RestoreDC(hDC, save);
        break;
    }
    case WM_GETMINMAXINFO:  //取窗口最大最小位置
    {
        LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
        if (p) {
            if (p->m_szMinWindow.cx > 0) lpMMI->ptMinTrackSize.x = p->m_szMinWindow.cx;
            if (p->m_szMinWindow.cy > 0) lpMMI->ptMinTrackSize.y = p->m_szMinWindow.cy;
            if (p->m_szMaxWindow.cx > 0) lpMMI->ptMaxTrackSize.x = p->m_szMaxWindow.cx;
            if (p->m_szMaxWindow.cy > 0) lpMMI->ptMaxTrackSize.y = p->m_szMaxWindow.cy;
        }
        lpMMI->ptMaxSize.y = GetSystemMetrics(SM_CYFULLSCREEN) + GetSystemMetrics(SM_CYCAPTION);// +GetSystemMetrics(SM_CYDLGFRAME);
        break;
    }
    case WM_SIZE:   //大小被改变
    {
        GetWindowRect(hWnd, (LPRECT)&p->m_rect);
        RECT tmprc;
        HRGN tmprgn;
        GetClientRect(p->m_hWnd, &tmprc);
        if (wParam == SIZE_MAXIMIZED) {
            tmprc.left += GetSystemMetrics(SM_CXSIZEFRAME);
            tmprc.right += GetSystemMetrics(SM_CXSIZEFRAME);
            tmprc.top += GetSystemMetrics(SM_CYSIZEFRAME);
            tmprc.bottom += GetSystemMetrics(SM_CYSIZEFRAME);
            if (!p->m_bMax) {
                p->m_bMax = TRUE;
            }
        }
        int w = 0, h = 0;
        if (p){
            w = p->m_pRoot->m_rRound.cx;
            h = p->m_pRoot->m_rRound.cy;
        }
        tmprgn = CreateRoundRectRgn(tmprc.left, tmprc.top, tmprc.right+1, tmprc.bottom+1,w,h);
        SetWindowRgn(p->m_hWnd, tmprgn, TRUE);
        DeleteObject(tmprgn);
        if (p->m_pRoot != NULL) {
            TEventUI event = { 0 };
            event.Type = ZEVENT_WINDOWSIZE;
            event.pSender = p->m_pFocus;
            event.wParam = wParam;
            event.lParam = lParam;
            event.dwTimestamp = GetTickCount64();
            event.ptMouse = p->m_ptLastMousePos;
            event.wKeyState = MapKeyState();
            ZuiControlEvent(p->m_pRoot, &event);
        }
        if (wParam == SIZE_RESTORED) {
            p->m_bMax = FALSE;
        }
        // 创建缓存图形
        if (p->m_bOffscreenPaint)
        {
            ZuiDestroyGraphics(p->m_hDcOffscreen);
            p->m_hDcOffscreen = ZuiCreateGraphics(LOWORD(lParam),HIWORD(lParam));
        }
        else {
            if (p->m_hDcOffscreen == NULL)
            {
                p->m_hDcOffscreen = ZuiCreateGraphics(0, 0);
            }
        }
        if (p->m_pRoot != NULL)
            ZuiControlNeedUpdate(p->m_pRoot);
        if (p->m_bLayered)
            ZuiOsInvalidate(p);
        return 0;
    }
    case WM_TIMER:  //时钟事件
    {
        if (LOWORD(wParam) == LAYEREDUPDATE_TIMERID) {
            if (p->m_bLayered && !IsRectEmpty((LPRECT)&p->m_rcLayeredUpdate)) {
                LRESULT lRes = 0;
                if (!IsIconic(p->m_hWnd))
                    __WndProc(p->m_hWnd, WM_PAINT, 0, 0L);
                break;
            }
        }
        for (int i = 0; i < darray_len(p->m_aTimers); i++) {
            const TIMERINFO* pTimer = (TIMERINFO*)(p->m_aTimers->data[i]);
            if (pTimer->hWnd == p->m_hWnd && pTimer->uWinTimer == LOWORD(wParam) && pTimer->bKilled == FALSE) {
                TEventUI event = { 0 };
                event.Type = ZEVENT_TIMER;
                event.pSender = pTimer->pSender;
                event.dwTimestamp = GetTickCount64();
                event.ptMouse = p->m_ptLastMousePos;
                event.wKeyState = MapKeyState();
                event.wParam = pTimer->nLocalID;
                event.lParam = lParam;
                ZuiControlEvent(pTimer->pSender, &event);
                break;
            }
        }
        break;
    }
    case WM_MOUSEHOVER: //鼠标悬停
    {
        if (p->m_pRoot == NULL) break;
        p->m_bMouseTracking = FALSE;
        ZPoint pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        ZuiControl pHover = NULL;
        if (p->m_pRoot)
            pHover = p->m_pRoot->call(Proc_FindControl, p->m_pRoot, p->m_pRoot->m_sUserData, &pt, (void *)(ZFIND_FROM_POINT | ZFIND_VISIBLE | ZFIND_HITTEST | ZFIND_TOP_FIRST));
        if (pHover == NULL) break;
        // Generate mouse hover event
        if (p->m_pEventHover != NULL) {
            TEventUI event = { 0 };
            event.Type = ZEVENT_MOUSEHOVER;
            event.pSender = p->m_pEventHover;
            event.wParam = wParam;
            event.lParam = lParam;
            event.dwTimestamp = GetTickCount64();
            event.ptMouse = pt;
            event.wKeyState = MapKeyState();
            ZuiControlEvent(p->m_pEventHover, &event);
        }
        // Create tooltip information
        if (!pHover->m_sToolTip)
            return TRUE;
        memset(&p->m_ToolTip, 0, sizeof(TOOLINFO));
        p->m_ToolTip.cbSize = sizeof(TOOLINFO);
        p->m_ToolTip.uFlags = TTF_IDISHWND;
        p->m_ToolTip.hwnd = p->m_hWnd;
        p->m_ToolTip.uId = (UINT_PTR)p->m_hWnd;
        p->m_ToolTip.hinst = m_hInstance;
        p->m_ToolTip.lpszText = (LPWSTR)pHover->m_sToolTip;
        memcpy(&p->m_ToolTip.rect, (ZRect *)ZuiControlCall(Proc_GetPos, pHover, NULL, NULL), sizeof(ZRect));
        if (p->m_hwndTooltip == NULL) {
            p->m_hwndTooltip = CreateWindowEx(0, TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, p->m_hWnd, NULL, m_hInstance, NULL);
            if (p->m_hwndTooltip != NULL && p->m_iTooltipWidth >= 0) {
                p->m_iTooltipWidth = (int)SendMessage(p->m_hwndTooltip, TTM_SETMAXTIPWIDTH, 0, p->m_iTooltipWidth);
            }
            SendMessage(p->m_hwndTooltip, TTM_ADDTOOL, 0, (LPARAM)&p->m_ToolTip);
        }
        SendMessage(p->m_hwndTooltip, TTM_SETMAXTIPWIDTH, 0, pHover->m_nTooltipWidth);
        SendMessage(p->m_hwndTooltip, TTM_SETTOOLINFO, 0, (LPARAM)&p->m_ToolTip);
        SendMessage(p->m_hwndTooltip, TTM_TRACKACTIVATE, TRUE, (LPARAM)&p->m_ToolTip);
        return 0;
    }
    case WM_MOUSELEAVE: //鼠标离开
    {
        if (p->m_pRoot == NULL) break;
        if (p->m_hwndTooltip != NULL) SendMessage(p->m_hwndTooltip, TTM_TRACKACTIVATE, FALSE, (LPARAM)&p->m_ToolTip);
        if (p->m_bMouseTracking) {
            ZPoint pt = { 0 };
            RECT rcWnd = { 0 };
            GetCursorPos((LPPOINT)&pt);
            GetWindowRect(p->m_hWnd, &rcWnd);
            if (!IsIconic(p->m_hWnd) && GetActiveWindow() == p->m_hWnd && ZuiIsPointInRect((ZuiRect)&rcWnd, &pt)) {
                if (SendMessage(p->m_hWnd, WM_NCHITTEST, 0, MAKELPARAM(pt.x, pt.y)) == HTCLIENT) {
                    ScreenToClient(p->m_hWnd, (LPPOINT)&pt);
                    SendMessage(p->m_hWnd, WM_MOUSEMOVE, 0, MAKELPARAM(pt.x, pt.y));
                }
                else
                    SendMessage(p->m_hWnd, WM_MOUSEMOVE, 0, (LPARAM)-1);
            }
            else
                SendMessage(p->m_hWnd, WM_MOUSEMOVE, 0, (LPARAM)-1);
        }
        p->m_bMouseTracking = FALSE;
        break;
    }
    case WM_MOUSEMOVE:  //鼠标移动
    {
        if (p->m_pRoot == NULL) break;
        // Start tracking this entire window again...
        if (!p->m_bMouseTracking) {
            TRACKMOUSEEVENT tme = { 0 };
            tme.cbSize = sizeof(TRACKMOUSEEVENT);
            tme.dwFlags = TME_HOVER | TME_LEAVE;
            tme.hwndTrack = p->m_hWnd;
            tme.dwHoverTime = p->m_hwndTooltip == NULL ? p->m_iHoverTime : (DWORD)SendMessage(p->m_hwndTooltip, TTM_GETDELAYTIME, TTDT_INITIAL, 0L);
            _TrackMouseEvent(&tme);
            p->m_bMouseTracking = TRUE;
        }
        // Generate the appropriate mouse messages
        ZPoint pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        p->m_ptLastMousePos = pt;
        ZuiControl pNewHover = NULL;
        if (p->m_pRoot)
            pNewHover = p->m_pRoot->call(Proc_FindControl, p->m_pRoot, p->m_pRoot->m_sUserData, &pt, (void *)(ZFIND_FROM_POINT | ZFIND_VISIBLE | ZFIND_ENABLED | ZFIND_HITTEST | ZFIND_TOP_FIRST));
        if (pNewHover != NULL && pNewHover->m_pOs != p) break;
        TEventUI event = { 0 };
        event.ptMouse = pt;
        event.wParam = wParam;
        event.lParam = lParam;
        event.dwTimestamp = GetTickCount64();
        event.wKeyState = MapKeyState();
        if (!p->m_bMouseCapture) {
            pNewHover = NULL;
            if (p->m_pRoot)
                pNewHover = p->m_pRoot->call(Proc_FindControl, p->m_pRoot, p->m_pRoot->m_sUserData, &pt, (void *)(ZFIND_FROM_POINT | ZFIND_VISIBLE | ZFIND_ENABLED | ZFIND_HITTEST | ZFIND_TOP_FIRST));
            if (pNewHover != NULL && pNewHover->m_pOs != p) break;
            if (pNewHover != p->m_pEventHover && p->m_pEventHover != NULL) {
                event.Type = ZEVENT_MOUSELEAVE;
                event.pSender = p->m_pEventHover;
                ZuiControlEvent(p->m_pEventHover, &event);
                p->m_pEventHover = NULL;
                if (p->m_hwndTooltip != NULL) SendMessage(p->m_hwndTooltip, TTM_TRACKACTIVATE, FALSE, (LPARAM)&p->m_ToolTip);
            }
            if (pNewHover != p->m_pEventHover && pNewHover != NULL) {
                event.Type = ZEVENT_MOUSEENTER;
                event.pSender = pNewHover;
                ZuiControlEvent(pNewHover, &event);
                p->m_pEventHover = pNewHover;
            }
        }
        if (p->m_pEventClick != NULL) {
            event.Type = ZEVENT_MOUSEMOVE;
            event.pSender = p->m_pEventClick;
            ZuiControlEvent(p->m_pEventClick, &event);
        }
        else if (pNewHover != NULL) {
            event.Type = ZEVENT_MOUSEMOVE;
            event.pSender = pNewHover;
            ZuiControlEvent(pNewHover, &event);
        }
        break;
    }
    case WM_LBUTTONDOWN://鼠标左键按下
    {
        // We alway set focus back to our app (this helps
        // when Win32 child windows are placed on the dialog
        // and we need to remove them on focus change).
        //无焦点窗口不做任何处理
        if (!p->m_bUnfocusPaintWindow)
        {
            SetFocus(p->m_hWnd);
        }
        if (p->m_pRoot == NULL) break;
        ZPoint pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        p->m_ptLastMousePos = pt;
        ZuiControl pControl = NULL;
        if (p->m_pRoot)
            pControl = p->m_pRoot->call(Proc_FindControl, p->m_pRoot, p->m_pRoot->m_sUserData, &pt, (void *)(ZFIND_FROM_POINT | ZFIND_VISIBLE | ZFIND_ENABLED | ZFIND_HITTEST | ZFIND_TOP_FIRST));
        if (pControl == NULL) break;
        if (pControl->m_pOs != p) break;
        if (pControl->m_drag) {
            TEventUI event = { 0 };
            event.Type = ZEVENT_KILLFOCUS;
            event.pSender = pControl;
            event.dwTimestamp = GetTickCount64();
            ZuiControlEvent(p->m_pFocus, &event);
            p->m_pFocus = NULL;
            return SendMessage(pControl->m_pOs->m_hWnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
        }
        p->m_pEventClick = pControl;
        ZuiControlCall(Proc_SetFocus, pControl, 0, 0);
        ZuiOsSetCapture(p);
        TEventUI event = { 0 };
        event.Type = ZEVENT_LBUTTONDOWN;
        event.pSender = pControl;
        event.wParam = wParam;
        event.lParam = lParam;
        event.ptMouse = pt;
        event.wKeyState = (WORD)wParam;
        event.dwTimestamp = GetTickCount64();
        ZuiControlEvent(pControl, &event);
        break;
    }
    case WM_LBUTTONDBLCLK://鼠标左键双击
    {
        //无焦点窗口不做任何处理
        if (!p->m_bUnfocusPaintWindow)
        {
            SetFocus(p->m_hWnd);
        }
        ZPoint pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        p->m_ptLastMousePos = pt;
        ZuiControl pControl = NULL;
        if (p->m_pRoot)
            pControl = p->m_pRoot->call(Proc_FindControl, p->m_pRoot, p->m_pRoot->m_sUserData, &pt, (void *)(ZFIND_FROM_POINT | ZFIND_VISIBLE | ZFIND_ENABLED | ZFIND_HITTEST | ZFIND_TOP_FIRST));
        if (pControl == NULL) break;
        if (pControl->m_pOs != p) break;
        ZuiOsSetCapture(p);
        TEventUI event = { 0 };
        event.Type = ZEVENT_LDBLCLICK;
        event.pSender = pControl;
        event.ptMouse = pt;
        event.wKeyState = (WORD)wParam;
        event.dwTimestamp = GetTickCount64();
        ZuiControlEvent(pControl, &event);
        p->m_pEventClick = pControl;
        break;
    }
    case WM_LBUTTONUP://鼠标左键弹起
    {
        ZPoint pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        p->m_ptLastMousePos = pt;
        if (p->m_pEventClick == NULL) break;
        ZuiOsReleaseCapture(p);
        TEventUI event = { 0 };
        event.Type = ZEVENT_LBUTTONUP;
        event.pSender = p->m_pEventClick;
        event.wParam = wParam;
        event.lParam = lParam;
        event.ptMouse = pt;
        event.wKeyState = (WORD)wParam;
        event.dwTimestamp = GetTickCount64();
        ZuiControl pClick = p->m_pEventClick;
        p->m_pEventClick = NULL;
        ZuiControlEvent(pClick, &event);
        break;
    }
    case WM_RBUTTONDOWN://鼠标右键按下
    {
        //无焦点窗口不做任何处理
        if (!p->m_bUnfocusPaintWindow)
        {
            SetFocus(p->m_hWnd);
        }
        ZPoint pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        p->m_ptLastMousePos = pt;
        ZuiControl pControl = NULL;
        if (p->m_pRoot)
            pControl = p->m_pRoot->call(Proc_FindControl, p->m_pRoot, p->m_pRoot->m_sUserData, &pt, (void *)(ZFIND_FROM_POINT | ZFIND_VISIBLE | ZFIND_HITTEST | ZFIND_TOP_FIRST));
        if (pControl == NULL) break;
        if (pControl->m_pOs != p) break;
        ZuiControlCall(Proc_SetFocus, pControl, 0, 0);
        TEventUI event = { 0 };
        event.Type = ZEVENT_RBUTTONDOWN;
        event.pSender = pControl;
        event.wParam = wParam;
        event.lParam = lParam;
        event.ptMouse = pt;
        event.wKeyState = (WORD)wParam;
        event.dwTimestamp = GetTickCount64();
        ZuiControlEvent(pControl, &event);
        p->m_pEventClick = pControl;
        break;
    }
    case WM_CONTEXTMENU:
    {
        if (p->m_pRoot == NULL) break;
        if (p->m_bMouseCapture) break;
        ZPoint pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        ScreenToClient(p->m_hWnd, (LPPOINT)&pt);
        p->m_ptLastMousePos = pt;
        if (p->m_pEventClick == NULL) break;
        TEventUI event = { 0 };
        event.Type = ZEVENT_CONTEXTMENU;
        event.pSender = p->m_pEventClick;
        event.ptMouse = pt;
        event.wKeyState = (WORD)wParam;
        event.lParam = (LPARAM)p->m_pEventClick;
        event.dwTimestamp = GetTickCount64();
        ZuiControlEvent(p->m_pEventClick, &event);
        p->m_pEventClick = NULL;
        break;
    }
    case WM_MOUSEWHEEL: //鼠标滚动
    {
        if (p->m_pRoot == NULL) break;
        ZPoint pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        ScreenToClient(p->m_hWnd, (LPPOINT)&pt);
        p->m_ptLastMousePos = pt;
        ZuiControl pControl = NULL;
        if (p->m_pRoot)
            pControl = p->m_pRoot->call(Proc_FindControl, p->m_pRoot, p->m_pRoot->m_sUserData, &pt, (void *)(ZFIND_FROM_POINT | ZFIND_VISIBLE | ZFIND_HITTEST | ZFIND_TOP_FIRST));
        if (pControl == NULL) break;
        if (pControl->m_pOs != p) break;
        int zDelta = (int)(short)HIWORD(wParam);
        TEventUI event = { 0 };
        event.Type = ZEVENT_SCROLLWHEEL;
        event.pSender = pControl;
        event.wParam = MAKELPARAM(zDelta < 0 ? SB_LINEDOWN : SB_LINEUP, 0);
        event.lParam = lParam;
        event.wKeyState = MapKeyState();
        event.dwTimestamp = GetTickCount64();
        ZuiControlEvent(pControl, &event);

        // Let's make sure that the scroll item below the cursor is the same as before...
        SendMessage(p->m_hWnd, WM_MOUSEMOVE, 0, (LPARAM)MAKELPARAM(p->m_ptLastMousePos.x, p->m_ptLastMousePos.y));
        break;
    }
    case WM_CHAR:
    {
        if (p->m_pRoot == NULL) break;
        if (p->m_pFocus == NULL) break;
        TEventUI event = { 0 };
        event.Type = ZEVENT_CHAR;
        event.pSender = p->m_pFocus;
        event.wParam = wParam;
        event.lParam = lParam;
        event.chKey = (TCHAR)wParam;
        event.ptMouse = p->m_ptLastMousePos;
        event.wKeyState = MapKeyState();
        event.dwTimestamp = GetTickCount64();
        ZuiControlEvent(p->m_pFocus, &event);
        break;
    }
    case WM_KEYDOWN://按下某键
    {
        if (p->m_pRoot == NULL) break;
        if (p->m_pFocus == NULL) break;
        TEventUI event = { 0 };
        event.Type = ZEVENT_KEYDOWN;
        event.pSender = p->m_pFocus;
        event.wParam = wParam;
        event.lParam = lParam;
        event.chKey = (TCHAR)wParam;
        event.ptMouse = p->m_ptLastMousePos;
        event.wKeyState = MapKeyState();
        event.dwTimestamp = GetTickCount64();
        ZuiControlEvent(p->m_pFocus, &event);
        p->m_pEventKey = p->m_pFocus;
        break;
    }
    case WM_KEYUP:  //弹起某键
    {
        if (p->m_pRoot == NULL) break;
        if (p->m_pEventKey == NULL) break;
        TEventUI event = { 0 };
        event.Type = ZEVENT_KEYUP;
        event.pSender = p->m_pEventKey;
        event.wParam = wParam;
        event.lParam = lParam;
        event.chKey = (TCHAR)wParam;
        event.ptMouse = p->m_ptLastMousePos;
        event.wKeyState = MapKeyState();
        event.dwTimestamp = GetTickCount64();
        ZuiControlEvent(p->m_pEventKey, &event);
        p->m_pEventKey = NULL;
        break;
    }
    case WM_SETCURSOR:
    {
        ZPoint pt = { 0 };
        if (p->m_pRoot == NULL) break;
        if (LOWORD(lParam) != HTCLIENT) break;
        if (p->m_bMouseCapture) return TRUE;

        GetCursorPos((LPPOINT)&pt);
        ScreenToClient(p->m_hWnd, (LPPOINT)&pt);
        ZuiControl pControl = NULL;
        if (p->m_pRoot)
            pControl = p->m_pRoot->call(Proc_FindControl, p->m_pRoot, p->m_pRoot->m_sUserData, &pt, (void *)(ZFIND_FROM_POINT | ZFIND_VISIBLE | ZFIND_HITTEST | ZFIND_TOP_FIRST));
        if (pControl == NULL) break;
        if (((int)ZuiControlCall(Proc_GetControlFlags, pControl, NULL, NULL) & ZFLAG_SETCURSOR) == 0) break;
        TEventUI event = { 0 };
        event.Type = ZEVENT_SETCURSOR;
        event.pSender = pControl;
        event.wParam = wParam;
        event.lParam = lParam;
        event.ptMouse = pt;
        event.wKeyState = MapKeyState();
        event.dwTimestamp = GetTickCount64();
        ZuiControlEvent(pControl, &event);
        return 0;
    }
    case WM_KILLFOCUS://失去焦点
    {
        TEventUI event = { 0 };
        event.Type = ZEVENT_KILLFOCUS;
        if (p->m_pFocus) {
            event.pSender = p->m_pFocus;
            event.dwTimestamp = GetTickCount64();
            ZuiControlEvent(p->m_pFocus, &event);
            p->m_pFocus = NULL;
        }
        event.pSender = p->m_pRoot;
        event.dwTimestamp = GetTickCount64();
        ZuiControlEvent(p->m_pRoot, &event);
        p->m_bMouseTracking = FALSE;
        p->m_pEventHover = NULL;
    }
    case WM_NOTIFY:
    {
        LPNMHDR lpNMHDR = (LPNMHDR)lParam;
        if (lpNMHDR != NULL)
            return SendMessage(lpNMHDR->hwndFrom, OCM__BASE + uMsg, wParam, lParam);
        return 0;
    }
    case WM_COMMAND:
    {
        if (lParam == 0) break;
        HWND hWndChild = (HWND)lParam;
        return SendMessage(hWndChild, OCM__BASE + uMsg, wParam, lParam);
    }
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORSTATIC:
    {
        // Refer To: http://msdn.microsoft.com/en-us/library/bb761691(v=vs.85).aspx
        // Read-only or disabled edit controls do not send the WM_CTLCOLOREDIT message; instead, they send the WM_CTLCOLORSTATIC message.
        if (lParam == 0) break;
        HWND hWndChild = (HWND)lParam;
        return SendMessage(hWndChild, OCM__BASE + uMsg, wParam, lParam);
    }
    case WM_IME_STARTCOMPOSITION: {
        if (p->m_pFocus == NULL) break;
        ZuiPoint pt = ZuiControlCall(Proc_GetImePoint, p->m_pFocus, 0, 0);
        COMPOSITIONFORM COMPOSITIONFORM;
        COMPOSITIONFORM.dwStyle = CFS_POINT | CFS_FORCE_POSITION;
        if (pt)
        {
            COMPOSITIONFORM.ptCurrentPos.x = pt->x + p->m_pFocus->m_rcItem.left;
            COMPOSITIONFORM.ptCurrentPos.y = pt->y + p->m_pFocus->m_rcItem.top;
        }

        ImmSetCompositionWindow(p->m_hIMC, &COMPOSITIONFORM);
    }
    case WM_NCHITTEST:
    {
        if (!p->m_nobox || IsZoomed(p->m_hWnd))
            break;
        int x = GET_X_LPARAM(lParam);
        int	y = GET_Y_LPARAM(lParam);
        if (x <= p->m_rect.left + 3 && y <= p->m_rect.top + 3) {
            return HTTOPLEFT;
        }
        else if (x <= p->m_rect.left + 3 && y >= p->m_rect.bottom - 3)
        {
            return HTBOTTOMLEFT;
        }
        else if (x >= p->m_rect.right - 3 && y <= p->m_rect.top + 3)
        {
            return HTTOPRIGHT;
        }
        else if (x >= p->m_rect.right - 3 && y >= p->m_rect.bottom - 3)
        {
            return HTBOTTOMRIGHT;
        }
        else if (x <= p->m_rect.left + 2)
        {
            return HTLEFT;
        }
        else if (y <= p->m_rect.top + 2)
        {
            return HTTOP;
        }
        else if (x >= p->m_rect.right - 2)
        {
            return HTRIGHT;
        }
        else if (y >= p->m_rect.bottom - 2)
        {
            return HTBOTTOM;
        }
        else
        {
            return HTCLIENT;
        }
        return HTCLIENT;
        break;
    }
    case WM_MOVE:
    {
        GetWindowRect(hWnd, (LPRECT)&p->m_rect);
    }
    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

ZuiBool ZuiOsInitialize() {
    // 关闭DPI缩放 ,UI库自己实现缩放
    FARPROC spdpia = GetProcAddress(GetModuleHandle(TEXT("user32")), "SetProcessDPIAware");
    if (spdpia != NULL)
        spdpia();
    // 获取DPI  
    HDC hdc = GetDC(NULL);
    if (hdc != NULL) {
        //g_dpix = GetDeviceCaps(hdc, LOGPIXELSX);
        //g_dpiy = GetDeviceCaps(hdc, LOGPIXELSY);
        ReleaseDC(NULL, hdc);
    }
    WNDCLASS wc = { 0 };
    wc.style = 8;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hIcon = NULL;
    wc.lpfnWndProc = __WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = _T("ZUI");
    RegisterClass(&wc);

    if (m_hUpdateRectPen == NULL) {
        m_hUpdateRectPen = CreatePen(PS_SOLID, 1, RGB(220, 0, 0));
        InitCommonControls();
        LoadLibrary(_T("msimg32.dll"));
}
    m_hMainThreadId = GetCurrentThreadId();
    return TRUE;
}
ZuiBool ZuiOsUnInitialize() {
    if (m_hUpdateRectPen) DeleteObject(m_hUpdateRectPen);
    return TRUE;
}

ZuiOsWindow ZuiOsCreateWindow(ZuiControl root, ZuiBool show, ZuiAny pcontrol) {
    /*保存相关参数到ZOsWindow*/
    ZuiOsWindow OsWindow = (ZuiOsWindow)malloc(sizeof(ZOsWindow));
    HWND tmphwnd = NULL;
    if (pcontrol)
        tmphwnd = ((ZuiControl)pcontrol)->m_pOs->m_hWnd;
    if (OsWindow)
    {
        memset(OsWindow, 0, sizeof(ZOsWindow));

        // Set the dialog root element
        OsWindow->m_pRoot = root;

        OsWindow->m_hWnd = CreateWindowEx(0, _T("ZUI"), _T(""),
            WS_OVERLAPPEDWINDOW | WS_VISIBLE ,
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
            tmphwnd, NULL, GetModuleHandle(NULL),
            OsWindow);

        OsWindow->m_hIMC = ImmGetContext(OsWindow->m_hWnd);//获取系统的输入法
                                             /*屏蔽输入法*/
        ImmAssociateContext(OsWindow->m_hWnd, NULL);

        OsWindow->m_hDcPaint = GetDC(OsWindow->m_hWnd);



        // Initiate all control
        root->m_pOs = OsWindow;
        ZuiControlCall(Proc_OnInit, root, 0,0);

        OsWindow->m_iTooltipWidth = -1;
        OsWindow->m_iHoverTime = 1000;
        //p->m_bShowUpdateRect = TRUE;
        OsWindow->m_uTimerID = 0x1000;
        OsWindow->m_bFirstLayout = TRUE;
        OsWindow->m_bOffscreenPaint = TRUE;
        OsWindow->m_nOpacity = 0xFF;

        OsWindow->m_ptLastMousePos.x = OsWindow->m_ptLastMousePos.y = -1;

        OsWindow->m_aTimers = darray_create();
        OsWindow->m_aPostPaintControls = darray_create();
        OsWindow->m_aFoundControls = darray_create();
        OsWindow->m_aDelayedCleanup = darray_create();
        //darray_append(m_aPreMessages, p);


        //是否立即显示
        if (!show)
            ShowWindow(OsWindow->m_hWnd, SW_SHOW);

        return OsWindow;
    }
    return NULL;
}
ZuiVoid ZuiOsDestroyWindow(ZuiOsWindow OsWindow) {
    if (OsWindow->m_hwndTooltip != NULL) //by jiangdong 修改当父窗体以成员变量形式在窗口类中存在时候,当点击父窗体关闭按钮的时候,提示框内容还停留在页面中，没有销毁。
    {
        DestroyWindow(OsWindow->m_hwndTooltip);
        OsWindow->m_hwndTooltip = NULL;
    }
    SetWindowLong(OsWindow->m_hWnd, GWLP_WNDPROC, DefWindowProc);
    if (OsWindow->m_hIMC) ImmReleaseContext(OsWindow->m_hWnd, OsWindow->m_hIMC);
    if (OsWindow->m_hDcPaint) ReleaseDC(OsWindow->m_hWnd, OsWindow->m_hDcPaint);
    DestroyWindow(OsWindow->m_hWnd);
    if (OsWindow->m_hDcOffscreen) ZuiDestroyGraphics(OsWindow->m_hDcOffscreen);
    if (OsWindow->m_aTimers) darray_destroy(OsWindow->m_aTimers);
    if (OsWindow->m_aPostPaintControls)  darray_destroy(OsWindow->m_aPostPaintControls);
    if (OsWindow->m_aFoundControls)  darray_destroy(OsWindow->m_aFoundControls);
    if (OsWindow->m_aDelayedCleanup)  darray_destroy(OsWindow->m_aDelayedCleanup);
    free(OsWindow);
}
ZuiBool ZuiOsSetWindowTitle(ZuiOsWindow OsWindow, ZuiText Title) {
    return SetWindowText(OsWindow->m_hWnd, Title);
}
ZuiBool ZuiOsSetWindowMin(ZuiOsWindow OsWindow) {
    return ShowWindow(OsWindow->m_hWnd, SW_MINIMIZE);
}
ZuiBool ZuiOsSetWindowMax(ZuiOsWindow OsWindow) {
    OsWindow->m_bMax = TRUE;
    return ShowWindow(OsWindow->m_hWnd, SW_MAXIMIZE);
}
ZuiBool ZuiOsSetWindowRestor(ZuiOsWindow OsWindow) {
    OsWindow->m_bMax = FALSE;
    return ShowWindow(OsWindow->m_hWnd, SW_RESTORE);
}
ZuiBool ZuiOsSetWindowSize(ZuiOsWindow OsWindow, int w, int h) {
    ZuiControlCall(Proc_SetFixedWidth, OsWindow->m_pRoot, (ZuiAny)w, NULL);
    ZuiControlCall(Proc_SetFixedHeight, OsWindow->m_pRoot, (ZuiAny)h, NULL);
    return SetWindowPos(OsWindow->m_hWnd, NULL, 0, 0, w, h, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
}
ZuiBool ZuiOsSetWindowNoBox(ZuiOsWindow OsWindow, ZuiBool b) {
    if (OsWindow->m_nobox == b)
        return FALSE;
    OsWindow->m_nobox = b;
    DWORD dwStyle = GetWindowLong(OsWindow->m_hWnd, GWL_STYLE);
    if (b)
    {
        SetWindowLong(OsWindow->m_hWnd, GWL_STYLE, dwStyle | WS_VISIBLE | WS_POPUP | WS_CLIPCHILDREN);
    }
    else {
        SetWindowLong(OsWindow->m_hWnd, GWL_STYLE, dwStyle | WS_VISIBLE | WS_POPUP | WS_CLIPCHILDREN);
    }
    GetWindowRect(OsWindow->m_hWnd, (LPRECT)&OsWindow->m_rect);
    return TRUE;
}
ZuiBool ZuiOsSetWindowComBo(ZuiOsWindow OsWindow, ZuiBool b) {
    if (OsWindow->m_combo == b)
        return FALSE;
    OsWindow->m_combo = b;
    DWORD dwStyle = GetWindowLong(OsWindow->m_hWnd, GWL_EXSTYLE);
    if (b) {
        dwStyle |= WS_EX_TOOLWINDOW;
    }
    else {
        dwStyle &= ~WS_EX_TOOLWINDOW;
    }
    SetWindowLong(OsWindow->m_hWnd, GWL_EXSTYLE, dwStyle);
    return TRUE;
}
ZuiBool ZuiOsSetWindowTool(ZuiOsWindow OsWindow, ZuiBool b) {
    DWORD dwStyle = GetWindowLong(OsWindow->m_hWnd, GWL_EXSTYLE);
    if (b) {
        dwStyle |= WS_EX_TOOLWINDOW;
    }
    else {
        dwStyle &= ~WS_EX_TOOLWINDOW;
    }
    SetWindowLong(OsWindow->m_hWnd, GWL_EXSTYLE, dwStyle);
    return TRUE;
}
ZuiVoid ZuiOsSetWindowVisible(ZuiOsWindow OsWindow, ZuiBool Visible) {
    if (Visible)
        ShowWindow(OsWindow->m_hWnd, SW_SHOWNORMAL);
    else
        ShowWindow(OsWindow->m_hWnd, SW_HIDE);
}
ZuiVoid ZuiOsWindowPopup(ZuiOsWindow OsWindow, ZuiPoint pt) {
    if (pt)
    {
        SetWindowPos(OsWindow->m_hWnd, NULL, pt->x, pt->y, OsWindow->m_rect.right - OsWindow->m_rect.left, OsWindow->m_rect.bottom - OsWindow->m_rect.top, SWP_NOZORDER | SWP_NOACTIVATE);
    }
    ShowWindow(OsWindow->m_hWnd, SW_SHOWNORMAL);
    SetFocus(OsWindow->m_hWnd);
}
ZuiVoid ZuiOsSetWindowCenter(ZuiOsWindow OsWindow) {
    ZRect rc, rc1, rctomove;
    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);
    rc.left = 0;
    rc.top = 0;
    rc.right = width;
    rc.bottom = height;
    GetClientRect(OsWindow->m_hWnd, (LPRECT)&rc1);
    rctomove.left = (rc.right - rc.left) / 2 - (rc1.right - rc1.left) / 2;
    rctomove.right = (rc.right - rc.left) / 2 + (rc1.right - rc1.left) / 2;
    rctomove.top = (rc.bottom - rc.top) / 2 - (rc1.bottom - rc1.top) / 2;
    rctomove.bottom = (rc.bottom - rc.top) / 2 + (rc1.bottom - rc1.top) / 2;
    SetWindowPos(OsWindow->m_hWnd, HWND_TOPMOST, rctomove.left, rctomove.top, rc1.right - rc1.left, rc1.bottom - rc1.top, SWP_NOZORDER | SWP_NOACTIVATE);
}

ZuiVoid ZuiOsSetCursor(int type) {
    SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(type)));
}
ZuiVoid ZuiOsSetCapture(ZuiOsWindow OsWindow) {
    SetCapture(OsWindow->m_hWnd);
    OsWindow->m_bMouseCapture = TRUE;
}
ZuiVoid ZuiOsReleaseCapture(ZuiOsWindow OsWindow) {
    ReleaseCapture();
    OsWindow->m_bMouseCapture = FALSE;
}

//指定区域失效
ZuiVoid ZuiOsInvalidate(ZuiOsWindow p) {
    if (!p->m_bLayered)
        InvalidateRect(p->m_hWnd, NULL, FALSE);
    else {
        ZRect rcClient = { 0 };
        GetClientRect(p->m_hWnd, (LPRECT)&rcClient);
        UnionRect((LPRECT)&p->m_rcLayeredUpdate, (const RECT *)&p->m_rcLayeredUpdate, (const RECT *)&rcClient);
    }
}
//指定区域失效
ZuiVoid ZuiOsInvalidateRect(ZuiOsWindow p, ZRect *rcItem)
{
    ZRect rc;
    rc.bottom = rcItem->bottom;
    rc.left = rcItem->left;
    rc.right = rcItem->right;
    rc.top = rcItem->top;
    if (rc.left < 0) rc.left = 0;
    if (rc.top < 0) rc.top = 0;
    if (rc.right < rc.left) rc.right = rc.left;
    if (rc.bottom < rc.top) rc.bottom = rc.top;
    if (!p->m_bLayered)
        InvalidateRect(p->m_hWnd, (const RECT *)&rc, FALSE);
    else
        UnionRect((LPRECT)&p->m_rcLayeredUpdate, (const RECT *)&p->m_rcLayeredUpdate, (const RECT *)&rc);
}
//创建时钟
ZuiBool ZuiOsSetTimer(ZuiControl pControl, int nTimerID, int uElapse) {
    TIMERINFO* pTimer;
    ASSERT(pControl != NULL);
    ASSERT(uElapse > 0);
    if (pControl->m_pOs) {
        for (int i = 0; i < darray_len(pControl->m_pOs->m_aTimers); i++) {
            pTimer = (TIMERINFO*)(pControl->m_pOs->m_aTimers->data[i]);
            if (pTimer->pSender == pControl
                && pTimer->hWnd == pControl->m_pOs->m_hWnd
                && pTimer->nLocalID == nTimerID) {
                if (pTimer->bKilled == TRUE) {
                    if (SetTimer(pControl->m_pOs->m_hWnd, pTimer->uWinTimer, uElapse, NULL)) {
                        pTimer->bKilled = FALSE;
                        return TRUE;
                    }
                    return FALSE;
                }
                return FALSE;
            }
        }

        pControl->m_pOs->m_uTimerID = (++pControl->m_pOs->m_uTimerID) % 0xFF;
        if (!SetTimer(pControl->m_pOs->m_hWnd, pControl->m_pOs->m_uTimerID, uElapse, NULL)) return FALSE;
        pTimer = malloc(sizeof(TIMERINFO));
        memset(pTimer, 0, sizeof(TIMERINFO));
        if (pTimer == NULL) return FALSE;
        pTimer->hWnd = pControl->m_pOs->m_hWnd;
        pTimer->pSender = pControl;
        pTimer->nLocalID = nTimerID;
        pTimer->uWinTimer = pControl->m_pOs->m_uTimerID;
        pTimer->bKilled = FALSE;
        return darray_append(pControl->m_pOs->m_aTimers, pTimer);
    }
    return FALSE;
}
//销毁时钟
ZuiBool ZuiOsKillTimer_Id(ZuiControl pControl, int nTimerID) {
    ASSERT(pControl != NULL);
    if (pControl->m_pOs) {
        for (int i = 0; i < darray_len(pControl->m_pOs->m_aTimers); i++) {
            TIMERINFO* pTimer = (TIMERINFO*)(pControl->m_pOs->m_aTimers->data[i]);
            if (pTimer->pSender == pControl
                && pTimer->hWnd == pControl->m_pOs->m_hWnd
                && pTimer->nLocalID == nTimerID)
            {
                if (pTimer->bKilled == FALSE) {
                    if (IsWindow(pControl->m_pOs->m_hWnd))
                        KillTimer(pTimer->hWnd, pTimer->uWinTimer);
                    pTimer->bKilled = TRUE;
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}
//销毁时钟
ZuiVoid ZuiOsKillTimer(ZuiControl pControl) {
    ASSERT(pControl != NULL);
    if (pControl->m_pOs) {
        int count = darray_len(pControl->m_pOs->m_aTimers);
        for (int i = 0, j = 0; i < count; i++) {
            TIMERINFO* pTimer = (TIMERINFO*)(pControl->m_pOs->m_aTimers->data[i - j]);
            if (pTimer->pSender == pControl && pTimer->hWnd == pControl->m_pOs->m_hWnd) {
                if (pTimer->bKilled == FALSE)
                    KillTimer(pTimer->hWnd, pTimer->uWinTimer);
                free(pTimer);
                darray_delete(pControl->m_pOs->m_aTimers, i - j);
                j++;
            }
        }
    }
}
//销毁全部时钟
ZuiVoid ZuiOsRemoveAllTimers(ZuiOsWindow p) {
    for (int i = 0; i < darray_len(p->m_aTimers); i++) {
        TIMERINFO* pTimer = (TIMERINFO*)(p->m_aTimers->data[i]);
        if (pTimer->hWnd == p->m_hWnd) {
            if (pTimer->bKilled == FALSE) {
                if (IsWindow(p->m_hWnd)) KillTimer(p->m_hWnd, pTimer->uWinTimer);
            }
            free(pTimer);
        }
    }

    darray_empty(p->m_aTimers);
}

//设置焦点控件
ZuiVoid ZuiOsSetFocus(ZuiOsWindow p, ZuiControl pControl, ZuiBool bFocusWnd)
{
    // Paint manager window has focus?
    HWND hFocusWnd = GetFocus();
    if (bFocusWnd && hFocusWnd != p->m_hWnd && pControl != p->m_pFocus) {
        //无焦点窗口不做任何处理
        if (!p->m_bUnfocusPaintWindow)
        {
            SetFocus(p->m_hWnd);
        }
    }
    // Already has focus?
    if (pControl == p->m_pFocus) return;
    // Remove focus from old control
    if (p->m_pFocus != NULL)
    {
        TEventUI event = { 0 };
        event.Type = ZEVENT_KILLFOCUS;
        event.pSender = pControl;
        event.dwTimestamp = GetTickCount64();
        ZuiControlEvent(p->m_pFocus, &event);
        p->m_pFocus = NULL;
    }
    if (pControl == NULL) return;
    // Set focus to new control
    if (pControl != NULL
        && pControl->m_pOs == p
        && pControl->m_bVisible
        && pControl->m_bEnabled)
    {
        p->m_pFocus = pControl;
        TEventUI event = { 0 };
        event.Type = ZEVENT_SETFOCUS;
        event.pSender = pControl;
        event.dwTimestamp = GetTickCount64();
        ZuiControlEvent(p->m_pFocus, &event);
    }
}

ZuiVoid ZuiOsReapObjects(ZuiOsWindow p, ZuiControl pControl) {
    if (pControl == NULL) return;
    if (pControl == p->m_pEventKey) p->m_pEventKey = NULL;
    if (pControl == p->m_pEventHover) p->m_pEventHover = NULL;
    if (pControl == p->m_pEventClick) p->m_pEventClick = NULL;
    if (pControl == p->m_pFocus) p->m_pFocus = NULL;
    ZuiOsKillTimer(pControl);
}

ZuiVoid ZuiOsAddDelayedCleanup(ZuiControl pControl, ZuiAny Param1, ZuiAny Param2)
{
    ZuiControlCall(Proc_Layout_Remove, pControl->m_pParent, pControl, (ZuiAny)TRUE);
    //ZuiControlCall(Proc_SetOs, pControl, pControl->m_pOs, NULL, (void*)FALSE);
    darray_append(pControl->m_pOs->m_aDelayedCleanup, pControl);
    PostMessage(pControl->m_pOs->m_hWnd, WM_APP + 1, (WPARAM)Param1, (LPARAM)Param2);
}

int ZuiOsMsgLoop() {
    MSG Msg;
    while (GetMessage(&Msg, NULL, 0, 0)) {
        if (WM_QUIT == Msg.message) {
            break;
        }
        TranslateMessage(&Msg);
        DispatchMessageW(&Msg);
    }
    return Msg.wParam;
}
ZuiVoid ZuiOsMsgLoopExit(int nRet) {
    PostQuitMessage(nRet);
}
ZuiVoid ZuiOsPostMessage(ZuiControl cp, ZuiAny Msg, ZuiAny Param1, ZuiAny Param2) {
    PostMessage(cp->m_pOs->m_hWnd, (UINT)Msg, (WPARAM)Param1, (LPARAM)Param2);
}

ZEXPORT int ZuiDoModel(ZuiControl cp)
{
    int nRet;
    HWND chwnd = cp->m_pOs->m_hWnd;
    HWND phwnd = GetWindowOwner((HWND)chwnd);
    SetWindowPos((HWND)chwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
    //禁用掉父窗口
    EnableWindow((HWND)phwnd, FALSE);
    MSG Msg;
    while (GetMessage(&Msg, NULL, 0, 0))
    {
        if (Msg.message == WM_APP + 1)
        {
            EnableWindow((HWND)phwnd, TRUE);
            SetFocus((HWND)phwnd);
        }
        //if (Msg.hwnd == (HWND)chwnd || Msg.message == WM_PAINT || Msg.message == WM_TIMER) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
        //}
        if (Msg.message == WM_QUIT) {
            break;
        }
    }
    //重新开启父窗口
    nRet = Msg.wParam;
    EnableWindow((HWND)phwnd, TRUE);
    SetFocus((HWND)phwnd);
    return nRet;
}
int ZuiOsUtf8ToUnicode(ZuiAny str, int slen, ZuiText out, int olen)
{
    return MultiByteToWideChar(CP_UTF8, 0, str, slen, out, olen);
}

int ZuiOsAsciiToUnicode(ZuiAny str, int slen, ZuiText out, int olen)
{
    return MultiByteToWideChar(CP_ACP, 0, str, slen, out, olen);
}

int ZuiOsUnicodeToAscii(ZuiText str, int slen, ZuiAny out, int olen)
{
    return WideCharToMultiByte(CP_ACP, 0, str, slen, out, olen, NULL, NULL);
}

int ZuiOsUnicodeToUtf8(ZuiText str, int slen, ZuiAny out, int olen)
{
    return WideCharToMultiByte(CP_UTF8, 0, str, slen, out, olen, NULL, NULL);
}

ZuiVoid ZuiOsClientToScreen(ZuiControl p, ZuiPoint pt) {
    if (p && pt) {
        ClientToScreen(p->m_pOs->m_hWnd, (LPPOINT)pt);
    }
}
ZuiVoid ZuiOsScreenToClient(ZuiControl p, ZuiPoint pt) {
    if (p && pt) {
        ScreenToClient(p->m_pOs->m_hWnd, (LPPOINT)pt);
    }
}

#endif
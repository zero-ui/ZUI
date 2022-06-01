#include "Button.h"
#include "Label.h"
#include <core/control.h>
#include <core/resdb.h>
#include <core/builder.h>
#include <platform/platform.h>
#include <stdlib.h>
ZEXPORT ZuiAny ZCALL ZuiButtonProc(int ProcId, ZuiControl cp, ZuiButton p, ZuiAny Param1, ZuiAny Param2) {
    switch (ProcId)
    {
    case Proc_OnEvent: {
        TEventUI *event = (TEventUI *)Param1;
        switch (event->Type)
        {
		case ZEVENT_KILLFOCUS:
		{
			p->type = 0;
			break;
		}
        case ZEVENT_MOUSELEAVE: {
            p->type = 0;
            ZuiControlInvalidate(cp, TRUE);
        }
                                break;
        case ZEVENT_MOUSEENTER: {
            p->type = 1;
            ZuiControlInvalidate(cp, TRUE);
        }
                                break;
        case ZEVENT_LBUTTONDOWN: {
            p->type = 2;
            ZuiControlInvalidate(cp, TRUE);
        }
                                 break;
        case ZEVENT_LBUTTONUP: {
            p->type = 1;
            ZuiControlInvalidate(cp, TRUE);
        }
                               break;
        default:
            break;
        }
        break;
    }
    case Proc_OnPaint: {
        //调整绘制顺序
        ZuiControlCall(Proc_OnPaintBkColor, cp, Param1, Param2);
        ZuiControlCall(Proc_OnPaintBkImage, cp, Param1, Param2);
        ZuiControlCall(Proc_OnPaintStatusImage, cp, Param1, Param2);
        ZuiControlCall(Proc_OnPaintText, cp, Param1, Param2);
        ZuiControlCall(Proc_OnPaintBorder, cp, Param1, Param2);
        return 0;
    }
    case Proc_OnPaintBorder: {
        ZuiGraphics gp = (ZuiGraphics)Param1;
        ZRect* rc = (ZRect*)&cp->m_rcItem;
        if (!p->m_ResNormal && p->type > 0) {
            ZuiDrawRoundRect(gp, p->m_BorderColor, rc, cp->m_rRound.cx, cp->m_rRound.cy,cp->m_dwBorderWidth);
            return 0;
        }
        break;
    }
    case Proc_OnPaintStatusImage: {
        ZuiGraphics gp = (ZuiGraphics)Param1;
        ZRect *rc = (ZRect *)&cp->m_rcItem;
        ZuiImage img;
        if (p->type == 0) {
            if (p->m_ResNormal) {
                img = p->m_ResNormal->p;
                ZuiDrawImageEx(gp, img, rc->left, rc->top, rc->right, rc->bottom, 0, 0, 0, 0, 255);
            }
            else {
                ZuiDrawFillRoundRect(gp, p->m_ColorNormal, rc,cp->m_rRound.cx,cp->m_rRound.cy);
            }
        }
        else if (p->type == 1) {
            if (p->m_ResHot) {
                img = p->m_ResHot->p;
                ZuiDrawImageEx(gp, img, rc->left, rc->top, rc->right, rc->bottom, 0, 0, 0, 0, 255);
            }
            else {
                ZuiDrawFillRoundRect(gp, p->m_ColorHot, rc, cp->m_rRound.cx, cp->m_rRound.cy);
            }
        }
        else if (p->type == 2) {
            if (p->m_ResPushed) {
                img = p->m_ResPushed->p;
                ZuiDrawImageEx(gp, img, rc->left, rc->top, rc->right, rc->bottom, 0, 0, 0, 0, 255);
            }
            else {
                ZuiDrawFillRoundRect(gp, p->m_ColorPushed, rc, cp->m_rRound.cx, cp->m_rRound.cy);
            }
        }
        else {
            if (p->m_ResDisabled) {
                img = p->m_ResDisabled->p;
                ZuiDrawImageEx(gp, img, rc->left, rc->top, rc->right, rc->bottom, 0, 0, 0, 0, 255);
            }
            else {
                ZuiDrawFillRoundRect(gp, p->m_ColorDisabled, rc, cp->m_rRound.cx, cp->m_rRound.cy);
            }
        }
        return 0;
    }
    case Proc_SetEnabled: {
        if (0 == (ZuiBool)Param1)
            p->type = -1;
        else
            p->type = 0;
        break;
    }
    case Proc_Button_SetResNormal: {
        if (p->m_ResNormal)
            ZuiResDBDelRes(p->m_ResNormal);
        p->m_ResNormal = Param1;
        return 0;
    }
    case Proc_Button_SetResHot: {
        if (p->m_ResHot)
            ZuiResDBDelRes(p->m_ResHot);
        p->m_ResHot = Param1;
        return 0;
    }
    case Proc_Button_SetResPushed: {
        if (p->m_ResPushed)
            ZuiResDBDelRes(p->m_ResPushed);
        p->m_ResPushed = Param1;
        return 0;
    }
    case Proc_Button_SetResFocused: {
        if (p->m_ResFocused)
            ZuiResDBDelRes(p->m_ResFocused);
        p->m_ResFocused = Param1;
        return 0;
    }
    case Proc_Button_SetResDisabled: {
        if (p->m_ResDisabled)
            ZuiResDBDelRes(p->m_ResDisabled);
        p->m_ResDisabled = Param1;
        return 0;
    }
    case Proc_Button_SetColorNormal: {
        p->m_ColorNormal = (ZuiColor)Param1;
        return 0;
    }
    case Proc_Button_SetColorHot: {
        p->m_ColorHot = (ZuiColor)Param1;
        return 0;
    }
    case Proc_Button_SetColorPushed: {
        p->m_ColorPushed = (ZuiColor)Param1;
        return 0;
    }
    case Proc_Button_SetColorFocused: {
        p->m_ColorFocused = (ZuiColor)Param1;
        return 0;
    }
    case Proc_Button_SetColorDisabled: {
        p->m_ColorDisabled = (ZuiColor)Param1;
        return 0;
    }
    case Proc_SetAnimation: {
        if (cp->m_aAnime)
            ZuiAnimationFree(cp->m_aAnime);
        cp->m_aAnime = ZuiAnimationNew(Param1, Param2);
        return 0;
    }
    case Proc_SetAttribute: {
        if (_tcsicmp(Param1, _T("normalimage")) == 0) ZuiControlCall(Proc_Button_SetResNormal, cp, ZuiResDBGetRes(Param2, ZREST_IMG), NULL);
        else if (_tcsicmp(Param1, _T("hotimage")) == 0) ZuiControlCall(Proc_Button_SetResHot, cp, ZuiResDBGetRes(Param2, ZREST_IMG), NULL);
        else if (_tcsicmp(Param1, _T("pushedimage")) == 0) ZuiControlCall(Proc_Button_SetResPushed, cp, ZuiResDBGetRes(Param2, ZREST_IMG), NULL);
        else if (_tcsicmp(Param1, _T("focusedimage")) == 0) ZuiControlCall(Proc_Button_SetResFocused, cp, ZuiResDBGetRes(Param2, ZREST_IMG), NULL);
        else if (_tcsicmp(Param1, _T("disabledimage")) == 0) ZuiControlCall(Proc_Button_SetResDisabled, cp, ZuiResDBGetRes(Param2, ZREST_IMG), NULL);
        
        else if (_tcsicmp(Param1, _T("normalcolor")) == 0) ZuiControlCall(Proc_Button_SetColorNormal, cp, (ZuiAny)ZuiStr2Color(Param2), NULL);
        else if (_tcsicmp(Param1, _T("hotcolor")) == 0) ZuiControlCall(Proc_Button_SetColorHot, cp, (ZuiAny)ZuiStr2Color(Param2), NULL);
        else if (_tcsicmp(Param1, _T("pushedcolor")) == 0) ZuiControlCall(Proc_Button_SetColorPushed, cp, (ZuiAny)ZuiStr2Color(Param2), NULL);
        else if (_tcsicmp(Param1, _T("focusedcolor")) == 0) ZuiControlCall(Proc_Button_SetColorFocused, cp, (ZuiAny)ZuiStr2Color(Param2), NULL);
        else if (_tcsicmp(Param1, _T("disabledcolor")) == 0) ZuiControlCall(Proc_Button_SetColorDisabled, cp, (ZuiAny)ZuiStr2Color(Param2), NULL);
        break;
    }
    case Proc_OnCreate: {
        p = (ZuiButton)malloc(sizeof(ZButton));
        memset(p, 0, sizeof(ZButton));

        //保存原来的回调地址,创建成功后回调地址指向当前函数
        p->old_udata = ZuiLabelProc(Proc_OnCreate, cp, 0, 0, 0);
        p->old_call = (ZCtlProc)&ZuiLabelProc;

        p->m_ColorNormal = 0xFF383838;
        p->m_ColorHot = 0xFF585858;
        p->m_ColorPushed = 0xFF787878;
        p->m_ColorDisabled = 0xFF989898;
        p->m_BorderColor = 0xFF1874CD;

        ((ZuiLabel)p->old_udata)->m_uTextStyle |= ZDT_CENTER;

        return p;
    }
    case Proc_OnDestroy: {
        ZCtlProc old_call = p->old_call;
        ZuiAny old_udata = p->old_udata;
        if (p->m_ResNormal)
            ZuiResDBDelRes(p->m_ResNormal);
        if (p->m_ResHot)
            ZuiResDBDelRes(p->m_ResHot);
        if (p->m_ResPushed)
            ZuiResDBDelRes(p->m_ResPushed);
        if (p->m_ResFocused)
            ZuiResDBDelRes(p->m_ResFocused);
        if (p->m_ResDisabled)
            ZuiResDBDelRes(p->m_ResDisabled);

        old_call(ProcId, cp, old_udata, Param1, Param2);

        free(p);

        return 0;
    }
    case Proc_GetObject:
        if (_tcsicmp(Param1, (ZuiAny)Type_Button) == 0)
            return (ZuiAny)p;
        break;
    case Proc_GetType:
        return (ZuiAny)Type_Button;
    case Proc_CoreInit:
        return (ZuiAny)TRUE;
    case Proc_CoreUnInit:
        return (ZuiAny)TRUE;
    default:
        break;
    }
    return p->old_call(ProcId, cp, p->old_udata, Param1, Param2);
}





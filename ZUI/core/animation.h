#ifndef __ZUI_CORE_ANIMATION_H__
#define __ZUI_CORE_ANIMATION_H__
#include <ZUI.h>
#include <platform/platform.h>
typedef ZuiVoid(ZCALL *ZAnimeProcOnPaint)(ZuiControl p, ZuiAny gp, ZuiAny rc);
typedef ZuiVoid(ZCALL *ZAnimeProcOnSize)(ZuiControl p, ZuiAny w, ZuiAny h);
typedef ZuiVoid(ZCALL *ZAnimeProcOnEvent)(ZuiControl p, TEventUI *event);
typedef struct _ZAnimation
{
    int              steup;
    ZuiGraphics         m_hDcOffscreen;		///内存缓冲区绘图DC
    ZAnimeProcOnPaint   OnPaint;    //动画的绘制hook 
    ZAnimeProcOnEvent   OnEvent;    //动画的事件hook
    ZAnimeProcOnSize    OnSize;     //大小被改变
}*ZuiAnimation, ZAnimation;

ZuiAnimation ZuiAnimationNew(ZuiAny Param1, ZuiAny Param2);
ZuiVoid ZuiAnimationFree(ZuiAnimation p);
#endif	//__ZUI_CORE_ANIMATION_H__

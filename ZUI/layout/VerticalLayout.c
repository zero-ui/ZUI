#include "VerticalLayout.h"
#include "Layout.h"
#include <core/control.h>
#include <stdlib.h>

void* ZCALL ZuiVerticalLayoutProc(int ProcId, ZuiControl cp, ZuiVerticalLayout p, ZuiAny Param1, ZuiAny Param2) {
    switch (ProcId)
    {
    case Proc_SetPos: {
        ZuiDefaultControlProc(ProcId, cp, 0, Param1, Param2);
        ZRect rc = cp->m_rcItem;
        ZuiLayout op = (ZuiLayout)p->old_udata;

        // Adjust for inset
		rc.left += op->m_rcInset.left;
		rc.top += op->m_rcInset.top;
		rc.right -= op->m_rcInset.right;
		rc.bottom -= op->m_rcInset.bottom;
		rc.left += cp->m_dwBorderWidth;
		rc.top += cp->m_dwBorderWidth;
		rc.right -= cp->m_dwBorderWidth;
		rc.bottom -= cp->m_dwBorderWidth;

        if (op->m_pVerticalScrollBar && op->m_pVerticalScrollBar->m_bVisible) { rc.right -= (int)ZuiControlCall(Proc_GetFixedWidth, op->m_pVerticalScrollBar, NULL, NULL); }
        if (op->m_pHorizontalScrollBar && op->m_pHorizontalScrollBar->m_bVisible) { rc.bottom -= (int)ZuiControlCall(Proc_GetFixedHeight, op->m_pHorizontalScrollBar, NULL, NULL); }

        if (darray_len(op->m_items) == 0) {
            ZuiControlCall(Proc_Layout_ProcessScrollBar, cp, &rc, 0);
            return 0;
        }

        // Determine the minimum size
        ZSize szAvailable = { rc.right - rc.left, rc.bottom - rc.top };
        if (op->m_pHorizontalScrollBar && op->m_pHorizontalScrollBar->m_bVisible)
            szAvailable.cx += (int)ZuiControlCall(Proc_ScrollBar_GetScrollRange, op->m_pHorizontalScrollBar, NULL, NULL);
        if (op->m_pVerticalScrollBar && op->m_pVerticalScrollBar->m_bVisible)
            szAvailable.cy += (int)ZuiControlCall(Proc_ScrollBar_GetScrollRange, op->m_pVerticalScrollBar, NULL, NULL);

        int cxNeeded = 0;
        int nAdjustables = 0;
        int cyFixed = 0;
        int nEstimateNum = 0;
        ZSize szControlAvailable;
        int iControlMaxWidth = 0;
        int iControlMaxHeight = 0;
        for (int it1 = 0; it1 < darray_len(op->m_items); it1++) {
            ZuiControl pControl = (ZuiControl)(op->m_items->data[it1]);
            if (!pControl->m_bVisible) continue;
            if (pControl->m_bFloat) continue;
            szControlAvailable = szAvailable;
            ZRect rcPadding = *(ZRect *)(ZuiControlCall(Proc_GetPadding, pControl, 0, 0));
            szControlAvailable.cx -= rcPadding.left + rcPadding.right;
            iControlMaxWidth = (int)ZuiControlCall(Proc_GetFixedWidth, pControl, 0, 0);
            iControlMaxHeight = (int)ZuiControlCall(Proc_GetFixedHeight, pControl, 0, 0);
            if (iControlMaxWidth <= 0) iControlMaxWidth = (int)ZuiControlCall(Proc_GetMaxWidth, pControl, 0, 0);
            if (iControlMaxHeight <= 0) iControlMaxHeight = (int)ZuiControlCall(Proc_GetMaxHeight, pControl, 0, 0);
            if (szControlAvailable.cx > iControlMaxWidth) szControlAvailable.cx = iControlMaxWidth;
            if (szControlAvailable.cy > iControlMaxHeight) szControlAvailable.cy = iControlMaxHeight;
            ZSize sz = *(ZSize *)ZuiControlCall(Proc_EstimateSize, pControl, (void *)&szControlAvailable, 0);
            if (sz.cy == 0) {
                nAdjustables++;
            }
            else {
                if (sz.cy < (int)ZuiControlCall(Proc_GetMinHeight, pControl, 0, 0)) sz.cy = (int)ZuiControlCall(Proc_GetMinHeight, pControl, 0, 0);
                if (sz.cy > (int)ZuiControlCall(Proc_GetMaxHeight, pControl, 0, 0)) sz.cy = (int)ZuiControlCall(Proc_GetMaxHeight, pControl, 0, 0);
            }
            cyFixed += sz.cy + ((ZRect *)(ZuiControlCall(Proc_GetPadding, pControl, 0, 0)))->top + ((ZRect *)(ZuiControlCall(Proc_GetPadding, pControl, 0, 0)))->bottom;

            sz.cx = MAX(sz.cx, 0);
            if (sz.cx < (int)ZuiControlCall(Proc_GetMinWidth, pControl, 0, 0)) sz.cx = (int)ZuiControlCall(Proc_GetMinWidth, pControl, 0, 0);
            if (sz.cx > (int)ZuiControlCall(Proc_GetMaxWidth, pControl, 0, 0)) sz.cx = (int)ZuiControlCall(Proc_GetMaxWidth, pControl, 0, 0);
            cxNeeded = MAX(cxNeeded, sz.cx + rcPadding.left + rcPadding.right);
            nEstimateNum++;
        }
        cyFixed += (nEstimateNum - 1) * op->m_iChildPadding;

        // Place elements
        int cyNeeded = 0;
        int cyExpand = 0;
        if (nAdjustables > 0) cyExpand = MAX(0, (szAvailable.cy - cyFixed) / nAdjustables);
        // Position the elements
        ZSize szRemaining = szAvailable;
        int iPosY = rc.top;
        if (op->m_pVerticalScrollBar && op->m_pVerticalScrollBar->m_bVisible) {
            iPosY -= (int)ZuiControlCall(Proc_ScrollBar_GetScrollPos, op->m_pVerticalScrollBar, NULL, NULL);
        }

        int iEstimate = 0;
        int iAdjustable = 0;
        int cyFixedRemaining = cyFixed;
        for (int it2 = 0; it2 < darray_len(op->m_items); it2++) {
            ZuiControl pControl = (ZuiControl)(op->m_items->data[it2]);
            if (!pControl->m_bVisible) continue;
            if (pControl->m_bFloat) {
                ZuiControlCall(Proc_Layout_SetFloatPos, cp, (ZuiAny)it2, 0);
                continue;
            }

            iEstimate += 1;
            ZRect rcPadding = *(ZRect *)(ZuiControlCall(Proc_GetPadding, pControl, 0, 0));
            szRemaining.cy -= rcPadding.top;

            szControlAvailable = szRemaining;
            szControlAvailable.cx -= rcPadding.left + rcPadding.right;
            iControlMaxWidth = (int)ZuiControlCall(Proc_GetFixedWidth, pControl, 0, 0);
            iControlMaxHeight = (int)ZuiControlCall(Proc_GetFixedHeight, pControl, 0, 0);
            if (iControlMaxWidth <= 0) iControlMaxWidth = (int)ZuiControlCall(Proc_GetMaxWidth, pControl, 0, 0);
            if (iControlMaxHeight <= 0) iControlMaxHeight = (int)ZuiControlCall(Proc_GetMaxHeight, pControl, 0, 0);
            if (szControlAvailable.cx > iControlMaxWidth) szControlAvailable.cx = iControlMaxWidth;
            if (szControlAvailable.cy > iControlMaxHeight) szControlAvailable.cy = iControlMaxHeight;
            cyFixedRemaining = cyFixedRemaining - (rcPadding.top + rcPadding.bottom);
            if (iEstimate > 1) cyFixedRemaining = cyFixedRemaining - op->m_iChildPadding;
            ZSize sz = *(ZSize *)ZuiControlCall(Proc_EstimateSize, pControl, (void *)&szControlAvailable, 0);
            if (sz.cy == 0) {
                iAdjustable++;
                sz.cy = cyExpand;
                // Distribute remaining to last element (usually round-off left-overs)
                if (iAdjustable == nAdjustables) {
                    sz.cy = MAX(0, szRemaining.cy - rcPadding.bottom - cyFixedRemaining);
                }
                if (sz.cy < (int)ZuiControlCall(Proc_GetMinHeight, pControl, 0, 0)) sz.cy = (int)ZuiControlCall(Proc_GetMinHeight, pControl, 0, 0);
                if (sz.cy > (int)ZuiControlCall(Proc_GetMaxHeight, pControl, 0, 0)) sz.cy = (int)ZuiControlCall(Proc_GetMaxHeight, pControl, 0, 0);
            }
            else {
                if (sz.cy < (int)ZuiControlCall(Proc_GetMinHeight, pControl, 0, 0)) sz.cy = (int)ZuiControlCall(Proc_GetMinHeight, pControl, 0, 0);
                if (sz.cy > (int)ZuiControlCall(Proc_GetMaxHeight, pControl, 0, 0)) sz.cy = (int)ZuiControlCall(Proc_GetMaxHeight, pControl, 0, 0);
                cyFixedRemaining -= sz.cy;
            }

            sz.cx = (int)ZuiControlCall(Proc_GetMaxWidth, pControl, 0, 0);
            if (sz.cx == 0) sz.cx = szAvailable.cx - rcPadding.left - rcPadding.right;
            if (sz.cx < 0) sz.cx = 0;
            if (sz.cx > szControlAvailable.cx) sz.cx = szControlAvailable.cx;
            if (sz.cx < (int)ZuiControlCall(Proc_GetMinWidth, pControl, 0, 0)) sz.cx = (int)ZuiControlCall(Proc_GetMinWidth, pControl, 0, 0);

            unsigned int iChildAlign = (unsigned int)ZuiControlCall(Proc_Layout_GetChildAlign, cp, NULL, NULL);
            if (iChildAlign == ZDT_CENTER) {
                int iPosX = (rc.right + rc.left) / 2;
                if (op->m_pHorizontalScrollBar && op->m_pHorizontalScrollBar->m_bVisible) {
                    iPosX += (cxNeeded - rc.right + rc.left) / 2;
                    iPosX -= (int)ZuiControlCall(Proc_ScrollBar_GetScrollPos, op->m_pHorizontalScrollBar, 0, NULL);
                }
                RECT rcCtrl = { iPosX - sz.cx / 2, iPosY + rcPadding.top, iPosX + sz.cx - sz.cx / 2, iPosY + sz.cy + rcPadding.top };
                ZuiControlCall(Proc_SetPos, pControl, &rcCtrl, FALSE);
            }
            else if (iChildAlign == ZDT_RIGHT) {
                int iPosX = rc.right;
                if (op->m_pHorizontalScrollBar && op->m_pHorizontalScrollBar->m_bVisible) {
                    iPosX += cxNeeded - rc.right + rc.left;
                    iPosX -= (int)ZuiControlCall(Proc_ScrollBar_GetScrollPos, op->m_pHorizontalScrollBar, 0, NULL);
                }
                RECT rcCtrl = { iPosX - rcPadding.right - sz.cx, iPosY + rcPadding.top, iPosX - rcPadding.right, iPosY + sz.cy + rcPadding.top };
                ZuiControlCall(Proc_SetPos, pControl, &rcCtrl, FALSE);
            }
            else {
                int iPosX = rc.left;
                if (op->m_pHorizontalScrollBar && op->m_pHorizontalScrollBar->m_bVisible) {
                    iPosX -= (int)ZuiControlCall(Proc_ScrollBar_GetScrollPos, op->m_pHorizontalScrollBar, 0, NULL);
                }
                RECT rcCtrl = { iPosX + rcPadding.left, iPosY + rcPadding.top, iPosX + rcPadding.left + sz.cx, iPosY + sz.cy + rcPadding.top };
                ZuiControlCall(Proc_SetPos, pControl, &rcCtrl, FALSE);
            }

            iPosY += sz.cy + op->m_iChildPadding + rcPadding.top + rcPadding.bottom;
            cyNeeded += sz.cy + rcPadding.top + rcPadding.bottom;
            szRemaining.cy -= sz.cy + op->m_iChildPadding + rcPadding.bottom;
        }
        cyNeeded += (nEstimateNum - 1) * op->m_iChildPadding;

        // Process the scrollbar
        ZuiControlCall(Proc_Layout_ProcessScrollBar, cp, (ZuiAny)&rc, (ZuiAny)(MAKEPARAM(cxNeeded,cyNeeded)));
        return 0;
    }
    case Proc_OnCreate: {
        p = (ZuiVerticalLayout)malloc(sizeof(ZVerticalLayout));
        memset(p, 0, sizeof(ZVerticalLayout));
        //创建继承的控件 保存数据指针
        p->old_udata = ZuiLayoutProc(Proc_OnCreate, cp, 0, 0, 0);
        p->old_call = (ZCtlProc)&ZuiLayoutProc;

        return p;
    }
    case Proc_OnDestroy: {
        ZCtlProc old_call = p->old_call;
        ZuiAny old_udata = p->old_udata;
        old_call(ProcId, cp, old_udata, Param1, Param2);
        
        free(p);
        return 0;
    }
    case Proc_GetType:
        return (ZuiAny)Type_VerticalLayout;
    case Proc_CoreInit:
        return (ZuiAny)TRUE;
    case Proc_CoreUnInit:
        return NULL;
    default:
        break;
    }
    return p->old_call(ProcId, cp, p->old_udata, Param1, Param2);
}





#include "TileLayout.h"
#include "Layout.h"
#include <core/control.h>
#include <stdlib.h>

void* ZCALL ZuiTileLayoutProc(int ProcId, ZuiControl cp, ZuiTileLayout p, void* Param1, void* Param2) {
    switch (ProcId)
    {
    case Proc_SetPos: {
        ZuiDefaultControlProc(ProcId, cp, 0, Param1, Param2);
        ZRect rc = cp->m_rcItem;

        // Adjust for inset
        ZuiLayout op = (ZuiLayout)p->old_udata;
        rc.left += op->m_rcInset.left;
        rc.top += op->m_rcInset.top;
        rc.right -= op->m_rcInset.right;
        rc.bottom -= op->m_rcInset.bottom;

        if (darray_len(op->m_items) == 0) {
            return 0;
        }

        // Position the elements
        if (p->m_szItem.cx > 0) p->m_nColumns = (rc.right - rc.left) / p->m_szItem.cx;
        if (p->m_nColumns == 0) p->m_nColumns = 1;

        int cyNeeded = 0;
        int cxWidth = (rc.right - rc.left) / p->m_nColumns;


        int cyHeight = 0;
        int iCount = 0;
        ZPoint ptTile = { rc.left, rc.top };

        int iPosX = rc.left;

        for (int it1 = 0; it1 < darray_len(op->m_items); it1++) {
            ZuiControl pControl = (ZuiControl)(op->m_items->data[it1]);
            if (!pControl->m_bVisible) continue;
            if (pControl->m_bFloat) {
                ZuiControlCall(Proc_Layout_SetFloatPos, cp, (ZuiAny)it1, 0);
                continue;
            }

            // Determine size
            ZRect rcTile = { ptTile.x, ptTile.y, ptTile.x + cxWidth, ptTile.y };
            if ((iCount % p->m_nColumns) == 0)
            {
                int iIndex = iCount;
                for (int it2 = it1; it2 < darray_len(op->m_items); it2++) {
                    ZuiControl pLineControl = (ZuiControl)(op->m_items->data[it2]);
                    if (!pLineControl->m_bVisible) continue;
                    if (pLineControl->m_bFloat) continue;

                    ZRect *rcPadding = (ZRect *)(ZuiControlCall(Proc_GetPadding, pLineControl, 0, 0));
                    ZSize szAvailable = { rcTile.right - rcTile.left - rcPadding->left - rcPadding->right, 9999 };

                    if (szAvailable.cx < (int)ZuiControlCall(Proc_GetMinWidth, pControl, 0, 0)) { szAvailable.cx = (int)ZuiControlCall(Proc_GetMinWidth, pControl, 0, 0); }
                    if (szAvailable.cx > (int)ZuiControlCall(Proc_GetMaxWidth, pControl, 0, 0)) { szAvailable.cx = (int)ZuiControlCall(Proc_GetMaxWidth, pControl, 0, 0); }


                    ZSize *pszTile = (ZSize *)ZuiControlCall(Proc_EstimateSize, pLineControl, (void *)&szAvailable, 0);
                    ZSize szTile;
                    szTile.cx = pszTile->cx;
                    szTile.cy = pszTile->cy;
                    if (szTile.cx == 0) szTile.cx = p->m_szItem.cx;
                    if (szTile.cy == 0) szTile.cy = p->m_szItem.cy;
                    if (szTile.cx < (int)ZuiControlCall(Proc_GetMinWidth, pControl, 0, 0)) szTile.cx = (int)ZuiControlCall(Proc_GetMinWidth, pControl, 0, 0);
                    if (szTile.cx > (int)ZuiControlCall(Proc_GetMaxWidth, pControl, 0, 0)) szTile.cx = (int)ZuiControlCall(Proc_GetMaxWidth, pControl, 0, 0);
                    if (szTile.cy < (int)ZuiControlCall(Proc_GetMinHeight, pControl, 0, 0)) szTile.cy = (int)ZuiControlCall(Proc_GetMinHeight, pControl, 0, 0);
                    if (szTile.cy > (int)ZuiControlCall(Proc_GetMaxHeight, pControl, 0, 0)) szTile.cy = (int)ZuiControlCall(Proc_GetMaxHeight, pControl, 0, 0);

                    cyHeight = MAX(cyHeight, szTile.cy + rcPadding->top + rcPadding->bottom);
                    if ((++iIndex % p->m_nColumns) == 0) break;
                }
            }

            ZRect *rcPadding = (ZRect *)(ZuiControlCall(Proc_GetPadding, pControl, 0, 0));

            rcTile.left += rcPadding->left;
            rcTile.right -= rcPadding->right;

            // Set position
            rcTile.top = ptTile.y + rcPadding->top;
            rcTile.bottom = ptTile.y + cyHeight;

            ZSize szAvailable = { rcTile.right - rcTile.left, rcTile.bottom - rcTile.top };

            ZSize *pszTile = (ZSize *)ZuiControlCall(Proc_EstimateSize, pControl, (void *)&szAvailable, 0);
            ZSize szTile;
            szTile.cx = pszTile->cx;
            szTile.cy = pszTile->cy;
            if (szTile.cx == 0) szTile.cx = p->m_szItem.cx;
            if (szTile.cy == 0) szTile.cy = p->m_szItem.cy;
            if (szTile.cx == 0) szTile.cx = szAvailable.cx;
            if (szTile.cy == 0) szTile.cy = szAvailable.cy;
            if (szTile.cx < (int)ZuiControlCall(Proc_GetMinWidth, pControl, 0, 0)) { szTile.cx = (int)ZuiControlCall(Proc_GetMinWidth, pControl, 0, 0); }
            if (szTile.cx > (int)ZuiControlCall(Proc_GetMaxWidth, pControl, 0, 0)) { szTile.cx = (int)ZuiControlCall(Proc_GetMaxWidth, pControl, 0, 0); }
            if (szTile.cy < (int)ZuiControlCall(Proc_GetMinHeight, pControl, 0, 0)) { szTile.cy = (int)ZuiControlCall(Proc_GetMinHeight, pControl, 0, 0); }
            if (szTile.cy > (int)ZuiControlCall(Proc_GetMaxHeight, pControl, 0, 0)) { szTile.cy = (int)ZuiControlCall(Proc_GetMaxHeight, pControl, 0, 0); }
            ZRect rcPos = { (rcTile.left + rcTile.right - szTile.cx) / 2, (rcTile.top + rcTile.bottom - szTile.cy) / 2,
                (rcTile.left + rcTile.right - szTile.cx) / 2 + szTile.cx, (rcTile.top + rcTile.bottom - szTile.cy) / 2 + szTile.cy };
            ZuiControlCall(Proc_SetPos, pControl, &rcPos, FALSE);
            if ((++iCount % p->m_nColumns) == 0) {
                ptTile.x = iPosX;
                ptTile.y += cyHeight + op->m_iChildPadding;
                cyHeight = 0;
            }
            else {
                ptTile.x += cxWidth;
            }
            cyNeeded = rcTile.bottom - rc.top;
        }
        return 0;
    }
    case Proc_SetAttribute: {
        if (_tcsicmp(Param1, L"itemsize") == 0) {
            ZuiText pstr = NULL;
            int x = _tcstol(Param2, &pstr, 10);  ASSERT(pstr);
            int y = _tcstol(pstr + 1, &pstr, 10);   ASSERT(pstr);
            ZuiControlCall(Proc_TileLayout_SetItemSize, cp, (ZuiAny)x, (ZuiAny)y);
        }
        else if (_tcsicmp(Param1, L"columns") == 0) ZuiControlCall(Proc_TileLayout_SetColumns, cp, (ZuiAny)_ttoi(Param2), NULL);
        break;
    }
    case Proc_TileLayout_SetColumns: {
        if (Param1 <= 0) return 0;
        p->m_nColumns = (int)Param1;
        ZuiControlNeedUpdate(cp);
        break;
    }
    case Proc_TileLayout_SetItemSize: {
        if (p->m_szItem.cx != (int)Param1 || p->m_szItem.cy != (int)Param2) {
            p->m_szItem.cx = (int)Param1;
            p->m_szItem.cy = (int)Param2;
            ZuiControlNeedUpdate(cp);
        }
        break;
    }
    case Proc_OnCreate: {
        p = (ZuiTileLayout)malloc(sizeof(ZTileLayout));
        memset(p, 0, sizeof(ZTileLayout));
        //创建继承的控件 保存数据指针
        p->old_udata = ZuiLayoutProc(Proc_OnCreate, cp, 0, 0, 0);
        p->old_call = (ZCtlProc)&ZuiLayoutProc;

        return p;
    }
    case Proc_OnDestroy: {
        ZCtlProc old_call = p->old_call;
        ZuiAny old_udata = p->old_udata;

        free(p);

        return old_call(ProcId, cp, old_udata, Param1, Param2);
    }
    case Proc_GetType:
        return (ZuiAny)Type_TileLayout;
    case Proc_CoreInit:
        return (ZuiAny)TRUE;
    case Proc_CoreUnInit:
        return NULL;
    default:
        break;
    }
    return p->old_call(ProcId, cp, p->old_udata, Param1, Param2);
}

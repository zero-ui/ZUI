#ifndef __LIST_H__
#define __LIST_H__
#include <ZUI.h>
#include <core/carray.h>
/**列表头元素结构*/
typedef struct _ZListHeaderItem
{
    ZPoint ptLastMouse;
    ZuiBool m_bDragable;
    unsigned int m_uButtonState;     //按钮状态
    int m_iSepWidth;         //分隔符区域


    ZuiRes m_diNormal;          //通常图片
    ZuiRes m_diHot;             //点燃图片
    ZuiRes m_diPushed;          //按下图片
    ZuiRes m_diFocused;         //焦点图片
    ZuiRes m_diSep;
	ZuiColor m_ColorNormal;     //正常状态
	ZuiColor m_ColorHot;        //高亮状态
	ZuiColor m_ColorPushed;     //按下状态
	ZuiColor m_ColorFocused;    //焦点图片
	ZuiColor m_ColorSep;

    ZuiRes      m_rFont;         //字体
	ZuiRes		m_rListFont;
    unsigned int     m_uTextStyle;    //字体控制
	unsigned int		m_uListTextStyle;//ListElement字体控制
    ZuiColor    m_cTextColor;   //字体颜色
	ZRect		m_rcPadding;

    ZRect Thumb;
    ZuiAny old_udata;
    ZCtlProc old_call;
}*ZuiListHeaderItem, ZListHeaderItem;

/**列表头结构*/
typedef struct _ZListHeader
{
    ZSize cXY;  //表头大小
    ZuiAny old_udata;
    ZCtlProc old_call;
}*ZuiListHeader, ZListHeader;

/**列表体结构*/
typedef struct _ZListBody
{
    ZuiControl m_pOwner;//宿主列表
    ZuiAny old_udata;
    ZCtlProc old_call;
}*ZuiListBody, ZListBody;

/**列表元素结构*/
typedef struct _ZListElement
{
    int m_iIndex;        //列索引
    ZuiBool m_bSelected;    //选中
    unsigned int m_uButtonState;    //列状态
    ZuiControl m_pOwner;    //宿主

    ZuiAny old_udata;
    ZCtlProc old_call;
}*ZuiListElement, ZListElement;

/**列表属性结构*/
#define ZLIST_MAX_COLUMNS 32
typedef struct _ZListInfo
{
    int nColumns;//列数量
    ZRect rcColumn[ZLIST_MAX_COLUMNS];

    ZuiBool bAlternateBk;       //使用交替背景色
    ZuiColor dwLineColor;          //行间线颜色
	ZuiColor dwColumnColor;			//列间线颜色
    ZuiColor dwBkColor;            //背景色
    ZuiColor dwHotBkColor;         //点燃背景色
    ZuiColor dwSelectedBkColor;    //选中背景色
    ZuiColor dwDisabledBkColor;    //禁用背景色

    ZuiBool bShowRowLine;       //显示行线
    ZuiBool bShowColumnLine;    //显示列线
} *ZuiListInfo, ZListInfo;

/**列表控件结构*/
typedef struct _ZList
{
    ZuiBool m_bScrollSelect;
    int m_iCurSel;           //现行选中项
    DArray* m_aSelItems;        //选中项数组
    ZuiBool m_bMultiSel;        //允许多选
    int m_iExpandedItem;

    ZuiControl m_pList;     //列表体容器
    ZuiControl m_pHeader;   //表头容器

    ZListInfo m_ListInfo;   //列表属性

    ZuiAny old_udata;
    ZCtlProc old_call;
}*ZuiList, ZList;
ZEXPORT ZuiAny ZCALL ZuiListProc(int ProcId, ZuiControl cp, ZuiList p, ZuiAny Param1, ZuiAny Param2);
ZEXPORT ZuiAny ZCALL ZuiListBodyProc(int ProcId, ZuiControl cp, ZuiListBody p, ZuiAny Param1, ZuiAny Param2);
ZEXPORT ZuiAny ZCALL ZuiListElementProc(int ProcId, ZuiControl cp, ZuiListElement p, ZuiAny Param1, ZuiAny Param2);
ZEXPORT ZuiAny ZCALL ZuiListHeaderProc(int ProcId, ZuiControl cp, ZuiListHeader p, ZuiAny Param1, ZuiAny Param2);
ZEXPORT ZuiAny ZCALL ZuiListHeaderItemProc(int ProcId, ZuiControl cp, ZuiListHeaderItem p, ZuiAny Param1, ZuiAny Param2);


#endif  //__LIST_H__

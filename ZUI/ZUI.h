#ifndef __ZUI_H__
#define __ZUI_H__

#include <stddef.h>
#include <stdint.h>
#include <tchar.h>
//config
#include "config.h"

#ifdef PLATFORM_OS_WIN
    # if defined(BUILDING_ZUI_SHARED)
        //编译成动态库
        #ifdef __cplusplus
        #define ZEXPORT extern "C" __declspec(dllexport)
        #else
        #define ZEXPORT __declspec(dllexport)
        #endif
        #define ZCALL __stdcall
    # elif defined(USING_ZUI_SHARED)
        //使用动态库
        #ifdef __cplusplus
        #define ZEXPORT extern "C" __declspec(dllimport)
        #else
        #define ZEXPORT __declspec(dllimport)
        #endif
        #define ZCALL __stdcall
    # else
        //编译成静态库
        #ifdef __cplusplus
        #define ZEXPORT extern "C"
        #else
        #define ZEXPORT 
        #endif
        #define ZCALL 
    # endif
#else
    #ifdef __cplusplus
        #define ZEXPORT extern "C" 
    #else
        #define ZEXPORT
    #endif
        #define ZCALL __attribute__((__stdcall__))
#endif

#define lengthof(x) (sizeof(x)/sizeof(*x))
#define MAX max
#define MIN min
#define CLAMP(x,a,b) (MIN(b,MAX(a,x)))
#ifndef ASSERT
#define ASSERT(expr)  _ASSERTE(expr)
#endif

#define MAKEPARAM(a, b)  ((b & 0xFFFFFFFF) << 16) | (a & 0xFFFF)
#define HPARAM(a)   (a & 0xFFFFFFFF) >> 16
#define LPARAM(a)   a & 0xFFFF
//--------------------------------------------------------------------基础数据类型
typedef wchar_t*        ZuiText, _ZuiText, ZText;   //内核默认Unicode存储字符
typedef float           ZuiReal;
typedef int             ZuiBool;
typedef void            ZuiVoid;
typedef void*           ZuiAny;
typedef uint32_t        ZuiColor;
typedef unsigned        ZuiByte;
/**矩形*/
typedef struct _ZRect
{
    int left;   ///左边
    int top;    ///顶边
    int right;  ///右边
    int bottom; ///底边
} *ZuiRect, ZRect;
/**矩形(浮点)*/
typedef struct _ZRectR
{
    ZuiReal left;///左边
    ZuiReal top;///顶边
    ZuiReal right;///宽度
    ZuiReal bottom;///高度
} *ZuiRectR, ZRectR;
/**点*/
typedef struct _ZPoint
{
    int x;
    int y;
} *ZuiPoint, ZPoint;
/**点(浮点)*/
typedef struct _ZPointR
{
    ZuiReal x;
    ZuiReal y;
} *ZuiPointR, ZPointR;
/**大小*/
typedef struct _ZSize
{
    int cx;	///宽度
    int cy;	///高度
} *ZuiSize, ZSize;
/**大小(浮点)*/
typedef struct _ZSizeR
{
	ZuiReal cx;	///宽度
	ZuiReal cy;	///高度
} *ZuiSizeR, ZSizeR;

#define MAKEZRECT(r, L, T, R, B) \
    r.left = L; \
    r.top = T;\
    r.right = R;\
    r.bottom = B;
#define MAKEZRECTP(r, L, T, R, B) \
    r->left = L; \
    r->top = T;\
    r->right = R;\
    r->bottom = B;
//--------------------------------------------------------------------结构定义
//内部结构
typedef struct _ZControl *ZuiControl, ZControl;
//资源包
typedef struct _ZResDB *ZuiResDB, ZResDB;
//单个资源
typedef struct _ZRes *ZuiRes, ZRes;
//图形
typedef struct _ZGraphics *ZuiGraphics, ZGraphics;
//图像
typedef struct _ZImage *ZuiImage, ZImage;
//字体
typedef struct _ZFont *ZuiFont, ZFont;
//路径
typedef struct _ZPath* ZuiPath, ZPath;
//初始化结构
typedef struct _ZuiInitConfig
{
#if (defined PLATFORM_OS_WIN)
    ZuiAny m_hInstance;  //库所在的模块句柄,动态库默认自动为当前模块,静态链接须设置此参数
#endif
    ZuiBool debug;              //启动调试器
    ZuiText default_res;        //默认资源文件,必备,资源字符串
    ZuiText default_fontname;   //默认字体名字

} *ZuiInitConfig, ZInitConfig;

//--------------------------------------------------------------------回调定义
typedef ZuiControl(ZCALL* FINDCONTROLPROC)(ZuiControl, ZuiAny);
typedef ZuiAny(ZCALL *ZCtlProc)(int ProcId, ZuiControl p, ZuiAny UserData, ZuiAny Param1, ZuiAny Param2);
typedef ZuiAny(ZCALL *ZNotifyProc)(ZuiText msg, ZuiControl p, ZuiAny UserData, ZuiAny Param1, ZuiAny Param2);

//查找控件参数
#define ZFIND_ALL           0x00000000  //查找全部控件
#define ZFIND_VISIBLE       0x00000001  //查找可视的控件
#define ZFIND_ENABLED       0x00000002  //查找可用的控件
#define ZFIND_HITTEST       0x00000004
#define ZFIND_UPDATETEST    0x00000008
#define ZFIND_TOP_FIRST     0x00000010  //自顶
#define ZFIND_FROM_UPDATE   0x01000000
#define ZFIND_FROM_TAB      0x02000000
#define ZFIND_FROM_POINT    0x04000000
#define ZFIND_FROM_NAME     0x08000000
#define ZFIND_ME_FIRST      0x80000000
//控件标志
#define ZFLAG_TABSTOP       0x00000001  
#define ZFLAG_SETCURSOR     0x00000002  
#define ZFLAG_WANTRETURN    0x00000004  
//--------------------------------------------------------------------ResDB类
/*资源类型*/
enum ZREST
{
    ZREST_IMG = 1,
    ZREST_TXT,
    ZREST_STREAM,
    ZREST_FONT, //字体
    ZREST_ZIP ///此类型比较特殊,如果压缩包包含备注信息则作为资源包打开,由ResDB管理,反之则作为普通资源由Res管理
};
//--------------------------------------------------------------------Debug类
#define ZLOG_TYPE_ERROR     0
#define ZLOG_TYPE_DEBUG     1
#define ZLOG_TYPE_WARNING     2
#define ZLOG_TYPE_INFO     3

#define ZuiOK		1
#define ZuiCANCEL	2
#define ZuiYES		3
#define ZuiNO		4
//--------------------------------------------------------------------plugin类
#if 1

#endif
//--------------------------------------------------------------------Base类
#if 1
//功能宏
#define Type_Null               _T("Null")

#define ZTYLE_BOX               1   //单线边框
#define ZTYLE_BKGColor          2   //具有背景色
//-----控件事件
#define Proc_CoreInit           1   //用于控件内核初始化
#define Proc_CoreUnInit         2   //用于控件内核反初始化
#define Proc_GetType            3   //取控件类型
#define Proc_GetObject          4  //获取控件被继承后某一类型的对象
#define Proc_OnCreate           5   //内核创建
#define Proc_OnInit             6   //用户初始化
#define Proc_OnDestroy          7   //销毁
#define Proc_OnSize             8   //
#define Proc_OnEvent            9   //
#define Proc_OnNotify           10   //
#define Proc_OnPaint            11  // 绘制循序：背景颜色->背景图->状态图->文本->边框
#define Proc_EndPaint           12  //子控件绘制完毕后调用
#define Proc_OnPaintBkColor     13  //背景色
#define Proc_OnPaintBkImage     14  //背景图片
#define Proc_OnPaintStatusImage 15  //状态图片
#define Proc_OnPaintText        16  //文本
#define Proc_OnPaintBorder      17  //边框
#define Proc_OnPostPaint        18  //

#define Proc_Invalidate         19  //刷新显示
#define Proc_FindControl        20  //查找控件
#define Proc_JsInit             21  //函数调用
#define Proc_JsCall             22  //函数调用
#define Proc_JsSet              23  //
#define Proc_JsGet              24  //处理js对象 返回0则是变量,返回1则是call

//----控件属性
#define Proc_SetAttribute       25  //解析属性
#define Proc_GetAttribute       26  //取属性
#define Proc_GetControlFlags    27  //
#define Proc_Activate           28  //活动
#define Proc_SetVisible         29  //设置是否可视
#define Proc_GetVisible         30  //是否可视
#define Proc_SetText            31  //设置文本
#define Proc_GetText            32  //设置文本
#define Proc_SetName            33  //设置名字
#define Proc_SetTooltip         34  //设置提示文本
#define Proc_GetPos             35  //得到控件位置
#define Proc_SetPos             36  //设置控件位置并重绘
#define Proc_SetOs              37  //设置控件的绘制管理者
#define Proc_SetAnimation       38  //设置动画类型
//设置大小的限制值
#define Proc_GetMinWidth        39  //
#define Proc_SetMinWidth        40  //
#define Proc_GetMaxWidth        41  //
#define Proc_SetMaxWidth        42  //
#define Proc_GetMinHeight       43  //
#define Proc_SetMinHeight       44  //
#define Proc_GetMaxHeight       45  //
#define Proc_SetMaxHeight       46  //
#define Proc_GetWidth           47  //
#define Proc_GetHeight          48  //
#define Proc_GetX               49  //
#define Proc_GetY               50
#define Proc_EstimateSize       51  //获取自适应大小
#define Proc_SetFloatPercent    52  //
#define Proc_GetPadding         53
#define Proc_SetPadding         54  // 设置外边距，由上层窗口绘制
#define Proc_GetFixedXY         55  // 实际大小位置使用GetPos获取，这里得到的是预设的参考值
#define Proc_SetFixedXY         56  // 仅float为true时有效
#define Proc_GetFixedWidth      57  // 实际大小位置使用GetPos获取，这里得到的是预设的参考值
#define Proc_SetFixedWidth      58  // 预设的参考值
#define Proc_GetFixedHeight     59  // 实际大小位置使用GetPos获取，这里得到的是预设的参考值
#define Proc_SetFixedHeight     60  // 预设的参考值
#define Proc_GetRelativePos     61  // 相对(父控件)位置
#define Proc_GetImePoint        62  // 获取输入法位置
#define Proc_SetFloat           63  //设置为浮动控件
#define Proc_SetEnabled         64  //设置可用状态
#define Proc_SetFocus           65  //设置焦点
#define Proc_SetDrag            66  //设置拖拽控件

//-------绘图资源
#define Proc_SetBkColor           67  //设置背景色
#define Proc_SetBkImage         68  //设置背景图片
#define Proc_SetBorderColor     69  //设置边框颜色

#define Proc_SetAnimationType   70  //设置动画类型
#define	Proc_SetBorderWidth		71
#define	Proc_GetBorderWidth		72
#define Proc_OnClose			73
#define Proc_GetEnabled         74
#define Proc_SetRound           75  //设置圆角
//------属性名称
#define BK_Color        0x00000001 //背景色
#define Border_Color    0x00000002 //边框颜色

#define ZuiControlSetDrag(p,b)  ZuiControlCall(Proc_SetDrag,(p),(b),NULL,NULL)
#define ZuiControlSetFixedHeight(p,h) ZuiControlCall(Proc_SetFixedHeight,(p),(h),NULL,NULL)
#endif // 1

//--------------------------------------------------------------------Layout类
#if 1
#define Type_Layout                     _T("Layout")
#define Type_VerticalLayout             _T("VerticalLayout")
#define Type_HorizontalLayout           _T("HorizontalLayout")
#define Type_TileLayout                 _T("TileLayout")
#define Type_TabLayout                  _T("TabLayout")

//--------base
#define Proc_Layout_Add                 101 ///添加控件
#define Proc_Layout_AddAt               102 ///插入控件
#define Proc_Layout_Remove              103 ///移除指定控件, 参数1为真由调用方销毁成员
#define Proc_Layout_RemoveAt            104 ///移除指定位置控件
#define Proc_Layout_RemoveAll           105 ///移除全部控件
#define Proc_Layout_GetCount            106 ///取容器控件数量
#define Proc_Layout_GetItemIndex        107 ///取指定控件索引
#define Proc_Layout_SetItemIndex        108 ///设置控件所在位置
#define Proc_Layout_GetItemAt           109 ///索引取控件
#define Proc_Layout_SetFloatPos         110 ///设置浮动控件位置
#define Proc_Layout_SetInset            112 ///设置内边距

//--------滚动条（由Layout管理）
#define Proc_Layout_GetScrollPos        113
#define Proc_Layout_GetScrollRange      114
#define Proc_Layout_SetScrollPos        115
#define Proc_Layout_SetScrollStepSize   116
#define Proc_Layout_GetScrollStepSize   117
#define Proc_Layout_LineUp              118
#define Proc_Layout_LineDown            119
#define Proc_Layout_PageUp              120
#define Proc_Layout_PageDown            121
#define Proc_Layout_HomeUp              122
#define Proc_Layout_EndDown             123
#define Proc_Layout_LineLeft            124
#define Proc_Layout_LineRight           125
#define Proc_Layout_PageLeft            126
#define Proc_Layout_PageRight           127
#define Proc_Layout_HomeLeft            128
#define Proc_Layout_EndRight            129
#define Proc_Layout_EnableScrollBar     130
#define Proc_Layout_GetVerticalScrollBar    131
#define Proc_Layout_GetHorizontalScrollBar  132
#define Proc_Layout_ProcessScrollBar    133 //调整滚动条位置
#define Proc_Layout_GetChildAlign       134
#define Proc_Layout_GetChildVAlign      135

#define ZuiLayoutAdd(p,cp) ZuiControlCall(Proc_Layout_Add,(p),(cp),NULL,NULL)

//--------Tile
#define Proc_TileLayout_SetColumns      150
#define Proc_TileLayout_SetItemSize     151

//--------Tab
#define Proc_TabLayout_SelectItem       161    //当前Tab

#endif // 1

//--------------------------------------------------------------------Label类
#if 1
#define Type_Label                  _T("Label")
#define Proc_Label_SetFont          171     //设置字体
#define Proc_Label_SetTextColor     172     //设置文本颜色
#define Proc_Label_SetTextPadding   173     //字体边距
#define Proc_Label_SetTextColorDisabled     174
#endif // 1

//--------------------------------------------------------------------Button类
#if 1
#define Type_Button                 _T("Button")

#define Proc_Button_SetResNormal    201    //普通颜色
#define Proc_Button_SetResHot       202    //高亮状态
#define Proc_Button_SetResPushed    203    //按下状态
#define Proc_Button_SetResFocused   204    //焦点图片
#define Proc_Button_SetResDisabled  205    //非激活状态

#define Proc_Button_SetColorNormal    206    //普通颜色
#define Proc_Button_SetColorHot       207    //高亮状态
#define Proc_Button_SetColorPushed    208    //按下状态
#define Proc_Button_SetColorFocused   209    //焦点图片
#define Proc_Button_SetColorDisabled  210    //非激活状态
#endif // 1

//--------------------------------------------------------------------类
#if 1

#endif // 1
//--------------------------------------------------------------------DrawPanel类
#if 1
#define Type_DrawPanel            _T("DrawPanel")
#endif // 1
//--------------------------------------------------------------------SplitterBar类
#if 1
#define Type_SplitterBar            _T("SplitterBar")
#endif // 1
//--------------------------------------------------------------------ProgressBar类
#if 1
#define Type_ProgressBar            _T("ProgressBar")
#define Proc_ProgressBar_SetColor   500
#define Proc_ProgressBar_SetBackColor   501
#define Proc_ProgressBar_SetPercet   502
#define Proc_ProgressBar_SetHeight  503
#endif

//--------------------------------------------------------------------CheckBox类
#if 1
#define Type_CheckBox               _T("CheckBox")
#endif // 1
//--------------------------------------------------------------------List类
#if 1
#define Type_List                           _T("List")
#define Type_ListBody                       _T("ListBody")
#define Type_ListElement                    _T("listElement")
#define Type_ListHeader                     _T("listHeader")
#define Type_ListHeaderItem                 _T("listHeaderItem")


#define Proc_List_GetHeader                 400    //取列表头控件
#define Proc_List_GetListInfo               401    //取列表属性
#define Proc_List_Add                       Proc_Layout_Add
#define Proc_List_GetCount                  Proc_Layout_GetCount
#define Proc_List_AddAt                     Proc_Layout_AddAt
#define Proc_List_Remove                    Proc_Layout_Remove
#define Proc_List_RemoveAt                  Proc_Layout_RemoveAt
#define Proc_List_RemoveAll                 Proc_Layout_RemoveAll
#define Proc_List_SetScrollPos              Proc_Layout_SetScrollPos
#define Proc_List_GetItemAt                 Proc_Layout_GetItemAt
#define Proc_List_SelectItem                402     //选择表项
#define Proc_List_SelectMultiItem           403     //多项选择
#define Proc_List_SetMultiSelect            404     //置允许多项选择
#define Proc_List_IsMultiSelect             405     //是否允许多项选择
#define Proc_List_UnSelectItem              406
#define Proc_List_SelectAllItems            407     //选择全部行
#define Proc_List_UnSelectAllItems          408     //取消所有选择项
#define Proc_List_EnsureVisible             409     //定位滚动条


#define Proc_List_GetVerticalScrollBar      Proc_Layout_GetVerticalScrollBar
#define Proc_List_GetHorizontalScrollBar    Proc_Layout_GetHorizontalScrollBar
#define Proc_List_EnableScrollBar           Proc_Layout_EnableScrollBar

#define Proc_ListHeaderItem_SetSepWidth     400
#define Proc_ListHeaderItem_SetNormalImage  401
#define Proc_ListHeaderItem_SetHotImage     402
#define Proc_ListHeaderItem_SetPushedImage  403
#define Proc_ListHeaderItem_SetFocusedImage 404
#define Proc_ListHeaderItem_SetSepImage     405
#define Proc_ListHeaderItem_GetThumbRect    406
#define Proc_ListHeaderItem_SetDragable     407
#define Proc_ListHeaderItem_EstimateSize    Proc_EstimateSize

#define Proc_ListBody_SetOwner              400    //设置宿主
#define Proc_ListBody_SetScrollPos          Proc_Layout_SetScrollPos

#define Proc_ListElement_SetOwner           400    //设置宿主
#define Proc_ListElement_SetIndex           401    //
#define Proc_ListElement_GetIndex           402    //
#define Proc_ListElement_Select             403     //选中
#define Proc_ListElement_SelectMulti        404     //多选
#endif // 1

//--------------------------------------------------------------------Window类
#if 1
#define Type_Window                 _T("Window")

#define Proc_Window_SetNoBox        1001    //设置为无边框窗体
#define Proc_Window_SetWindowMin    1002    //
#define Proc_Window_SetWindowMax    1003
#define Proc_Window_SetWindowRestor 1004
#define Proc_Window_SetMinInfo      1005
#define Proc_Window_SetMaxInfo      1006
#define Proc_Window_SetSize         1007
#define Proc_Window_SetComBo        1008
#define Proc_Window_Popup           1009
#define Proc_Window_SetToolWindow   1010
#define Proc_Window_Center          1011

#endif // 1

//--------------------------------------------------------------------Option类
#if 1
#define Type_Option                         _T("Option")
#define Proc_Option_SetSelected             1031    //
#define Proc_Option_GetSelected             1032    //

#define Proc_Option_SetResNormal            1033    //选中的普通状态
#define Proc_Option_SetResHot               1034
#define Proc_Option_SetResPushed            1035
#define Proc_Option_SetResFocused           1036
#define Proc_Option_SetResDisabled          1037

#define Proc_Option_SetColorNormal          1038
#define Proc_Option_SetColorHot             1039
#define Proc_Option_SetColorPushed          1040
#define Proc_Option_SetColorFocused         1041
#define Proc_Option_SetColorDisabled        1042

#define Proc_Option_SetGroup                1043    //设置到组
#endif // 1

//--------------------------------------------------------------------ScrollBar类
#if 1
#define Type_ScrollBar                  _T("ScrollBar")
#define Proc_ScrollBar_SetHorizontal    1050 //设置为横向滚动条
#define Proc_ScrollBar_SetScrollPos     1051 //设置位置
#define Proc_ScrollBar_GetScrollPos     1052 //获取位置
#define Proc_ScrollBar_GetScrollRange   1053
#define Proc_ScrollBar_SetOwner         1054
#define Proc_ScrollBar_SetScrollRange   1055
#define Proc_ScrollBar_SetColor         1056
#define Proc_ScrollBar_bShow            1057
//------属性名称
#define ScrollBar_tN_Color        0x00000001 //滑块普通颜色
#define ScrollBar_tH_Color        0x00000002
#define ScrollBar_tP_Color        0x00000004
#define ScrollBar_bN_Color        0x00000008 //按钮普通颜色
#define ScrollBar_bH_Color        0x00000010
#define ScrollBar_bP_Color        0x00000020
#define ScrollBar_Di_Color        0x00000040 //禁用时颜色
#define ScrollBar_BK_Color        0x00000080 //背景色
#define ScrollBar_B1_Show         0x00000001
#define ScrollBar_B2_Show         0x00000002

#endif //1
//--------------------------------------------------------------------TreeView类
#if 1
#define Type_TreeView                   _T("TreeView")
#define Type_TreeNode                   _T("TreeNode")

#define Proc_TreeView_Add                   Proc_Layout_Add  
#define Proc_TreeView_AddAt                 Proc_Layout_AddAt
#define Proc_TreeView_Remove                Proc_Layout_Remove
#define Proc_TreeView_RemoveAt              Proc_Layout_RemoveAt
#define Proc_TreeView_RemoveAll             Proc_Layout_RemoveAll
#define Proc_TreeView_SetItemExpand         1100    //展开关闭
#define Proc_TreeView_SetItemCheckBox       1101    //选中反选

#define Proc_TreeNode_GetCountChild         1100
#define Proc_TreeNode_GetChildNode          1101
#define Proc_TreeNode_SetTreeView           1102
#define Proc_TreeNode_Add                   Proc_Layout_Add  
#define Proc_TreeNode_AddAt                 Proc_Layout_AddAt
#define Proc_TreeNode_Remove                Proc_Layout_Remove
#define Proc_TreeNode_AddChildNode          1103
#define Proc_TreeNode_CalLocation           1104    //计算缩进
#define Proc_TreeNode_GetLastNode           1105
#define Proc_TreeNode_GetTreeIndex          1106    //取得全局树视图的索引
#define Proc_TreeNode_GetDottedLine         1107
#define Proc_TreeNode_SetParentNode         1108    //设置父节点
#define Proc_TreeNode_GetItemButton         1109
#define Proc_TreeNode_IsHasChild            1110    //是否有子节点
#define Proc_TreeNode_SetVisibleFolderBtn   1111
#define Proc_TreeNode_GetVisibleFolderBtn   1112
#define Proc_TreeNode_SetVisibleCheckBtn    1113
#define Proc_TreeNode_GetVisibleCheckBtn    1114
#define Proc_TreeNode_GetFolderButton       1115
#define Proc_TreeNode_GetCheckBox           1116
#endif // 1
//--------------------------------------------------------------------内部函数导出表
//zui引擎
#define ZuiFuncsVersion                     1.1
typedef struct _ZuiFuncs {
    uint16_t size;      //结构大小
    uint16_t version;   //结构版本

    int(ZCALL *ZuiMsgLoop)();
    ZuiVoid(ZCALL *ZuiMsgLoop_exit)(int nRet);
    ZuiControl(ZCALL *NewZuiControl)(ZuiText classname, ZuiAny Param1, ZuiAny Param2);
    ZuiVoid(ZCALL *FreeZuiControl)(ZuiControl p, ZuiBool Delayed);
    ZuiAny(ZCALL *ZuiControlCall)(int ProcId, ZuiControl p, ZuiAny Param1, ZuiAny Param2);

}ZuiFuncs;
//js引擎
typedef struct _ZuvFuncs {
    uint16_t size;      //结构大小
    uint16_t version;   //结构版本
                        

}ZuvFuncs;
//--------------------------------------------------------------------导出函数
#if defined(__cplusplus)
extern "C"
{
#endif
    //初始化
    ZEXPORT ZuiBool ZCALL ZuiInit(ZuiInitConfig config);
    //反初始化
    ZEXPORT ZuiBool ZCALL ZuiUnInit();
    //Zui消息循环.
    ZEXPORT int ZCALL ZuiMsgLoop();
    //退出Zui消息循环.
    ZEXPORT ZuiVoid ZCALL ZuiMsgLoop_exit(int nRet);
    
    ZEXPORT ZuiControl ZCALL NewZuiControl(ZuiText classname, ZuiAny Param1, ZuiAny Param2);//创建控件
    ZEXPORT ZuiVoid ZCALL FreeZuiControl(ZuiControl p, ZuiBool Delayed);//销毁控件
    ZEXPORT ZuiAny ZCALL ZuiControlCall(int ProcId, ZuiControl p, ZuiAny Param1, ZuiAny Param2);//调用控件处理函数
    ZEXPORT ZuiControl ZCALL ZuiControlFindName(ZuiControl p, ZuiText Name);
    ZEXPORT ZuiVoid ZCALL ZuiControlRegNotify(ZuiControl p, ZNotifyProc pNotify);
    ZEXPORT int ZCALL ZuiMsgBox(ZuiControl rp, ZuiText text, ZuiText title);

    //载入布局窗口
    ZEXPORT ZuiControl ZCALL ZuiLayoutLoad(ZuiAny xml, int len);

    //资源包
    ZEXPORT ZuiResDB ZCALL ZuiResDBCreateFromBuf(ZuiAny data, int len, ZuiText Pass);
    ZEXPORT ZuiResDB ZCALL ZuiResDBCreateFromFile(ZuiText Path, ZuiText Pass);
    ZEXPORT ZuiVoid ZCALL ZuiResDBDestroy(ZuiResDB db);
    //资源
    ZEXPORT ZuiRes ZCALL ZuiResDBNewTempRes(ZuiAny b, int buflen, int type);
    ZEXPORT ZuiRes ZCALL ZuiResDBGetRes(ZuiText Path, int type);//获取一个资源
    ZEXPORT ZuiVoid ZCALL ZuiResDBDelRes(ZuiRes res);//释放一个资源
    ZEXPORT ZuiAny ZCALL ZuiResGetData(ZuiRes res,int *plen);//获取资源中的数据
#ifdef PLATFORM_OS_WIN
    ZEXPORT ZuiBool ZCALL ZuiResDBAddPE(ZuiText name, ZuiAny hInstance);//添加一个PE文件到资源池
#endif // PLATFORM_OS_WIN


#if defined(__cplusplus)
}
#endif

#endif

#ifndef __ZUI_PLATFORM_GRAPH_H__
#define __ZUI_PLATFORM_GRAPH_H__
#include <ZUI.h>
#if defined(__cplusplus)
extern "C"
{
#endif

#define ARGB(A,R,G,B) ((int)((((A)&0xff)<<24)|(((R)&0xff)<<16)|(((G)&0xff)<<8)|((B)&0xff)))
#define RGBA(R,G,B,A) ARGB(A,R,G,B)
#define RGB2ARGB(COLOR,A) RGBA(((COLOR) >> 16 & 0xFF), ((COLOR) >> 8 & 0xFF), ((COLOR) & 0xFF), (A))
#define ARGB2BGR(COLOR)  (0x0)|(((COLOR) & 0xFF) << 16 )|((COLOR) & 0x0000FF00)|(((COLOR) & 0x00FF0000) >> 16)
#ifdef LINUX
#define RGB(r,g,b) ((int)(((ZuiByte)(r)|((short)((ZuiByte)(g))<<8))|(((int)(ZuiByte)(b))<<16)))
#endif

    /*FontStyleRegular    = 0,//常规
    FontStyleBold       = 1,//加粗
    FontStyleItalic     = 2,//斜体
    FontStyleBoldItalic = 3,//粗斜
    FontStyleUnderline  = 4,//下划线
    FontStyleStrikeout  = 8//强调线，即在字体中部划线
    */
#define ZTS_BOLD            1	//粗体
#define ZTS_ITALIC          2	//斜体
#define ZTS_ALIGN_LEFT      0	//水平居左
#define ZTS_ALIGN_CENTER    4	//水平居中
#define ZTS_ALIGN_RIGHT		8	//水平居右
#define ZTS_VALIGN_TOP		0	//垂直顶部对齐
#define ZTS_VALIGN_MIDDLE	16	//垂直居中对齐
#define ZTS_VALIGN_BOTTOM	32	//垂直底部对齐
#define ZTS_SHADOW			64	//阴影


#include "format.h"

    /** 此函数用作初始化图形引擎.
    * @return 此函数没有返回值.
    */
    ZuiBool ZuiGraphInitialize();
    ZuiVoid ZuiGraphUnInitialize();
    //--------------------Render

    //填充矩形
    ZEXPORT ZuiVoid ZCALL ZuiDrawFillRect(ZuiGraphics gp, ZuiColor incolor, ZuiRect rc);
    //绘制矩形
    ZEXPORT ZuiVoid ZCALL ZuiDrawRect(ZuiGraphics gp, ZuiColor incolor, ZuiRect rc, ZuiReal LineWidth);
    //填充圆角矩形
    ZEXPORT ZuiVoid ZCALL ZuiDrawFillRoundRect(ZuiGraphics gp, ZuiColor incolor, ZuiRect rc, ZuiReal w, ZuiReal h);
    //绘制圆角矩形
    ZEXPORT ZuiVoid ZCALL ZuiDrawRoundRect(ZuiGraphics gp, ZuiColor incolor, ZuiRect rc, ZuiReal w, ZuiReal h, ZuiReal LineWidth);

    //填充三角形
    ZEXPORT ZuiVoid ZCALL ZuiDrawFilltriangle(ZuiGraphics gp, ZuiColor incolor, ZuiReal x1, ZuiReal y1, ZuiReal x2, ZuiReal y2, ZuiReal x3, ZuiReal y3);
    //绘制三角形
    ZEXPORT ZuiVoid ZCALL ZuiDrawtriangle(ZuiGraphics gp, ZuiColor incolor, ZuiReal x1, ZuiReal y1, ZuiReal x2, ZuiReal y2, ZuiReal x3, ZuiReal y3, ZuiReal LineWidth);
    //绘制路径
    ZEXPORT ZuiVoid ZCALL ZuiDrawPath(ZuiGraphics gp, ZuiColor incolor, ZuiPointR * point, int count, ZuiReal LineWidth);
    //绘制直线
    ZEXPORT ZuiVoid ZCALL ZuiDrawLine(ZuiGraphics gp, ZuiColor incolor, ZuiRect rc, ZuiReal LineWidth);
    //绘制文本
    ZEXPORT ZuiVoid ZCALL ZuiDrawStringPt(ZuiGraphics gp, ZuiFont Font, ZuiColor incolor, ZuiText String, int StrLens, ZPointR Pt[]);
    ZEXPORT ZuiVoid ZCALL ZuiDrawString(ZuiGraphics gp, ZuiFont Font, ZuiText String, int StrLens, ZRect * Rect, ZuiColor incolor, unsigned int TextStyle);
    //测量字符大小
    ZEXPORT ZuiVoid ZCALL ZuiMeasureTextSize(ZuiFont Font, _ZuiText String, ZuiSizeR Size);
    //绘制图像
    ZEXPORT ZuiVoid ZCALL ZuiDrawImageEx(ZuiGraphics gp, ZuiImage Img, ZuiReal x, ZuiReal y, ZuiReal Width, ZuiReal Height, ZuiReal xSrc, ZuiReal ySrc, ZuiReal WidthSrc, ZuiReal HeightSrc, ZuiByte Alpha);
    //复制图形
    ZEXPORT ZuiVoid ZCALL ZuiAlphaBlendEx(ZuiGraphics Dest, int srcX1, int srcY1, int srcX2, int srcY2, int dstX, int dstY, ZuiGraphics Src, ZuiByte Alpha);
    ZEXPORT ZuiVoid ZCALL ZuiAlphaBlend(ZuiGraphics Dest, int x, int y, int Width, int Height, ZuiGraphics Src, int xSrc, int ySrc, ZuiByte Alpha);
    ZEXPORT ZuiVoid ZCALL ZuiBitBltEx(ZuiGraphics Dest, int srcX1, int srcY1, int srcX2, int srcY2, int dstX, int dstY, ZuiGraphics Src);
    //清空图形
    ZEXPORT ZuiVoid ZCALL ZuiGraphicsClear(ZuiGraphics gp, ZuiColor incolor);
    //创建字体
    ZEXPORT ZuiFont ZCALL ZuiCreateFont(ZuiText FontName, ZuiReal FontSize, ZuiBool Bold, ZuiBool Italic);
    //销毁字体
    ZEXPORT ZuiVoid ZCALL ZuiDestroyFont(ZuiFont Font);
    //创建图形
    //ZEXPORT ZuiGraphics ZCALL ZuiCreateGraphicsInMemory(int Width, int Height);
    ZEXPORT ZuiGraphics ZCALL ZuiCreateGraphics(int Width, int Height);
    /*附加到一块内存上*/
    ZEXPORT ZuiGraphics ZCALL ZuiCreateGraphicsAttach(ZuiGraphics gp, ZuiAny bits, int Width, int Height, int stride);
    //销毁图形
    ZEXPORT ZuiVoid ZCALL ZuiDestroyGraphics(ZuiGraphics gp);

    //设置剪裁区
    ZEXPORT ZuiBool ZCALL ZuiGraphicsPushClipRect(ZuiGraphics gp, ZuiRect box, int mode);
    //ZEXPORT ZuiBool ZCALL PushClipRegion(IRegion* pRegion, UINT mode = RGN_AND);
    ZEXPORT ZuiBool ZCALL ZuiGraphicsPopClip(ZuiGraphics gp);
    ZEXPORT ZuiBool ZCALL ZuiGraphicsSetClipBox(ZuiGraphics gp, ZuiRect box, int mode);
    ZEXPORT ZuiBool ZCALL ZuiGraphicsExcludeClipRect(ZuiGraphics gp, ZuiRect box);
    ZEXPORT ZuiBool ZCALL ZuiGraphicsIntersectClipRect(ZuiGraphics gp, ZuiRect box);
    //
    ZEXPORT ZuiBool ZCALL ZuiGraphicsSaveClip(ZuiGraphics gp, int * pnState);
    ZEXPORT ZuiBool ZCALL ZuiGraphicsRestoreClip(ZuiGraphics gp, int nState);
    //获取剪裁区
    //ZEXPORT ZuiVoid ZCALL GetClipRegion(IRegion** ppRegion);
    ZEXPORT ZuiBool ZCALL ZuiGraphicsGetClipBox(ZuiGraphics gp, ZuiRect box);


    /** 此函数用作从文件载入图像.
    * @param FileName 路径
    * @return 成功返回ZuiImage对象.
    */
    ZEXPORT ZuiImage ZCALL ZuiLoadImageFromFile(ZuiText FileName);
    /** 此函数用作从字节集载入图像.
    * @param buf 地址
    * @param len 长度
    * @return 成功返回ZuiImage对象.
    */
    ZEXPORT ZuiImage ZCALL ZuiLoadImageFromBinary(ZuiAny buf, size_t len);


    /*取图像帧数*/
    ZEXPORT int ZCALL ZuiImageGetFrameCount(ZuiImage Img);
    /*取图像各帧延时*/
    ZEXPORT int ZCALL ZuiImageGetFrameIniDly(ZuiImage Img, int * arry);
    /*设置图像当前帧*/
    ZEXPORT int ZCALL ZuiImageSetFrame(ZuiImage Img, int index);
    /** 此函数用作销毁ZuiImage对象.
    * @param Image ZuiImage对象
    * @return 此函数没有返回值.
    */
    ZEXPORT ZuiVoid ZCALL ZuiDestroyImage(ZuiImage Img);


#ifdef __cplusplus
}
#endif
#endif
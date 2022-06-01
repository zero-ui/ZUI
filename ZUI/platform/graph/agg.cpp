#include "agg.h"
#include "graph.h"
#include "core/carray.h"
#if PLATFORM_GRAPH_AGG
#if defined(__cplusplus)
#include "include\agg_color_rgba.h"
#pragma comment(lib, "Gdiplus.lib")
//图像解码器

#include <vector>


#include "agg2d.h"

typedef struct
{
    int GdiplusVersion;
    int DebugEventCallback;
    int SuppressBackgroundThreadc;
    int SuppressExternalCodecs;
}GdiplusStartupInput;
#ifdef __cplusplus
extern "C" {
#endif
    int __stdcall GdiplusStartup(int *token, GdiplusStartupInput *input, void *output);
    int __stdcall GdiplusShutdown(int token);
    int __stdcall GdipCreateFromHDC(HDC hdc, void **graphics);
    int __stdcall GdipDeleteGraphics(void *graphics);
    int __stdcall GdipDrawImageI(void *graphics, void *image, int x, int y);
    int __stdcall GdipGraphicsClear(void *graphics, ZuiColor color);
    int __stdcall GdipLoadImageFromStream(void* stream, void **image);
    int __stdcall GdipGetImageWidth(void *image, int *width);
    int __stdcall GdipGetImageHeight(void *image, int *height);
    int __stdcall GdipImageGetFrameCount(void *image, char *guid, int *cunt);
    int __stdcall GdipImageSelectActiveFrame(void *image, char *guid, int *index);
    int __stdcall GdipGetPropertyItemSize(void *, int, int*);
    int __stdcall GdipGetPropertyItem(void *, int, int, char*);
    int __stdcall GdipDisposeImage(void *image);
    int __stdcall GdipDrawImageRectRectI(void *graphics, void *image, int dstx, int dsty, int dstwidth, int dstheight, int srcx, int srcy, int srcwidth, int srcheight, int srcUnit, void* imageAttributes, void *callback, void * callbackData);
    int __stdcall CreateStreamOnHGlobal(HGLOBAL hGlobal, BOOL fDeleteOnRelease, void **ppstm);
#ifdef __cplusplus
}
#endif


extern "C"
{
#endif
#define ARGBTORGBA8(A) (agg::rgba8(((A) >> 16) & 0xFF, ((A) >> 8) & 0xFF, (A) & 0xff, ((A) >> 24) & 0xFF))
#define ARGBTORGBA8P(A) (new agg::rgba8(((A) >> 16) & 0xFF, ((A) >> 8) & 0xFF, (A) & 0xff, ((A) >> 24) & 0xFF))
    /**AGG图形*/
    struct ZuiAggGraphics {
        Agg2D *graphics;                    //图形对象
    };
    /**AGG图像*/
    struct ZuiAggImage {
        HDC hdc;
        HBITMAP HBitmap;///位图句柄
        void *graphics;
        void *buf;
        void *image;
        Agg2D::Image *img;
    };
    /**AGG字体*/
    struct ZuiAggFont {
        Agg2D::Font *font;
    };
    DArray* rcDarray;

    int pGdiToken;
    /*初始化图形接口*/
    ZuiBool ZuiGraphInitialize() {
        GdiplusStartupInput gdiplusStartupInput;
        memset(&gdiplusStartupInput, 0, sizeof(GdiplusStartupInput));
        gdiplusStartupInput.GdiplusVersion = 1;
        GdiplusStartup((int *)&pGdiToken, &gdiplusStartupInput, NULL);//初始化GDI+
        rcDarray = darray_create();
        return TRUE;
    }
    /*反初始化图形接口*/
    ZuiVoid ZuiGraphUnInitialize() {
        GdiplusShutdown(pGdiToken);
        darray_destroy(rcDarray);
    }
    
    /*填充矩形*/
    ZEXPORT ZuiVoid ZCALL ZuiDrawFillRect(ZuiGraphics Graphics, ZuiColor Color, ZuiReal Left, ZuiReal Top, ZuiReal Right, ZuiReal Bottom) {
        if (Graphics)
        {
            Graphics->graphics->graphics->fillColor(ARGBTORGBA8(Color));
            Graphics->graphics->graphics->noLine();//不画边框
            Graphics->graphics->graphics->rectangle(Left, Top, Right, Bottom);
        }
    }
    /*画矩形*/
    ZEXPORT ZuiVoid ZCALL ZuiDrawRect(ZuiGraphics Graphics, ZuiColor Color, ZuiReal Left, ZuiReal Top, ZuiReal Right, ZuiReal Bottom, ZuiReal LineWidth) {
        if (Graphics) {
            Graphics->graphics->graphics->lineWidth(LineWidth);
            Graphics->graphics->graphics->lineColor(ARGBTORGBA8(Color));
            Graphics->graphics->graphics->noFill();//不填充
            Graphics->graphics->graphics->rectangle(Left + LineWidth/2, Top + LineWidth / 2, Right - LineWidth / 2, Bottom - LineWidth / 2);
        }
    }
    //填充圆角矩形
    ZEXPORT ZuiVoid ZCALL ZuiDrawFillRoundRect(ZuiGraphics Graphics, ZuiColor Color, ZuiReal Left, ZuiReal Top, ZuiReal Right, ZuiReal Bottom, ZuiReal w, ZuiReal h) {   
        if (Graphics)
        {
            Graphics->graphics->graphics->fillColor(ARGBTORGBA8(Color));
            Graphics->graphics->graphics->noLine();//不画边框
            Graphics->graphics->graphics->roundedRect(Left, Top, Right, Bottom,w,h);
        }
    }
    //绘制圆角矩形
    ZEXPORT ZuiVoid ZCALL ZuiDrawRoundRect(ZuiGraphics Graphics, ZuiColor Color, ZuiReal Left, ZuiReal Top, ZuiReal Right, ZuiReal Bottom, ZuiReal w, ZuiReal h, ZuiReal LineWidth) {
        if (Graphics) {
            Graphics->graphics->graphics->lineWidth(LineWidth);
            Graphics->graphics->graphics->lineColor(ARGBTORGBA8(Color));
            Graphics->graphics->graphics->noFill();//不填充
            Graphics->graphics->graphics->roundedRect(Left, Top, Right, Bottom,w,h);
        }
    }

    //填充三角形
    ZEXPORT ZuiVoid ZCALL ZuiDrawFilltriangle(ZuiGraphics Graphics, ZuiColor Color, ZuiReal x1, ZuiReal y1, ZuiReal x2, ZuiReal y2, ZuiReal x3, ZuiReal y3)
    {
        if (Graphics)
        {
            Graphics->graphics->graphics->fillColor(ARGBTORGBA8(Color));
            Graphics->graphics->graphics->noLine();//不画边框
            Graphics->graphics->graphics->triangle(x1, y1, x2, y2, x3, y3);
        }
    }
    //绘制三角形
    ZEXPORT ZuiVoid ZCALL ZuiDrawtriangle(ZuiGraphics Graphics, ZuiColor Color, ZuiReal x1, ZuiReal y1, ZuiReal x2, ZuiReal y2, ZuiReal x3, ZuiReal y3, ZuiReal LineWidth)
    {
        if (Graphics)
        {
            Graphics->graphics->graphics->lineWidth(LineWidth);
            Graphics->graphics->graphics->lineColor(ARGBTORGBA8(Color));
            Graphics->graphics->graphics->noFill();//不填充
            Graphics->graphics->graphics->triangle(x1, y1, x2, y2, x3, y3);
        }
    }
    /*画直线*/
    ZEXPORT ZuiVoid ZCALL ZuiDrawLine(ZuiGraphics Graphics, ZuiColor Color, ZuiReal x1, ZuiReal y1, ZuiReal x2, ZuiReal y2, ZuiReal LineWidth)
    {
        if (Graphics) {
            Graphics->graphics->graphics->lineWidth(LineWidth);
            Graphics->graphics->graphics->lineColor(ARGBTORGBA8(Color));
            Graphics->graphics->graphics->line(x1 + LineWidth / 2, y1 + LineWidth / 2, x2 - LineWidth / 2, y2 - LineWidth / 2);
        }
    }
    /*画文本(按照计算好的坐标)*/
    ZEXPORT ZuiVoid ZCALL ZuiDrawStringPt(ZuiGraphics Graphics, ZuiFont Font, ZuiColor Color, ZuiText String, int StrLens, ZPointR Pt[]) {
        if (String && Font && Graphics) {
            //指定文字颜色
            Graphics->graphics->graphics->fillColor(ARGBTORGBA8(Color));
            Graphics->graphics->graphics->noLine();//不描边
            for (int i = 0; i < StrLens; i++)
            {
                Graphics->graphics->graphics->text(*Font->font->font, Pt[i].x, Pt[i].y, String[i]);
            }
        }
    }
    ZEXPORT ZuiVoid ZCALL ZuiDrawString(ZuiGraphics Graphics, ZuiFont Font, ZuiText String, int StrLen, ZRectR *Rect, ZuiColor Color, unsigned int TextStyle) {
        if (String && Font && Graphics) {
            //指定文字颜色
            Graphics->graphics->graphics->fillColor(ARGBTORGBA8(Color));
            Graphics->graphics->graphics->noLine();//不描边
            Graphics->graphics->graphics->text(*(Font->font->font), Rect->left, Rect->top, Rect->right, Rect->bottom, String, StrLen, TextStyle);
        }
    }
    /*测量文本大小*/
    ZEXPORT ZuiVoid ZCALL ZuiMeasureTextSize(ZuiFont Font, _ZuiText String, ZuiSizeR Size)
    {
        if (String && Font) {
            Size->cx = Font->font->font->textWidth(String);
            Size->cy = Font->font->font->fontHeight();
        }
    }
    /*画图像缩放*/
    ZEXPORT ZuiVoid ZCALL ZuiDrawImageEx(ZuiGraphics Graphics, ZuiImage Image, ZuiReal x, ZuiReal y, ZuiReal Right, ZuiReal Bottom, ZuiReal xSrc, ZuiReal ySrc, ZuiReal WidthSrc, ZuiReal HeightSrc, ZuiByte Alpha) {
        ZuiReal Width, Height;
        if (!(Graphics && Image)) {
            return;
        }
        if (Image->src.left)
            xSrc = Image->src.left;
        if (Image->src.top)
            ySrc = Image->src.top;
        Width = Right - x;
        Height = Bottom - y;
        if (Width <= 0) {
            if (Image->src.right)
                Width = Image->src.right - xSrc;
            else
                Width = (int)(Image->Width - xSrc);
        }
        if (Height <= 0) {
            if (Image->src.bottom)
                Height = Image->src.bottom - ySrc;
            else
                Height = (int)(Image->Height - ySrc);
        }


        if (WidthSrc <= 0 || WidthSrc + xSrc > Image->Width) {
            if (Image->src.right)
                WidthSrc = Image->src.right;
            else
                WidthSrc = (int)(Image->Width - xSrc);
        }
        if (HeightSrc <= 0 || HeightSrc + ySrc > Image->Height) {
            if (Image->src.bottom)
                HeightSrc = Image->src.bottom;
            else
                HeightSrc = (int)(Image->Height - ySrc);
        }

        Graphics->graphics->graphics->transformImage(*Image->image->img, xSrc, ySrc, xSrc+WidthSrc, ySrc+HeightSrc,
            x, y, x + Width, y + Height);
        return;
    }
    /*复制位图*/
    ZEXPORT ZuiVoid ZCALL ZuiAlphaBlendEx(ZuiGraphics Dest, int srcX1, int srcY1, int srcX2, int srcY2, int dstX, int dstY, ZuiGraphics Src, ZuiByte Alpha) {
        if (Dest && Src) {
            Dest->graphics->graphics->blend(*Src->graphics->graphics, srcX1, srcY1, srcX2, srcY2, dstX, dstY, Alpha);
        }
    }
    ZEXPORT ZuiVoid ZCALL ZuiBitBltEx(ZuiGraphics Dest, int srcX1, int srcY1, int srcX2, int srcY2, int dstX, int dstY, ZuiGraphics Src) {
        if (Dest && Src) {
            Dest->graphics->graphics->copy(*Src->graphics->graphics, srcX1, srcY1, srcX2, srcY2, dstX, dstY);
        }
    }
    /*清除图形*/
    ZEXPORT ZuiVoid ZCALL ZuiGraphicsClear(ZuiGraphics Graphics, ZuiColor Color) {
        if (Graphics)
        {
            Graphics->graphics->graphics->clearAll(ARGBTORGBA8(Color));
        }
    }
    /*创建字体*/
    ZEXPORT ZuiFont ZCALL ZuiCreateFont(ZuiText FontName, ZuiReal FontSize, ZuiBool Bold, ZuiBool Italic) {
        int i = 0;
        ZuiFont Font = (ZuiFont)malloc(sizeof(ZFont));
        if (!Font) { return NULL; }
        memset(Font, 0, sizeof(ZFont));
        Font->font = new ZuiAggFont;
        Font->font->font = new Agg2D::Font(FontName, FontSize, Bold, Italic, Agg2D::VectorFontCache);
        Font->font->font->flipText(true);
        return Font;
    }
    /*销毁字体*/
    ZEXPORT ZuiVoid ZCALL ZuiDestroyFont(ZuiFont StringFormat) {
        if (StringFormat) {
            delete StringFormat->font->font;
            delete StringFormat->font;
            free(StringFormat);
        }
    }
    /*创建图形*/
    ZEXPORT ZuiGraphics ZCALL ZuiCreateGraphicsInMemory(int Width, int Height) {
        ZuiGraphics Graphics = (ZuiGraphics)malloc(sizeof(ZGraphics));
        if (!Graphics) { return NULL; }
        memset(Graphics, 0, sizeof(ZGraphics));
        Graphics->Width = Width;
        Graphics->Height = Height;
        HDC tempdc = GetDC(0);
        Graphics->hdc = CreateCompatibleDC(tempdc);
        ReleaseDC(0, tempdc);
        BITMAPINFO BitmapInfo;
        memset(&BitmapInfo, 0, sizeof(BitmapInfo));
        BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
        BitmapInfo.bmiHeader.biBitCount = 32;
        BitmapInfo.bmiHeader.biWidth = Width;
        BitmapInfo.bmiHeader.biHeight = Height;
        BitmapInfo.bmiHeader.biPlanes = 1;
        BitmapInfo.bmiHeader.biCompression = BI_RGB;
        Graphics->HBitmap = CreateDIBSection(Graphics->hdc, &BitmapInfo, DIB_RGB_COLORS, &Graphics->Bits, 0, 0);
        if (!Graphics->HBitmap) {
            DeleteDC(Graphics->hdc);
            return NULL;
        }
        HBITMAP OldBitmap = (HBITMAP)SelectObject(Graphics->hdc, (HGDIOBJ)Graphics->HBitmap);

        Graphics->graphics = new ZuiAggGraphics();

        
        Graphics->graphics->graphics = new Agg2D;
        Graphics->graphics->graphics->antiAliasGamma(1.0);

        Graphics->graphics->graphics->attach((agg::int8u *)Graphics->Bits, Width, Height, -Width * 4);

        return Graphics;
    }
    /*创建一个可复用的图形*/
    ZEXPORT ZuiGraphics ZCALL ZuiCreateGraphics() {
        ZuiGraphics Graphics = (ZuiGraphics)malloc(sizeof(ZGraphics));
        if (!Graphics) { return NULL; }
        memset(Graphics, 0, sizeof(ZGraphics));
        Graphics->graphics = new ZuiAggGraphics();
        Graphics->graphics->graphics = new Agg2D;
        Graphics->graphics->graphics->antiAliasGamma(1.0);
        return Graphics;
    }
    /*附加到一块内存上*/
    ZEXPORT ZuiGraphics ZCALL ZuiCreateGraphicsAttach(ZuiGraphics Graphics, ZuiAny bits, int Width, int Height, int stride) {
        if (!Graphics) { return NULL; }
        if (Graphics->hdc) {
            DeleteDC(Graphics->hdc);
            Graphics->hdc = NULL;
        }
        if (Graphics->HBitmap) {
            DeleteObject(Graphics->HBitmap);
            Graphics->HBitmap = NULL;
        }
        Graphics->Bits = bits;
        Graphics->Width = Width;
        Graphics->Height = Height;
        Graphics->graphics->graphics->attach((agg::int8u *)Graphics->Bits, Width, Height, stride * 4);
        return Graphics;
    }
    ZEXPORT ZuiBool ZCALL ZuiGraphicsPushClipRect(ZuiGraphics Graphics, ZuiRectR box, int mode) {
        if (!Graphics)
            return FALSE;
        ZuiRectR prc = (ZuiRectR)malloc(sizeof(ZRectR));
        Agg2D::RectD rc = Graphics->graphics->graphics->clipBox();
        prc->left = rc.x1; prc->top = rc.y1; prc->right = rc.x2; prc->bottom = rc.y2;
        darray_append(rcDarray,prc);
        Graphics->graphics->graphics->clipBox(box->left, box->top, box->right, box->bottom);
        return TRUE;
    }
    //ZEXPORT ZuiVoid ZCALL PushClipRegion(IRegion* pRegion, int mode){}
    /*弹出剪裁区*/
    ZEXPORT ZuiBool ZCALL ZuiGraphicsPopClip(ZuiGraphics Graphics) {
        if (!Graphics)
            return FALSE;
        ZuiRectR prc = (ZuiRectR)darray_getat(rcDarray,rcDarray->count - 1);
        Graphics->graphics->graphics->clipBox(prc->left, prc->top, prc->right, prc->bottom);
        darray_delete(rcDarray,rcDarray->count - 1);
        free(prc);
        return TRUE;
    }

    /*销毁图形*/
    ZEXPORT ZuiVoid ZCALL ZuiDestroyGraphics(ZuiGraphics Graphics) {
        if (Graphics) {
            delete Graphics->graphics->graphics;
            delete (ZuiAggGraphics*)Graphics->graphics;
            if (Graphics->hdc) {
                DeleteDC(Graphics->hdc);
                Graphics->hdc = NULL;
            }
            if (Graphics->HBitmap) {
                DeleteObject(Graphics->HBitmap);
                Graphics->HBitmap = NULL;
            }
            free(Graphics);
        }
    }
    //设置剪裁区
    ZEXPORT ZuiBool ZCALL ZuiGraphicsSetClipBox(ZuiGraphics Graphics, ZuiRectR box) {
        if (Graphics && box) {
            Graphics->graphics->graphics->clipBox(box->left, box->top, box->right, box->bottom);
            return TRUE;
        }
        return FALSE;
    }
    //获取剪裁区
    ZEXPORT ZuiBool ZCALL ZuiGraphicsGetClipBox(ZuiGraphics Graphics, ZuiRectR box) {
        if (Graphics && box) {
            Agg2D::RectD rc = Graphics->graphics->graphics->clipBox();
            box->left = rc.x1;
            box->top = rc.y1;
            box->right = rc.x2;
            box->bottom = rc.y2;
            return TRUE;
        }
        return FALSE;
    }
    //重置剪裁区
    ZEXPORT ZuiBool ZCALL ZuiGraphicsResetClip(ZuiGraphics Graphics) {
        if (Graphics) {
            Graphics->graphics->graphics->clipBox(0, 0, 0, 0);
        }
        return FALSE;
    }


    /*加载图像自内存*/
    ZEXPORT ZuiImage ZCALL ZuiLoadImageFromBinary(ZuiAny buf, int len) {
        ZuiImage Image = (ZuiImage)malloc(sizeof(ZImage));
        if (!Image) { return NULL; }
        memset(Image, 0, sizeof(ZImage));
        Image->image = new ZuiAggImage();
		if (!Image->image) { free(Image); return NULL; }
        HGLOBAL hMem = GlobalAlloc(GMEM_ZEROINIT | GMEM_MOVEABLE, len);
        void *str;
        CreateStreamOnHGlobal(hMem, 0, &str);
        void * mem = GlobalLock(hMem);
        memcpy(mem, buf, len);
        GlobalUnlock(hMem);
        GdipLoadImageFromStream(str, &Image->image->image);
        GdipGetImageHeight(Image->image->image, (int *)&Image->Height);
        GdipGetImageWidth(Image->image->image, (int *)&Image->Width);

        //得到数据
        HDC tempdc = GetDC(0);
        Image->image->hdc = CreateCompatibleDC(tempdc);
        ReleaseDC(0, tempdc);
        BITMAPINFO BitmapInfo;
        memset(&BitmapInfo, 0, sizeof(BitmapInfo));
        BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
        BitmapInfo.bmiHeader.biBitCount = 32;
        BitmapInfo.bmiHeader.biWidth = Image->Width;
        BitmapInfo.bmiHeader.biHeight = Image->Height;
        BitmapInfo.bmiHeader.biPlanes = 1;
        BitmapInfo.bmiHeader.biCompression = BI_RGB;
        Image->image->HBitmap = CreateDIBSection(Image->image->hdc, &BitmapInfo, DIB_RGB_COLORS, &Image->image->buf, 0, 0);
        if (!Image->image->HBitmap) {
            DeleteDC(Image->image->hdc);
            GdipDisposeImage(Image->image->image);
            delete Image->image;
            free(Image);
            return NULL;
        }
        SelectObject(Image->image->hdc, (HGDIOBJ)Image->image->HBitmap);


        GdipCreateFromHDC(Image->image->hdc, &Image->image->graphics);
        //把图片画到DC上
        GdipDrawImageI(Image->image->graphics, Image->image->image, 0, 0);


        //创建AGG对象
        Image->image->img = new Agg2D::Image((agg::int8u *)Image->image->buf, Image->Width, Image->Height, -Image->Width * 4);
        return Image;
    }
    /*销毁图像*/
    ZEXPORT ZuiVoid ZCALL ZuiDestroyImage(ZuiImage Image) {
        if (Image) {
            GdipDeleteGraphics(Image->image->graphics);
            DeleteDC(Image->image->hdc);
            DeleteObject(Image->image->HBitmap);
            GdipDisposeImage(Image->image->image);
            delete Image->image->img;
            delete Image->image;
            free(Image);
        }
    }

#ifdef __cplusplus
}
#endif


#endif // PLATFORM_GRAPH_AGG

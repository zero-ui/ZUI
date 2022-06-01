#include "gdi+.h"
#include "graph.h"
#include "core/carray.h"
#include "stdlib.h"
#include  <comdef.h>
#if PLATFORM_GRAPH_GDIX
#include "gdiplus.h"
#pragma comment(lib, "Gdiplus.lib")
using namespace Gdiplus;

typedef struct
{
    int GdiplusVersion;
    void * DebugEventCallback;
    int SuppressBackgroundThreadc;
    int SuppressExternalCodecs;
}GdiplusStartupInputQ;

#ifdef __cplusplus
extern "C" {
#endif
    int __stdcall GdiplusStartup(int* token, GdiplusStartupInputQ* input, void* output);
    int __stdcall GdiplusShutdown(int token);
    int __stdcall GdipCreateFromHDC(HDC hdc, void** gp);
    int __stdcall GdipDeleteGraphics(void* gp);
    int __stdcall GdipDrawImageI(void* gp, void* Img, int x, int y);
    int __stdcall GdipGraphicsClear(void* gp, ZuiColor incolor);
    int __stdcall GdipLoadImageFromStream(void* stream, void** Img);
    int __stdcall GdipGetImageWidth(void* Img, int* width);
    int __stdcall GdipGetImageHeight(void* Img, int* height);
    int __stdcall GdipImageGetFrameCount(void* Img, char* guid, int* cunt);
    int __stdcall GdipImageSelectActiveFrame(void* Img, char* guid, int* index);
    int __stdcall GdipGetPropertyItemSize(void*, int, int*);
    int __stdcall GdipGetPropertyItem(void*, int, int, char*);
    int __stdcall GdipDisposeImage(void* Img);
    int __stdcall GdipDrawImageRectRectI(void* gp, void* Img, int dstx, int dsty, int dstwidth, int dstheight, int srcx, int srcy, int srcwidth, int srcheight, int srcUnit, void* ImgAttributes, void* callback, void* callbackData);

    DArray* rcDarray;

    int pGdiToken;
    /*初始化图形接口*/
    ZuiBool ZuiGraphInitialize() {
        GdiplusStartupInputQ gdiplusStartupInput;
        memset(&gdiplusStartupInput, 0, sizeof(GdiplusStartupInputQ));
        gdiplusStartupInput.GdiplusVersion = 1;
        GdiplusStartup((int*)&pGdiToken, &gdiplusStartupInput, NULL);//初始化GDI+
        rcDarray = darray_create();
        return TRUE;
    }
    /*反初始化图形接口*/
    ZuiVoid ZuiGraphUnInitialize() {
        GdiplusShutdown(pGdiToken);
        darray_destroy(rcDarray);
    }

    /*填充矩形*/
    ZEXPORT ZuiVoid ZCALL ZuiDrawFillRect(ZuiGraphics gp, ZuiColor incolor, ZuiRect rc) {
        if (gp)
        {
            SolidBrush brush(incolor);
            Graphics gpp(gp->hdc);
            gpp.FillRectangle(&brush,rc->left,rc->top,rc->right-rc->left,rc->bottom-rc->top);
            gpp.~Graphics();
        }
    }
    /*画矩形*/
    ZEXPORT ZuiVoid ZCALL ZuiDrawRect(ZuiGraphics gp, ZuiColor incolor, ZuiRect rc, ZuiReal LineWidth) {
        if (gp) {
            Pen pen(incolor, LineWidth);
            pen.SetAlignment(PenAlignmentInset);
            Graphics gpp(gp->hdc);
            gpp.DrawRectangle(&pen, rc->left, rc->top, rc->right - rc->left-1, rc->bottom - rc->top-1);
            pen.~Pen();
            gpp.ReleaseHDC(gp->hdc);
            gpp.~Graphics();
 
        }
    }
    //填充圆角矩形
    ZEXPORT ZuiVoid ZCALL ZuiDrawFillRoundRect(ZuiGraphics gp, ZuiColor incolor, ZuiRect rc, ZuiReal w, ZuiReal h) {
        if (gp)
        {
            GraphicsPath path;
            SolidBrush brush(incolor);
            Graphics gpp(gp->hdc);
            if (gp->sysverinfo.dwMajorVersion > 5) {
                gpp.SetSmoothingMode(SmoothingModeAntiAlias);
            }
            path.AddArc((REAL)rc->left, (REAL)rc->top, 2*w, 2*h, 180, 90);
            path.AddLine((REAL)rc->left + w, (REAL)rc->top, rc->right - w, (REAL)rc->top);
            path.AddArc((REAL)rc->right-2*w-1, (REAL)rc->top, 2 * w, 2 * h, 270, 90);
            path.AddLine((REAL)rc->right-1, (REAL)rc->top+h, (REAL)rc->right-1, rc->bottom-h);
            path.AddArc(rc->right - 2*w-1, (REAL)rc->bottom - 2*h-1, 2 * w, 2 * h, 0, 90);
            path.AddLine((REAL)rc->left+w, (REAL)rc->bottom-1, rc->right-w, (REAL)rc->bottom-1);
            path.AddArc((REAL)rc->left, (REAL)rc->bottom - 2*h-1, 2 * w, 2 * h, 90, 90);
            path.AddLine((REAL)rc->left, rc->bottom-h-1, (REAL)rc->left, rc->top + h);
            gpp.FillPath(&brush, &path);
            path.~GraphicsPath();
            gpp.~Graphics();
        }
    }
    //绘制圆角矩形
    ZEXPORT ZuiVoid ZCALL ZuiDrawRoundRect(ZuiGraphics gp, ZuiColor incolor, ZuiRect rc, ZuiReal w, ZuiReal h, ZuiReal LineWidth) {
        if (gp) {
            GraphicsPath path;
            Pen pen(incolor,LineWidth);
            pen.SetAlignment(PenAlignmentInset);
            Graphics gpp(gp->hdc);
            if (gp->sysverinfo.dwMajorVersion > 5) {
                gpp.SetSmoothingMode(SmoothingModeAntiAlias);
            }
            path.AddArc((REAL)rc->left, (REAL)rc->top, 2 * w, 2 * h, 180, 90);
            path.AddLine((REAL)rc->left + w, (REAL)rc->top, rc->right - w, (REAL)rc->top);
            path.AddArc((REAL)rc->right - 2 * w - 1, (REAL)rc->top, 2 * w, 2 * h, 270, 90);
            path.AddLine((REAL)rc->right - 1, (REAL)rc->top + h, (REAL)rc->right - 1, rc->bottom - h);
            path.AddArc(rc->right - 2 * w - 1, (REAL)rc->bottom - 2 * h-1, 2 * w, 2 * h, 0, 90);
            path.AddLine((REAL)rc->left + w, (REAL)rc->bottom - 1, rc->right - w, (REAL)rc->bottom - 1);
            path.AddArc((REAL)rc->left, (REAL)rc->bottom - 2 * h - 1, 2 * w, 2 * h, 90, 90);
            path.AddLine((REAL)rc->left, rc->bottom - h - 1, (REAL)rc->left, rc->top + h);
            gpp.DrawPath(&pen, &path);
            pen.~Pen();
            path.~GraphicsPath();
            gpp.~Graphics();
        }
    }

    //填充三角形
    ZEXPORT ZuiVoid ZCALL ZuiDrawFilltriangle(ZuiGraphics gp, ZuiColor incolor, ZuiReal x1, ZuiReal y1, ZuiReal x2, ZuiReal y2, ZuiReal x3, ZuiReal y3)
    {
        if (gp)
        {
            PointF pt[3];
            SolidBrush brush(incolor);
            Graphics gpp(gp->hdc);
            pt[0].X = x1;
            pt[0].Y = y1;
            pt[1].X = x2;
            pt[1].Y = y2;
            pt[2].X = x3;
            pt[2].Y = y3;
            gpp.FillPolygon(&brush, pt, 3);
            gpp.~Graphics();
        }
    }
    //绘制三角形
    ZEXPORT ZuiVoid ZCALL ZuiDrawtriangle(ZuiGraphics gp, ZuiColor incolor, ZuiReal x1, ZuiReal y1, ZuiReal x2, ZuiReal y2, ZuiReal x3, ZuiReal y3, ZuiReal LineWidth)
    {
        if (gp)
        {
            Point pt[3];
            Pen pen(incolor, LineWidth);
            Graphics gpp(gp->hdc);
            pt[0].X = x1;
            pt[0].Y = y1;
            pt[1].X = x2;
            pt[1].Y = y2;
            pt[2].X = x3;
            pt[2].Y = y3;
            pen.~Pen();
            gpp.DrawPolygon(&pen,pt,3);
            gpp.~Graphics();
        }
    }
    /*画直线*/
    ZEXPORT ZuiVoid ZCALL ZuiDrawLine(ZuiGraphics gp, ZuiColor incolor, ZuiRect rc, ZuiReal LineWidth)
    {
        if (gp) {
            HPEN gPen = CreatePen(PS_SOLID, LineWidth, incolor);
            HPEN oPen = (HPEN)SelectObject(gp->hdc, gPen);
            MoveToEx(gp->hdc, rc->left, rc->top, NULL);
            LineTo(gp->hdc, rc->right, rc->bottom);
            SelectObject(gp->hdc, oPen);
            DeleteObject(gPen);
        }
    }
    /*画文本(按照计算好的坐标)*/
    ZEXPORT ZuiVoid ZCALL ZuiDrawStringPt(ZuiGraphics gp, ZuiFont Font, ZuiColor incolor, ZuiText String, int StrLens, ZPointR Pt[]) {
        if (String && Font && gp) {

        }
    }
    ZEXPORT ZuiVoid ZCALL ZuiDrawString(ZuiGraphics gp, ZuiFont Font, ZuiText String, int StrLen, ZRect* Rect, ZuiColor incolor, unsigned int TextStyle) {
        if (String && Font && gp) {
            SetTextColor(gp->hdc, ARGB2BGR(incolor));
            SetBkMode(gp->hdc, TRANSPARENT);
            SelectObject(gp->hdc, Font->font);
            DrawTextEx(gp->hdc, String, StrLen, (LPRECT)Rect, TextStyle, NULL);
        }
    }
    /*测量文本大小*/
    ZEXPORT ZuiVoid ZCALL ZuiMeasureTextSize(ZuiFont Font, _ZuiText String, ZuiSizeR Size)
    {
        if (String && Font) {

        }
    }
    /*画图像缩放*/
    ZEXPORT ZuiVoid ZCALL ZuiDrawImageEx(ZuiGraphics gp, ZuiImage Img, ZuiReal x, ZuiReal y, ZuiReal Right, ZuiReal Bottom, ZuiReal xSrc, ZuiReal ySrc, ZuiReal WidthSrc, ZuiReal HeightSrc, ZuiByte Alpha) {
        ZuiReal Width, Height;
        if ((gp && Img)) {
            //wprintf(_T("draw...");
            Image image((LPSTREAM)Img->pstream,0);
            Rect rc(x,y,Right-x,Bottom-y);
            Width = Img->src.right - Img->src.left;
            Height = Img->src.bottom - Img->src.top;
            if (Width == 0 && Height == 0) {
                Width = image.GetWidth();
                Height = image.GetHeight();
            }
            Graphics gpp(gp->hdc);
            gpp.DrawImage(&image, rc, Img->src.left, Img->src.top, Width, Height,UnitPixel);
            gpp.~Graphics();
            image.~Image();
        }
    }
    /*复制位图*/
    ZEXPORT ZuiVoid ZCALL ZuiAlphaBlendEx(ZuiGraphics Dest, int srcX1, int srcY1, int srcX2, int srcY2, int dstX, int dstY, ZuiGraphics Src, ZuiByte Alpha) {
        if (Dest && Src) {

        }
    }
    ZEXPORT ZuiVoid ZCALL ZuiBitBltEx(ZuiGraphics Dest, int srcX1, int srcY1, int srcX2, int srcY2, int dstX, int dstY, ZuiGraphics Src) {
        if (Dest && Src) {

        }
    }
    /*清除图形*/
    ZEXPORT ZuiVoid ZCALL ZuiGraphicsClear(ZuiGraphics gp, ZuiColor incolor) {
        if (gp)
        {

        }
    }
    /*创建字体*/
    ZEXPORT ZuiFont ZCALL ZuiCreateFont(ZuiText FontName, ZuiReal FontSize, ZuiBool Bold, ZuiBool Italic) {
        ZuiFont Font = (ZuiFont)malloc(sizeof(ZFont));
        if (!Font) { return NULL; }
        memset(Font, 0, sizeof(ZFont));
        LOGFONT lf;
        GetObject(GetStockObject(SYSTEM_FONT), sizeof(LOGFONT), &lf);
        lf.lfHeight = -MulDiv(FontSize, GetDeviceCaps(GetDC(0), LOGPIXELSY), 72);
        lf.lfWidth = 0;
        lf.lfItalic = Italic;
        lf.lfWeight = FW_NORMAL;
        if (Bold)
            lf.lfWeight = FW_BOLD;
        wsprintf(lf.lfFaceName, _T("%s\0"), FontName);
        Font->font = CreateFont(lf.lfHeight, lf.lfWidth, lf.lfEscapement, lf.lfOrientation, lf.lfWeight, lf.lfItalic,
            lf.lfUnderline, lf.lfStrikeOut, lf.lfCharSet, lf.lfOutPrecision, lf.lfClipPrecision, lf.lfQuality, lf.lfPitchAndFamily, lf.lfFaceName);
        return Font;
    }
    /*销毁字体*/
    ZEXPORT ZuiVoid ZCALL ZuiDestroyFont(ZuiFont Font) {
        if (Font) {
            DeleteObject(Font->font);
            free(Font);
        }
    }
    /*创建图形*/
    ZEXPORT ZuiGraphics ZCALL ZuiCreateGraphics(int Width, int Height) {
        ZuiGraphics gp = (ZuiGraphics)malloc(sizeof(ZGraphics));
        if (!gp) { return NULL; }
        memset(gp, 0, sizeof(ZGraphics));
        if (Width && Height) {
            gp->Width = Width;
            gp->Height = Height;
            gp->Clip.left = 1;
            gp->Clip.top = 1;
            gp->Clip.right = Width;
            gp->Clip.bottom = Height;
            HDC tempdc = GetDC(0);
            gp->hdc = CreateCompatibleDC(tempdc);
            gp->hdc = gp->hdc;
            ReleaseDC(0, tempdc);
            BITMAPINFO BitmapInfo;
            memset(&BitmapInfo, 0, sizeof(BitmapInfo));
            BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
            BitmapInfo.bmiHeader.biBitCount = 32;
            BitmapInfo.bmiHeader.biWidth = Width;
            BitmapInfo.bmiHeader.biHeight = Height;
            BitmapInfo.bmiHeader.biPlanes = 1;
            BitmapInfo.bmiHeader.biCompression = BI_RGB;
            gp->HBitmap = CreateDIBSection(gp->hdc, &BitmapInfo, DIB_RGB_COLORS, &gp->Bits, 0, 0);
            gp->HBitmap = gp->HBitmap;
            if (!gp->HBitmap) {
                DeleteDC(gp->hdc);
                return NULL;
            }
            HBITMAP OldBitmap = (HBITMAP)SelectObject(gp->hdc, (HGDIOBJ)gp->HBitmap);
            if (OldBitmap != NULL) {
                DeleteObject(OldBitmap);
            }
        }
        gp->sysverinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        GetVersionEx((OSVERSIONINFO*)&gp->sysverinfo);
        return gp;
    }
    /*销毁图形*/
    ZEXPORT ZuiVoid ZCALL ZuiDestroyGraphics(ZuiGraphics gp) {
        if (gp) {
            if (gp->HBitmap) {
                DeleteObject(gp->HBitmap);
                gp->HBitmap = NULL;
                DeleteDC(gp->hdc);
                gp->hdc = NULL;
            }
            free(gp);
        }
    }
    /*附加到一块内存上*/
    ZEXPORT ZuiGraphics ZCALL ZuiCreateGraphicsAttach(ZuiGraphics gp, ZuiAny bits, int Width, int Height, int stride) {
        if (!gp) { return NULL; }
        if (gp->hdc) {
            DeleteDC(gp->hdc);
            gp->hdc = NULL;
        }
        if (gp->HBitmap) {
            DeleteObject(gp->HBitmap);
            gp->HBitmap = NULL;
        }
        gp->Bits = bits;
        gp->Width = Width;
        gp->Height = Height;
        return gp;
    }
    ZEXPORT ZuiBool ZCALL ZuiGraphicsPushClipRect(ZuiGraphics gp, ZuiRect box, int mode) {
        if (!gp)
            return FALSE;
        ZuiRect prc = (ZuiRect)malloc(sizeof(ZRect));
        if (!prc)
            return FALSE;
        memcpy(prc, &(gp->Clip), sizeof(ZRect));
        darray_append(rcDarray, prc);
        memcpy(&(gp->Clip), box, sizeof(ZRect));

        HRGN hrgn = CreateRectRgn(box->left, box->top, box->right, box->bottom);
        SelectClipRgn(gp->hdc, hrgn);
        DeleteObject(hrgn);
        return TRUE;
    }
    //ZEXPORT ZuiVoid ZCALL PushClipRegion(IRegion* pRegion, int mode){}
    /*弹出剪裁区*/
    ZEXPORT ZuiBool ZCALL ZuiGraphicsPopClip(ZuiGraphics gp) {
        if (!gp)
            return FALSE;
        ZuiRect prc = (ZuiRect)darray_getat(rcDarray, rcDarray->count - 1);
        memcpy(&(gp->Clip), prc, sizeof(ZRect));
        darray_delete(rcDarray, rcDarray->count - 1);

        HRGN hrgn = CreateRectRgn(prc->left, prc->top, prc->right, prc->bottom);
        SelectClipRgn(gp->hdc, hrgn);
        DeleteObject(hrgn);

        free(prc);
        return TRUE;
    }

    //设置剪裁区
    ZEXPORT ZuiBool ZCALL ZuiGraphicsSetClipBox(ZuiGraphics gp, ZuiRect box, int mode) {

        return FALSE;
    }
    //获取剪裁区
    ZEXPORT ZuiBool ZCALL ZuiGraphicsGetClipBox(ZuiGraphics gp, ZuiRect box) {
        return FALSE;
    }
    //重置剪裁区
    ZEXPORT ZuiBool ZCALL ZuiGraphicsResetClip(ZuiGraphics gp) {
        return FALSE;
    }


    /*加载图像自内存*/
    ZEXPORT ZuiImage ZCALL ZuiLoadImageFromBinary(ZuiAny buf, size_t len) {
        ZuiImage Img = (ZuiImage)malloc(sizeof(ZImage));
        if (!Img) { return NULL; }
        memset(Img, 0, sizeof(ZImage));
        HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, len);
        LPVOID pMem = ::GlobalLock(hMem);
        memcpy(pMem, buf, len);
        ::GlobalUnlock(hMem);

        // create IStream* from global memory
        HRESULT hr = CreateStreamOnHGlobal(hMem, TRUE, (LPSTREAM*)&Img->pstream);
        if (hr != S_OK){
            GlobalFree(hMem);
            free(Img);
            return NULL;
        }
        Img->len = len;
        return Img;
    }
    /*销毁图像*/
    ZEXPORT ZuiVoid ZCALL ZuiDestroyImage(ZuiImage Img) {
        if (Img) {
            if (Img->pstream) {
                LPSTREAM(Img->pstream)->Release();
            }
            free(Img);
        }
    }

#ifdef __cplusplus
}
#endif

#endif // PLATFORM_GRAPH_GDIX
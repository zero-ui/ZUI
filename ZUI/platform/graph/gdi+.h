#ifndef __ZUI_PLATFORM_GRAPH_GDIX_H__
#define __ZUI_PLATFORM_GRAPH_GDIX_H__

#include <ZUI.h>
#ifdef PLATFORM_OS_WIN
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif
/**ͼ��*/
typedef struct _ZImage {
    int Width;
    int Height;
    ZRect src;	//Դ����
#ifdef PLATFORM_OS_WIN
    ZuiAny pstream;
#endif
    size_t len;
}*ZuiImage, ZImage;

/**ͼ��*/
typedef struct _ZGraphics {
    int Width;
    int Height;
    ZRect Clip;
#ifdef PLATFORM_OS_WIN
    HDC hdc;		///�ڴ泡��
    HBITMAP HBitmap;///λͼ���
    OSVERSIONINFOEX sysverinfo;
#endif
    void* Bits;		///�ڴ�ָ��
}*ZuiGraphics, ZGraphics;

/**����*/
typedef struct _ZFont {
    ZuiColor TextColor;
    ZuiColor ShadowColor;
    ZuiReal FontSize;       ///�����С
#ifdef PLATFORM_OS_WIN
    HFONT font;
#endif
}*ZuiFont, ZFont;

#endif //__ZUI_PLATFORM_GRAPH_GDIX_H__

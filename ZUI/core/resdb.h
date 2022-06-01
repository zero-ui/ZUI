#ifndef __ZUI_CORE_RESDB_H__
#define __ZUI_CORE_RESDB_H__
#include <ZUI.h>
#include "tree.h"
/*
资源池设计方案


*/
/*资源包来源位置及类型*/
enum ZRESDBT
{
    ZRESDBT_ZIP_FILE = 1, ///压缩文件,来自文件
    ZRESDBT_ZIP_STREAM, ///压缩文件,来着字节流
    ZRESDBT_FILE, ///文件
    ZRESDBT_STREAM, ///字节流
    ZRESDBT_URL, ///网络资源
    ZRESDBT_PE, ///应用资源
    ZRESDBT_FONT ///字体也属于一类资源,由字体管理器管理
};
/**资源包结构*/
typedef struct _ZResDB
{
    RB_ENTRY(_ZResDB) entry;
    uint32_t key;
#if (defined PLATFORM_OS_WIN)
    HINSTANCE Instance; //pe句柄
#endif // PLATFORM_OS_WIN
    void* uf; //压缩包句柄
    ZuiText pass; //压缩包密码
    int type; //资源包类型
} *ZuiResDB, ZResDB;
RB_HEAD(_ZResDB_Tree, _ZResDB);
RB_PROTOTYPE(_ZResDB_Tree, _ZResDB,,);
/**单个资源*/
typedef struct _ZRes
{
    RB_ENTRY(_ZRes) entry;
    uint32_t hash;

    ZuiAny p; //资源指针
    int plen; //资源长度

    int ref; //引用计数器
    int type; //资源类型
} *ZuiRes, ZRes;
RB_HEAD(_ZRes_Tree, _ZRes);
RB_PROTOTYPE(_ZRes_Tree, _ZRes,,);
/**资源池*/
typedef struct _ZResDBPool
{
    struct _ZResDB_Tree resdb; //所有已经加载过的资源包
    struct _ZRes_Tree res; //所有已经加载过的资源
} *ZuiResDBPool, ZResDBPool;
ZuiBool ZuiResDBInit();
ZuiVoid ZuiResDBUnInit();

#endif //__ZUI_CORE_RESDB_H__


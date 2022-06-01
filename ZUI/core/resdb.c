#include <stdio.h>
#include <platform/platform.h>
#include "tree.h"
#include "resdb.h"
#include "function.h"
#include "builder.h"
#include "unzip.h"
#include "string.h"

#if (defined PLATFORM_OS_WIN)
#pragma comment(lib, "wininet.lib") 
#include <wininet.h>
#endif

static int ZRes_Compare(struct _ZRes *e1, struct _ZRes *e2)
{
    return (e1->hash < e2->hash ? -1 : e1->hash > e2->hash);
}
RB_GENERATE(_ZRes_Tree, _ZRes, entry, ZRes_Compare);
static int ZResDB_Compare(struct _ZResDB *e1, struct _ZResDB *e2)
{
    return (e1->key < e2->key ? -1 : e1->key > e2->key);
}
RB_GENERATE(_ZResDB_Tree, _ZResDB, entry, ZResDB_Compare);

ZuiResDBPool Global_ResDB;                  //全局资源包
ZuiBool ZuiResDBInit() {
    Global_ResDB = (ZuiResDBPool)malloc(sizeof(ZResDBPool));
    if (Global_ResDB) {
        memset(Global_ResDB, 0, sizeof(ZResDBPool));
        //创建流和文件的默认资源包
        ZuiResDB p = (ZuiResDB)malloc(sizeof(ZResDB));
        memset(p, 0, sizeof(ZResDB));
        p->type = ZRESDBT_FILE;
        p->key = Zui_Hash(_T("file"));
        RB_INSERT(_ZResDB_Tree, &Global_ResDB->resdb, p);

        p = (ZuiResDB)malloc(sizeof(ZResDB));
        memset(p, 0, sizeof(ZResDB));
        p->type = ZRESDBT_STREAM;
        p->key = Zui_Hash(_T("stream"));
        RB_INSERT(_ZResDB_Tree, &Global_ResDB->resdb, p);

        p = (ZuiResDB)malloc(sizeof(ZResDB));
        memset(p, 0, sizeof(ZResDB));
        p->type = ZRESDBT_URL;
        p->key = Zui_Hash(_T("url"));
        RB_INSERT(_ZResDB_Tree, &Global_ResDB->resdb, p);
#if (defined PLATFORM_OS_WIN)
        p = (ZuiResDB)malloc(sizeof(ZResDB));
        memset(p, 0, sizeof(ZResDB));
        p->type = ZRESDBT_PE;
        p->Instance = m_hInstance;
        p->key = Zui_Hash(_T("pe_zui"));
        RB_INSERT(_ZResDB_Tree, &Global_ResDB->resdb, p);
#endif
        p = (ZuiResDB)malloc(sizeof(ZResDB));
        memset(p, 0, sizeof(ZResDB));
        p->type = ZRESDBT_FONT;
        p->key = Zui_Hash(_T("font"));
        RB_INSERT(_ZResDB_Tree, &Global_ResDB->resdb, p);

        //加载默认资源包
        //ZuiResDBGetRes(_T("pe_zui:zip:6666"), ZREST_ZIP);
        return TRUE;
    }
    return FALSE;
}
ZuiVoid ZuiResDBUnInit() {
    struct _ZResDB * c = NULL;
    struct _ZResDB * cc = NULL;
    RB_FOREACH_SAFE(c, _ZResDB_Tree, &Global_ResDB->resdb, cc) {
        RB_REMOVE(_ZResDB_Tree, &Global_ResDB->resdb, c);
		ZuiResDBDestroy(c);
        //free(c);
    }
    struct _ZRes * _c = NULL;
    struct _ZRes * _cc = NULL;
    RB_FOREACH_SAFE(_c, _ZRes_Tree, &Global_ResDB->res, _cc) {
        ZuiResDBDelRes(_c);
    }
    free(Global_ResDB);
}
ZEXPORT ZuiResDB ZCALL ZuiResDBCreateFromBuf(ZuiAny data, int len, ZuiText Pass)
{
    ZuiResDB p = (ZuiResDB)malloc(sizeof(ZResDB));
    if (p)
    {
        memset(p, 0, sizeof(ZResDB));
        if (Pass) {
            p->pass = (ZuiText)malloc((wcslen(Pass) + 1)*2);
            memcpy(p->pass, Pass, (wcslen(Pass) + 1) * 2);
        }
        p->uf = unzOpen(0, data, len);
        if (p->uf) {
            char name[265];
            if (unzGetGlobalComment(p->uf, (char *)&name, 255)>0) {
                p->type = ZRESDBT_ZIP_STREAM;
                int bufsize = ZuiAsciiToUnicode(&name, -1, 0, 0) * sizeof(wchar_t);
                wchar_t *txtbuf = malloc(bufsize + 30);
                bufsize = ZuiAsciiToUnicode(&name, bufsize / sizeof(wchar_t), txtbuf, bufsize);
                //添加到资源池
                p->key = Zui_Hash(txtbuf);
				if (RB_FIND(_ZResDB_Tree, &Global_ResDB->resdb, p)) {
					ZuiResDBDestroy(p);
					free(txtbuf);
					return NULL;
				}
                RB_INSERT(_ZResDB_Tree, &Global_ResDB->resdb, p);

                free(txtbuf);
            }else{
                p->type = 0;
            }
            return p;
        }
    }
    return 0;
}
ZEXPORT ZuiResDB ZCALL ZuiResDBCreateFromFile(ZuiText FileName, ZuiText Pass)
{
    ZuiResDB p = (ZuiResDB)malloc(sizeof(ZResDB));
    if (p)
    {
        memset(p, 0, sizeof(ZResDB));
        if (Pass) {
            p->pass = (ZuiText)malloc((wcslen(Pass) + 1) * 2);
            memcpy(p->pass, Pass, (wcslen(Pass) + 1) * 2);
        }
        p->uf = unzOpen(FileName, 0, 0);
        if (p->uf) {
            char name[255];
            p->type = ZRESDBT_ZIP_FILE;
            unzGetGlobalComment(p->uf, (char *)&name, 255);
            int bufsize = ZuiAsciiToUnicode(&name, -1, 0, 0) * sizeof(wchar_t);
            ZuiText txtbuf = (ZuiText)malloc(bufsize);
            bufsize = ZuiAsciiToUnicode(&name, bufsize / sizeof(wchar_t), txtbuf, bufsize);
            //添加到资源池
            p->key = Zui_Hash(txtbuf);
			if (RB_FIND(_ZResDB_Tree, &Global_ResDB->resdb, p)) {
				ZuiResDBDestroy(p);
				free(txtbuf);
				return NULL;
			}
            RB_INSERT(_ZResDB_Tree, &Global_ResDB->resdb, p);
            free(txtbuf);
            return p;
        }
    }
    free(p);
    return 0;
}

//资源操作
ZEXPORT ZuiVoid ZCALL ZuiResDBDestroy(ZuiResDB db)
{
    if (db)
    {
        switch (db->type)
        {
        case ZRESDBT_ZIP_FILE:
        case ZRESDBT_ZIP_STREAM:
            free(db->pass);
            unzClose(db->uf);
            break;
        case ZRESDBT_FILE:
        default:
            break;
        }
        
        
        free(db);
    }
}
ZEXPORT ZuiRes ZCALL ZuiResDBGetRes(ZuiText Path, int type) {
    if (Path) {
        ZuiText pathbuf[1024];
        ZuiText arr[20];
        int arrnum = 20;
        memset(pathbuf, 0, 1024 * sizeof(_ZuiText));
        wcsncpy(pathbuf, Path, 1023);
        ZuiStingSplit(pathbuf, _T(":"), arr, &arrnum);
        if (arrnum < 2)
            return NULL;
        //先查找已经加载过的资源里面是否存在
        ZRes theNode = { 0 };
        theNode.hash = Zui_Hash(Path);
        ZuiRes node = RB_FIND(_ZRes_Tree, &Global_ResDB->res, &theNode);
        if (node) {
            node->ref++;//增加引用计数
            return node;
        }
        //没找到就查找对应的资源包
        ZResDB theDbNode = { 0 };
        theDbNode.key = Zui_Hash(arr[0]);
        ZuiResDB db = RB_FIND(_ZResDB_Tree, &Global_ResDB->resdb, &theDbNode);
        if (!db) {
            return NULL;
        }
        //找到对应的资源包并提取资源
        ZuiAny buf = 0;
        int buflen = 0;
        /*压缩*/if (db->type == ZRESDBT_ZIP_FILE || db->type == ZRESDBT_ZIP_STREAM)
        {
            //转换路径编码
            int len = ZuiUnicodeToAscii(arr[1], -1, 0, 0);
            ZuiAny n = malloc(len);
            ZuiUnicodeToAscii(arr[1], len, n, len);
            unz_file_info64 info;
            int ret = unzLocateFile(db->uf, n, 0);
            if (ret == 0)
            {
                unzGetCurrentFileInfo64(db->uf, &info, n, 256, 0, 0, 0, 0);
                unzOpenCurrentFilePassword(db->uf, (const char *)db->pass);
                buf = malloc((size_t)info.uncompressed_size);
                buflen = (int)info.uncompressed_size;
                ret = unzReadCurrentFile(db->uf, buf, (int)info.uncompressed_size);
                if (ret < 0) {
                    free(buf);
					buf = NULL;
					buflen = 0;
                }
            }
            free(n);
        }
        /*文件*/else if (db->type == ZRESDBT_FILE) {
            FILE*f = _wfopen(arr[1], _T("rb"));
            if (f) {
                fseek(f, 0L, SEEK_END);
                buflen = ftell(f); /* 得到文件大小 */
                buf = malloc(buflen); /* 根据文件大小动态分配内存空间 */
                fseek(f, 0L, SEEK_SET); /* 定位到文件开头 */
                fread(buf, buflen, 1, f); /* 一次性读取全部文件内容 */
                fclose(f);
            }
        }
        /*字节*/else if (db->type == ZRESDBT_STREAM) {
			if (arrnum < 4) return NULL;
            buflen = _wtoi(arr[3]);
            buf = malloc(buflen);
            memcpy(buf, (void *)_wtoi(arr[2]), buflen);
        }
#if (defined PLATFORM_OS_WIN)
        /*网络*/else if (db->type == ZRESDBT_URL) {
            const wchar_t *parseptr1;
            const wchar_t *parseptr2;
            wchar_t host[256];
            wchar_t prot[20];
            WORD wport = INTERNET_DEFAULT_HTTP_PORT;
            ZuiBool https = FALSE;
            int len;
            int i;
            parseptr2 = Path + 4;
            parseptr1 = wcschr(parseptr2, L':');
            if (NULL != parseptr1) {
                len = parseptr1 - parseptr2;
                for (i = 0; i < len; i++) {
                    if (!iswalpha(parseptr2[i])) {
                        goto url_erro;
                    }
                }
                if (len == 5) {
                    wport = INTERNET_DEFAULT_HTTPS_PORT;
                    https = TRUE;
                }
                parseptr1++;
                parseptr2 = parseptr1;
                for (i = 0; i < 2; i++) {
                    if (L'/' != *parseptr2) {
                        goto url_erro;
                    }
                    parseptr2++;
                }
                parseptr1 = wcschr(parseptr2, L':');
                //parseptr1 = NULL;
                if (NULL == parseptr1)//判断有无端口号
                {
                    parseptr1 = wcschr(parseptr2, L'/');
                    if (NULL == parseptr1) {
                        goto url_erro;
                    }
                    len = parseptr1 - parseptr2;
                    //解析主机
                    memcpy(&host, parseptr2, len * sizeof(wchar_t));
                    host[len] = 0;
                }
                else {
                    len = parseptr1 - parseptr2;
                    //解析主机
                    memcpy(&host, parseptr2, len * sizeof(wchar_t));
                    host[len] = 0;
                    parseptr1++;
                    parseptr2 = parseptr1;
                    parseptr1 = wcschr(parseptr2, L'/');
                    if (NULL == parseptr1) {
                        goto url_erro;
                    }
                    len = parseptr1 - parseptr2;
                    memcpy(&prot, parseptr2, len * sizeof(wchar_t));
                    prot[len] = 0;
                    wport = _wtoi(prot);
                    //解析端口
                }
                HINTERNET hInet = InternetOpen(_T("ZuiHttp"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
                if (hInet)
                {
                    HINTERNET  hConnect = InternetConnect(hInet, host, wport, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
                    if (hConnect)
                    {
                        HINTERNET hOpenRequest = 0;
                        if (https)
                            hOpenRequest = HttpOpenRequest(hConnect, _T("GET"), parseptr1, HTTP_VERSION, NULL, 0, INTERNET_FLAG_SECURE, 0); //创建https请求
                        else
                            hOpenRequest = HttpOpenRequest(hConnect, _T("GET"), parseptr1, HTTP_VERSION, NULL, 0, INTERNET_FLAG_DONT_CACHE, 1); //创建http请求
                        if (hOpenRequest)
                        {
                            if (HttpSendRequestA(hOpenRequest, NULL, 0, NULL, 0))
                            {
                                len = 20;
                                HttpQueryInfo(hOpenRequest, HTTP_QUERY_CONTENT_LENGTH, &prot, &len, 0);
                                prot[len] = 0;
                                buflen = _wtoi(prot);
                                buf = malloc(buflen);
                                if (!InternetReadFile(hOpenRequest, buf, buflen, &buflen)) {
                                    buflen = 0;
                                    free(buf);
                                }
                            }
                            InternetCloseHandle(hOpenRequest);
                        }
                        InternetCloseHandle(hConnect);
                    }
                    InternetCloseHandle(hInet);
                }
            url_erro:;
            }
        }
        /*PE*/else if (db->type == ZRESDBT_PE)
        {
            //定位我们的自定义资源，这里因为我们是从本模块定位资源，所以将句柄简单地置为NULL即可
            HRSRC hRsrc = FindResource(db->Instance, (LPCWSTR)_wtoi(arr[2]), arr[1]);
            if (hRsrc) {
                //获取资源的大小
                buflen = SizeofResource(db->Instance, hRsrc);
                if (buflen) {
                    //加载资源
                    HGLOBAL hGlobal = LoadResource(db->Instance, hRsrc);
                    if (hGlobal) {
                        //锁定资源
                        buf = malloc(buflen);
                        if (buf)
                        {
                            void * pbuf = LockResource(hGlobal);
                            if (pbuf)
                            {
                                memcpy(buf, pbuf, buflen);
                            }
                            else
                            {
                                free(buf);
                                buf = 0;
                            }
                        }
                    }
                }
            }
        }
#endif
        /*字体*/else if (db->type == ZRESDBT_FONT)
        {
			buf = (ZuiAny)-1;
			buflen = -1;
        }
        if (buf == 0 || buflen == 0)
            return NULL;
        {
            //创建对应的资源类型
            ZuiRes res = malloc(sizeof(ZRes));
            if (!res)
                return NULL;
            memset(res, 0, sizeof(ZRes));
            res->type = type;
            if (type == ZREST_IMG) {
                ZuiImage img = ZuiLoadImageFromBinary(buf, buflen);
                res->p = img;
                //释放缓冲
                free(buf);
                if (!res->p) {
                    free(res);
                    return NULL;
                }
                for (int i = 2; i < arrnum; i++)
                {
                    if (wcsncmp(arr[i], _T("src='"), 5) == 0) {
                        ZuiText pstr = NULL;
                        img->src.left = _tcstol(arr[i] + 5, &pstr, 10);  ASSERT(pstr);
                        img->src.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
                        img->src.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
                        img->src.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
                    }
                }

            }
            else if (type == ZREST_TXT) {
                int bufsize;
                wchar_t *txtbuf;
                if (ZuiStingIsUtf8(buf, buflen))
                {
                    bufsize = ZuiUtf8ToUnicode(buf, -1, 0, 0) * sizeof(wchar_t);
                    txtbuf = malloc(bufsize + 2);
                    bufsize = ZuiUtf8ToUnicode(buf, buflen, txtbuf, bufsize);
                    txtbuf[bufsize] = 0;
                }
                else
                {
                    bufsize = ZuiAsciiToUnicode(buf, -1, 0, 0) * sizeof(wchar_t);
                    txtbuf = malloc(bufsize + 2);
                    bufsize = ZuiAsciiToUnicode(buf, buflen, txtbuf, bufsize);
                    txtbuf[bufsize] = 0;
                }
                free(buf);
                res->p = txtbuf;
            }
            else if (type == ZREST_ZIP)
            {
                res->p = ZuiResDBCreateFromBuf(buf, buflen, 0);
                free(buf);
                if (!res->p) {
                    free(res);
                    return NULL;
                }
                if (res->type > 0) {
                    //是资源包 不由Res管理
                    free(res);
                    return NULL;
                }
            }
            else if (type == ZREST_STREAM) {
                res->p = buf;
                res->plen = buflen;
            }
            else if (type == ZREST_FONT)
            {
                ZuiText name = Global_DefaultFontName;//字体名字
                ZuiColor color = ARGB(254, 0, 0, 0);
                unsigned int size = 12;
                ZuiBool bold = FALSE;
                ZuiBool italic = FALSE;
                for (int i = 1; i < arrnum; i++)
                {
                    if (wcsncmp(arr[i], _T("name='"), 6) == 0) {
                        if (arr[i][wcslen(arr[i])-1] == '\'')
                        {
                            name = arr[i] + 6;
                            name[wcslen(name)-1] = 0;
                        }
                    }
                    else if (wcsncmp(arr[i], _T("size="), 5) == 0)
                    {
                        size = _wtoi(arr[i] + 5);
                    }
                    else if (wcsncmp(arr[i], _T("bold="), 5) == 0)
                    {
                        //粗体
                        if (wcsncmp(arr[i] + 5, _T("true"), 4) == 0)
                        {
                            bold = TRUE;
                        }
                    }
                    else if (wcsncmp(arr[i], _T("italic="), 7) == 0)
                    {
                        //斜体
                        if (wcsncmp(arr[i] + 7, _T("true"), 4) == 0)
                        {
                            italic = TRUE;
                        }
                    }
                }
                res->p = ZuiCreateFont(name, (ZuiReal)size, bold, italic);
            }
            if (!res->p) {
                free(res);
                return NULL;
            }
            //保存到资源map
            res->ref++;////增加引用计数
			if (db->type == ZRESDBT_STREAM) {			//从字节流添加资源stream:name:address:size 后，
				TCHAR * pstr = _tcschr(Path, _T(':'));	//使用stream:name 引用资源 address:size 不需要一直保存。
				pstr = _tcschr(++pstr, _T(':'));
				if (pstr)
					*pstr = 0;
			}
			res->hash = Zui_Hash(Path);
            RB_INSERT(_ZRes_Tree, &Global_ResDB->res, res);
            return res;
        }
    }
    return NULL;
}
ZEXPORT ZuiVoid ZCALL ZuiResDBDelRes(ZuiRes res) {
    if (res && res->ref) {
        res->ref--;///减少引用计数
        if (res->ref == 0) {
            //计数为0,释放资源
            RB_REMOVE(_ZRes_Tree, &Global_ResDB->res, res);//从map中移除
            if (res->type == ZREST_IMG) {
                ZuiDestroyImage(res->p);
            }
            else if (res->type == ZREST_TXT || res->type == ZREST_STREAM) {
                free(res->p);
            }
            else if (res->type == ZREST_ZIP) {
                ZuiResDBDestroy(res->p);
            }
			else if (res->type == ZREST_FONT) {
				ZuiDestroyFont(res->p);
			}
            free(res);
        }
    }
}

ZEXPORT ZuiAny ZCALL ZuiResGetData(ZuiRes res, int *plen) {
    if (res && plen) {
        *plen = res->plen;
        return res->p;
    }
    return NULL;
}
ZEXPORT ZuiRes ZCALL ZuiResDBNewTempRes(ZuiAny b, int buflen, int type) {
    //创建对应的资源类型
    ZuiRes res = malloc(sizeof(ZRes));
    if (!res)
        return NULL;
    memset(res, 0, sizeof(ZRes));
    res->type = type;
    ZuiAny buf = malloc(buflen);
    memcpy(buf, b, buflen);
    if (type == ZREST_IMG) {
        ZuiImage img = ZuiLoadImageFromBinary(buf, buflen);
        res->p = img;
        //释放缓冲
        free(buf);
        if (!res->p) {
            free(res);
            return NULL;
        }
    }
    else if (type == ZREST_TXT) {
        int bufsize;
        wchar_t *txtbuf;
        if (ZuiStingIsUtf8(buf, buflen))
        {
            bufsize = ZuiUtf8ToUnicode(buf, buflen, 0, 0) * sizeof(wchar_t);
            txtbuf = malloc(bufsize + 2);
            bufsize = ZuiUtf8ToUnicode(buf, buflen, txtbuf, bufsize);
            txtbuf[bufsize] = 0;
        }
        else
        {
            bufsize = ZuiAsciiToUnicode(buf, buflen, 0, 0) * sizeof(wchar_t);
            txtbuf = malloc(bufsize + 2);
            bufsize = ZuiAsciiToUnicode(buf, buflen, txtbuf, bufsize);
            txtbuf[bufsize] = 0;
        }
        free(buf);
        res->p = txtbuf;
    }
    else if (type == ZREST_ZIP)
    {
        res->p = ZuiResDBCreateFromBuf(buf, buflen, 0);
        free(buf);
        if (!res->p) {
            free(res);
            return NULL;
        }
    }
    else if (type == ZREST_STREAM) {
        res->p = buf;
        res->plen = buflen;
    }
    if (!res->p) {
        free(res);
        return NULL;
    }
    //保存到资源map
    res->ref++;////增加引用计数
    return res;
}
#if (defined PLATFORM_OS_WIN)
ZEXPORT ZuiBool ZCALL ZuiResDBAddPE(ZuiText name, ZuiAny hInstance) {
    ZuiResDB p = (ZuiResDB)malloc(sizeof(ZResDB));
    memset(p, 0, sizeof(ZResDB));
    p->type = ZRESDBT_PE;
    p->Instance = hInstance;
    p->key = Zui_Hash(name);
    RB_INSERT(_ZResDB_Tree, &Global_ResDB->resdb, p);
    return TRUE;
}
#endif

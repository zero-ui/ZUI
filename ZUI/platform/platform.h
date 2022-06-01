#ifndef __ZUI_PLATFORM_H__
#define __ZUI_PLATFORM_H__
/*
* 平台移植层头文件
*/
#include <ZUI.h>


#if (defined PLATFORM_OS_WIN)
    #include "os/win.h"
#elif (defined PLATFORM_OS_LINUX)
    #include "os/linux.h"
#elif (defined PLATFORM_OS_MACX)
    #include "os/macx.h"
#elif (defined PLATFORM_OS_ANDROID)
    #include "os/android.h"
#endif

#if (defined PLATFORM_GRAPH_AGG)&&(PLATFORM_GRAPH_AGG == 1)
	#include "graph/agg.h"
#elif (defined PLATFORM_GRAPH_GDIX)&&(PLATFORM_GRAPH_GDIX == 1)
    #include "graph/gdi+.h"
#endif

/*移植层接口定义头文件*/
#include "graph/graph.h"
#include "os/os.h"


#endif //__ZUI_PLATFORM_H__
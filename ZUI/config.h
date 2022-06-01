#ifndef __CONFIG_H__
#define __CONFIG_H__

//config

//运行平台配置
#ifdef _WIN32
    #define PLATFORM_OS_WIN
#elif defined _WIN64
    #define PLATFORM_OS_WIN
#elif defined __linux__
    #define PLATFORM_OS_LINUX
#elif defined MACX
    #define PLATFORM_OS_MACX
#elif defined __ANDROID__
    #define PLATFORM_OS_ANDROID
#endif

//图形接口定义
#define PLATFORM_GRAPH_AGG  0
#define PLATFORM_GRAPH_GDIX 1

#endif

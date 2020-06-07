/*********************************************************************************
 * 文件名称: aliyun_iot_common_log.h
 * 作       者:
 * 版       本:
 * 日       期: 2016-05-30
 * 描       述:
 * 其       它:
 * 历       史:
 **********************************************************************************/
#ifndef ALIYUN_IOT_COMMON_LOG_H
#define ALIYUN_IOT_COMMON_LOG_H

#include <stdio.h>
#include <stdlib.h>
#include "aliyun_iot_platform_datatype.h"


typedef enum IOT_LOG_LEVEL        //日志
{
    ALIOT_LOG_LEVEL_DEBUG = 0,     //此输出级别用于开发阶段的调试，可以是某几个逻辑关键点的变量值的输出，或者是函数返回值的验证等等   
    ALIOT_LOG_LEVEL_INFO,          //此输出级别常用语业务事件信息。例如某项业务处理完毕，或者业务处理过程中的一些信息
    ALIOT_LOG_LEVEL_WARN,          //代表存在潜在的错误，或者触发了容易引起错误的操作。程序可以继续运行，但必须多加注意
    ALIOT_LOG_LEVEL_ERROR,         //代表发生了必须马上处理的错误。此类错误出现以后可以允许程序继续运行，但必须马上修正，如果不修正，就会导致不能完成相应的业务。
    ALIOT_LOG_LEVEL_FATAL,         //代表发生了最严重的错误，会导致整个服务停止（或者需要整个服务停止）。简单地说就是服务死掉了。
    ALIOT_LOG_LEVEL_NONE,          //
}aliot_log_level_t;

extern aliot_log_level_t g_iotLogLevel;

void aliyun_iot_common_log_set_level(aliot_log_level_t iotLogLevel);
#if 0
#define ALIOT_LOG_DEBUG(format, ...) 
#define ALIOT_LOG_INFO(format, ...)
#define ALIOT_LOG_WARN(format, ...)
#define ALIOT_LOG_ERROR(format,...)
#define ALIOT_LOG_FATAL(format, ...) 
#else
#define ALIOT_LOG_DEBUG(format, ...) \
{\
    if(g_iotLogLevel <= ALIOT_LOG_LEVEL_DEBUG)\
    {\
        printf("[debug] %s:%d %s()| "format"\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);\
        fflush(stdout);\
    }\
}

#define ALIOT_LOG_INFO(format, ...) \
{\
    if(g_iotLogLevel <= ALIOT_LOG_LEVEL_INFO)\
    {\
        printf("[info] %s:%d %s()| "format"\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);\
        fflush(stdout);\
    }\
}

#define ALIOT_LOG_WARN(format, ...) \
{\
    if(g_iotLogLevel <= ALIOT_LOG_LEVEL_WARN)\
    {\
        printf("[warn] %s:%d %s()| "format"\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);\
        fflush(stdout);\
    }\
}

#define ALIOT_LOG_ERROR(format,...) \
{\
    if(g_iotLogLevel <= ALIOT_LOG_LEVEL_ERROR)\
    {\
        printf("[error] %s:%d %s()| "format"\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);\
        fflush(stdout);\
    }\
}

#define ALIOT_LOG_FATAL(format, ...) \
{\
    if(g_iotLogLevel <= ALIOT_LOG_LEVEL_FATAL)\
    {\
        printf("[notice] %s:%d %s()| "format"\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);\
        fflush(stdout);\
    }\
}
#endif

#endif

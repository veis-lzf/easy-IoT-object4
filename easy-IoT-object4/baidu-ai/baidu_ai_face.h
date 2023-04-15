
#ifndef BAIDU_AI_FACE_H
#define BAIDU_AI_FACE_H

#if defined(__cplusplus)
extern "C"
{
#endif

#include "aliyun_iot_platform_datatype.h"
#include "aliyun_iot_common_error.h"


#define TOKEN_LEN        (100)

typedef struct AI_AUTH_INFO
{
    char *api_key;
    char *secret_key;
    char* target_face_gid;
    char* target_face_uid;
    char token[TOKEN_LEN];
    char userid[TOKEN_LEN];
}ai_auth_info_t;

#define HTTP_MAX_HOST_LEN   (512)
#define HTTP_RESP_MAX_LEN   (512)

//authentication to AI, get token, return 0 if successful
int32_t ai_get_token(void);
extern void baidu_ai_image_client(void);
//  utf8转为Unicode，让pad可显示中文
int UTF8toGBK(const unsigned char* pszBufIn, int nBufInLen, unsigned char* pszBufOut, int* pnBufOutLen);
#if defined(__cplusplus)
}
#endif


#endif

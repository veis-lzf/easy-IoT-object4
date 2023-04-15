/***************************************
* voice recongnition based on Baidu AI platform
* control the devie(a light) based on the voice/translate utf-8 text from usart to mp3 and play it
* by: wumu
* @copyright  easynetworking.cn
****************************************/
#define DEBUG
//#define DEBUG_TASK

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>




#include <string.h>

#include "aliyun_iot_platform_network.h"
#include "aliyun_iot_common_net.h"

#include "aliyun_iot_platform_datatype.h"
#include "aliyun_iot_platform_memory.h"

#include "aliyun_iot_common_error.h"
#include "aliyun_iot_common_log.h"
#include "aliyun_iot_common_md5.h"
#include "aliyun_iot_common_hmac.h"
#include "aliyun_iot_common_httpclient.h"
#include "aliyun_iot_common_jsonparser.h"

#include "aliyun_iot_ca.h"
#include "baidu_ai_voice.h"
#include  "debug.h"
#include  "drivers.h"

#include "mad.h"   //for mp3 playback
#if BAIDU_AI_VOICE_ENABLE

//#define APIKEY "fmNTHNK2KqXsoosYxhUxL2R7"
//#define SECRETKEY "jEm58eC6ra9fAap7cKOX20LIFQiPhcvr"
//#define FACE_GID "test_group_1"
//#define FACE_UID "test_user_1"

//const static char *ai_auth_host = "https://aip.baidubce.com/oauth/2.0/token";
const static char *ai_voice2text_host = "https://vop.baidu.com/server_api";
const static char *ai_text2voice_host = "https://tsn.baidu.com/text2audio";

extern ai_auth_info_t auth_info;


extern void dac_close(void);
extern int  dac_open(void);

#if 0
aliot_err_t ai_httpclient_audio_recv_response(httpclient_t *client, httpclient_data_t *client_data)
{
    int recflag = 0, ret = ERROR_HTTP_CONN, recvlen = 0, recvindex = 0;
    char buf[HTTPCLIENT_CHUNK_SIZE] = { 0 };
    char * pH = NULL;
    //recflag: 1->already recvd the whole http header
    client_data->response_content_len = 0;
    do
    {
        if (client->net.handle < 0)
        {
            return ret;
        }

        recvlen = HTTPCLIENT_CHUNK_SIZE - 1 - recvindex;
        ret = client->net.read(&client->net, buf + recvindex, recvlen, 10000);
        if(ret > 0)
        {
            buf[recvindex + ret] = 0;
            ALIOT_LOG_ERROR("recvlen=[%d], ret = [%d]\n", recvlen, ret);
            ALIOT_LOG_ERROR("buf = [%s]\n", buf);
            if(recflag == 0)
            {
                pH = strstr(buf, "\r\n\r\n");
                if(pH == NULL)
                {
                    ALIOT_LOG_ERROR("NO header ending found\n");
                    if(ret >= 4)
                    {
                        memcpy(buf, &buf[ret - 4], 3);
                        recvindex = 3;
                    }
                }
                else
                {
                    pH += strlen("\r\n\r\n");
                    while(*pH != 0)
                    {
                        client_data->response_buf[client_data->response_content_len++] = *pH;
                        //need to check the buffer overflow
                        pH++;
                    }

                    recflag = 1;
                }
            }
            else
            {
                pH = buf;
                while(*pH != 0)
                {
                    client_data->response_buf[client_data->response_content_len++] = *pH;
                    //need to check the buffer overflow
                    pH++;
                }
            }

            if(ret == recvlen)
            {   //full data get, try one more chance
                continue;
            }

            client_data->response_buf[client_data->response_content_len++] = 0;
            pH = strstr(client_data->response_buf, "result" );
            if(pH)
            {
                pH += 10;//skip result":["
                while(*pH != '"')
                {
                    printf("[ZZZZZZ] audio out [%x]\n", (int)*pH);
                    pH++;
                }
            }
            uart1_send(client_data->response_buf, client_data->response_content_len);
            return SUCCESS_RETURN;
        }
        else if(ret == 0)
        {
            ALIOT_LOG_ERROR("recv TIMEOUT ret = [%d]\n", ret);
            //return HTTP_RETRIEVE_MORE_DATA;     //timeout, need to read data later
            aliyun_iot_pthread_taskdelay(500);
            continue;
        }
        else
        {
            ALIOT_LOG_ERROR("recv ERROR ret = [%d]\n", ret);
            break;
        }
    } while(1);

    return ret;
}

int ai_httpclient_voice_send_header(httpclient_t *client, char *url, int method, httpclient_data_t *client_data)
{
    char scheme[8] = { 0 };
    char host[HTTPCLIENT_MAX_HOST_LEN] = { 0 };
    char path[HTTPCLIENT_MAX_URL_LEN] = { 0 };
    int len;
    char send_buf[HTTPCLIENT_SEND_BUF_SIZE] = { 0 };
    char buf[HTTPCLIENT_SEND_BUF_SIZE] = { 0 };
    char *meth = (method == HTTPCLIENT_GET) ? "GET" : (method == HTTPCLIENT_POST) ? "POST" : (method == HTTPCLIENT_PUT) ? "PUT" : (method == HTTPCLIENT_DELETE) ? "DELETE" :
                 (method == HTTPCLIENT_HEAD) ? "HEAD" : "";
    int ret;
    int port;
    int i = 0, ext_len = 0;

    /* First we need to parse the url (http[s]://host[:port][/[path]]) */
    //int res = httpclient_parse_url(url, scheme, sizeof(scheme), host, sizeof(host), &(client->remote_port), path, sizeof(path));
    int res = httpclient_parse_url(url, scheme, sizeof(scheme), host, sizeof(host), &port, path, sizeof(path));
    if (res != SUCCESS_RETURN)
    {
        ALIOT_LOG_ERROR("httpclient_parse_url returned %d", res);
        return res;
    }

    //if (client->remote_port == 0)
    //{
    if (strcmp(scheme, "http") == 0)
    {
        //client->remote_port = HTTP_PORT;
    }
    else if (strcmp(scheme, "https") == 0)
    {
        //client->remote_port = HTTPS_PORT;
    }
    //}

    /* Send request */
    memset(send_buf, 0, HTTPCLIENT_SEND_BUF_SIZE);
    len = 0; /* Reset send buffer */

    snprintf(buf, sizeof(buf), "%s %s HTTP/1.1\r\nHost: %s\r\n", meth, path, host); /* Write request */
    ret = httpclient_get_info(client, send_buf, &len, buf, strlen(buf));
    if (ret)
    {
        ALIOT_LOG_ERROR("Could not write request");
        return ERROR_HTTP_CONN;
    }

    /* Send all headers */
    if (client->auth_user)
    {
        httpclient_send_auth(client, send_buf, &len); /* send out Basic Auth header */
    }

    /* Add user header information */
    if (client->header)
    {
        httpclient_get_info(client, send_buf, &len, (char *) client->header, strlen(client->header));
    }

    if (client_data->post_buf != NULL)
    {
        snprintf(buf, sizeof(buf), "Content-Length: %d\r\n", client_data->post_buf_len);
        httpclient_get_info(client, send_buf, &len, buf, strlen(buf));

        //snprintf(buf, sizeof(buf), "Connection: keep-alive\r\n");
        //httpclient_get_info(client, send_buf, &len, buf, strlen(buf));

        if (client_data->post_content_type != NULL)
        {
            snprintf(buf, sizeof(buf), "Content-Type: %s\r\n", client_data->post_content_type);
            httpclient_get_info(client, send_buf, &len, buf, strlen(buf));
        }
    }

    /* Close headers */
    httpclient_get_info(client, send_buf, &len, "\r\n", 0);

    ALIOT_LOG_DEBUG("Trying to write %d bytes http header:%s", len, send_buf);

    //ret = httpclient_tcp_send_all(client->net.handle, send_buf, len);
    ret = client->net.write(&client->net, send_buf, len, 5000);
    if (ret > 0)
    {
        ALIOT_LOG_DEBUG("Written %d bytes", ret);
    }
    else if (ret == 0)
    {
        ALIOT_LOG_ERROR("ret == 0,Connection was closed by server");
        return ERROR_HTTP_CLOSED; /* Connection was closed by server */
    }
    else
    {
        ALIOT_LOG_ERROR("Connection error (send returned %d)", ret);
        return ERROR_HTTP_CONN;
    }

    return SUCCESS_RETURN;
}
#endif
int ai_httpclient_voice_send_userdata(httpclient_t *client, httpclient_data_t *client_data)      //从flash里面获取数据并向服务器发送数据
{
    int ret = 0;
    int len = 0, send_len = 0;
#define MAX_SEND_LEN 512
    char senddata[MAX_SEND_LEN];
    char *rawdata = NULL;

    if (client_data->post_buf_len)
    {
        //ALIOT_LOG_DEBUG("client_data->post_buf:%s", client_data->post_buf);

        while(len < client_data->post_buf_len)
        {
            send_len = (client_data->post_buf_len - len) > MAX_SEND_LEN ? MAX_SEND_LEN : (client_data->post_buf_len - len);

            ret = get_ai_audio_pcm_data(len, send_len, senddata);        // 从flash里面获取数据
            if(ret != 0)
            {
                ALIOT_LOG_ERROR("get_ai_audio_pcm_data error returned %d", ret);
                return ERROR_HTTP_CONN;
            }
            else
            {

                ret = client->net.write(&client->net, senddata, send_len, 5000);         //向服务器中写入数据
                if (ret > 0)
                {
                    ALIOT_LOG_DEBUG("Written %d bytes", ret);
                    len += ret;
                }
                else if (ret == 0)
                {
                    ALIOT_LOG_ERROR("ret == 0,Connection was closed by server");
                    return ERROR_HTTP_CLOSED; /* Connection was closed by server */
                }
                else
                {
                    ALIOT_LOG_ERROR("Connection error (send returned %d)", ret);
                    return ERROR_HTTP_CONN;
                }
            }
        }
    }

    return SUCCESS_RETURN;
}

aliot_err_t ai_httpclient_voice_send_request(httpclient_t *client, char *url, int method, httpclient_data_t *client_data)      //发送头部信息 ，从flash里面获取数据并向服务器发送数据
{
    int ret = ERROR_HTTP_CONN;

    if (client->net.handle < 0)
    {
        return ret;
    }

    // ret = ai_httpclient_voice_send_header(client, url, method, client_data);
    ret = httpclient_send_header(client, url, method, client_data);                      //发送头部信息
    if (ret != 0)
    {
        ALIOT_LOG_ERROR("httpclient_send_header is error,ret = %d", ret);
        return ret;
    }

    if (method == HTTPCLIENT_POST || method == HTTPCLIENT_PUT)
    {   //since voice data maybe very large, we define a new function to send data
        ret = ai_httpclient_voice_send_userdata(client, client_data);                       //从flash里面获取数据并向服务器发送数据
    }

    return ret;
}

int ai_httpclient_voice2text(ai_auth_info_t  *auth_info, httpclient_t *client, char *url, int port, char *ca_crt, int method, httpclient_data_t *client_data)    //音频转文字
{
    int ret = ERROR_HTTP_CONN;
    char *rawdata = NULL;
    int  rawlen = 0;
    char port_str[6] = { 0 };
    char host[HTTPCLIENT_MAX_HOST_LEN] = { 0 };

    httpclient_parse_host(url, host, sizeof(host));
    snprintf(port_str, sizeof(port_str), "%d", port);

    ALIOT_LOG_DEBUG("connect host:%s, port:%s", host, port_str);

    aliyun_iot_net_init(&client->net, host, port_str, ca_crt);

    ret = httpclient_connect(client);    //连接服务器
    if(0 != ret)
    {
        ALIOT_LOG_ERROR("httpclient_connect return ret = [%d]", ret);
        httpclient_close(client);
        return ret;
    }

    //rawdata = client_data->post_buf;
    //rawlen = client_data->post_buf_len;
    if(1)
    {
        rawlen = get_ai_audio_pcm_data_len();   //rawlen   音频存放的地址      连接服务器后一直等待信号量ai_audio_wait的到来
        if(rawlen <= 0)
        {
            ALIOT_LOG_ERROR("get_ai_audio_pcm_data_len get NULL");
            httpclient_close(client);
            return -1;
        }

        client_data->post_buf = (char *)&rawdata; //point to some radom value so that we can send content-length
        client_data->post_buf_len = rawlen;
        //client_data->retrieve_len = len1; //reuse retrieve_len to point to image start
        ret = ai_httpclient_voice_send_request(client, url, method, client_data);          //向服务器发送数据
        if(0 != ret)
        {
            ALIOT_LOG_ERROR("httpclient_send_request is error,ret = %d", ret);
            httpclient_close(client);
            return ret;
        }

        //ret = ai_httpclient_audio_recv_response(client, client_data);
        ret = httpclient_recv_response_content(client, client_data);    //从服务器中接收信息    client_data->response_buf  转换后的文字信息
        if (0 != ret)
        {
            ALIOT_LOG_ERROR("httpclient_recv_response is error,ret = %d", ret);
            httpclient_close(client);
            return ret;
        }
        ALIOT_LOG_ERROR("Voice, content = [%s]\n", client_data->response_buf);
        client_data->post_buf = NULL;
        client_data->response_buf_len = 0;
    }

    httpclient_close(client);

    return ret;
}
#if 0
int32_t baidu_ai_audio_auth(ai_auth_info_t  *auth_info)         //获取token
{
#define HTTP_POST_MAX_LEN   (512)
#define HTTP_RESP_MAX_LEN   (256)
    int ret = -1, length;
    char *post_buf = NULL, *response_buf = NULL;
    //get iot-id and iot-token from response
    int type;
    const char *pvalue = NULL, *presrc = NULL;
    char port_str[6];

    httpclient_t httpclient;
    httpclient_data_t httpclient_data;

    memset(&httpclient, 0, sizeof(httpclient_t));
    memset(&httpclient_data, 0, sizeof(httpclient_data_t));

    post_buf = (char *) aliyun_iot_memory_malloc(HTTP_POST_MAX_LEN);
    if (NULL == post_buf) {
        ALIOT_LOG_ERROR("malloc http post buf failed!");
        return ERROR_MALLOC;
    }
    memset(post_buf, 0, HTTP_POST_MAX_LEN);

    ret = snprintf(post_buf,
                   HTTP_POST_MAX_LEN,
                   "grant_type=client_credentials&client_id=%s&client_secret=%s",
                   auth_info->api_key,
                   auth_info->secret_key);

    if ((ret < 0) || (ret >= HTTP_POST_MAX_LEN)) {
        ALIOT_LOG_ERROR("http message body is too long");
        ret = -1;
        goto do_exit;
    }

    ALIOT_LOG_DEBUG("http content:%s\n\r", post_buf);

    ret = strlen(post_buf);

    response_buf = (char *) aliyun_iot_memory_malloc(HTTP_RESP_MAX_LEN);
    if (NULL == response_buf) {
        ALIOT_LOG_ERROR("malloc http response buf failed!");
        return ERROR_MALLOC;
    }
    memset(response_buf, 0, HTTP_RESP_MAX_LEN);

    httpclient_data.post_content_type = "application/json; charset=UTF-8";
    httpclient_data.post_buf = post_buf;
    httpclient_data.post_buf_len = ret;
    httpclient_data.response_buf = response_buf;
    httpclient_data.response_buf_len = HTTP_RESP_MAX_LEN;

    ret = aliyun_iot_common_post(                      //post 连接网络    发送请求   接收响应
              &httpclient,
              (char *)ai_auth_host,
              443,
              (char *)aliyun_iot_ca_get(),
              &httpclient_data);

    ALIOT_LOG_DEBUG("http response:%s", httpclient_data.response_buf);

    //if(SUCCESS_RETURN == ret)
    if(1)
    {
        pvalue = strstr(httpclient_data.response_buf, "\"access_token\":\"");
        if(pvalue)
        {
            pvalue += strlen("\"access_token\":\"");
            for(length = 0; length < TOKEN_LEN - 1; length++)
            {
                if(pvalue[length] == '\"')
                {
                    break;
                }

                auth_info->token[length] = pvalue[length];
            }

            auth_info->token[length] = '\0';
            ret = SUCCESS_RETURN;
        }
        else
        {
            ret = ERROR_HTTP;
        }

        ALIOT_LOG_DEBUG("BAIDU AI tocken:[%s]\n\r", auth_info->token);
    }

    else
    {
        ALIOT_LOG_DEBUG("BAIDU AI tocken:GET ERROR\n\r");
    }

do_exit:

    if (NULL != post_buf) {
        aliyun_iot_memory_free(post_buf);
    }

    if (NULL != response_buf) {
        aliyun_iot_memory_free(response_buf);
    }

    return ret;
}
#endif
//this is a simple example of voice control of light
void update_light_status(char *text, int len)           //一个简单的声控例程
{   //utf-8 for chinese "kaideng" and "guandeng"
    char openlight_utf8[] = {0xE5, 0xBC, 0x80, 0xE7, 0x81, 0xAF};
    char closelight_utf8[] = {0xE5, 0x85, 0xB3, 0xE7, 0x81, 0xAF};
    int i = 0;
    if(len != sizeof(openlight_utf8))
    {
        return;
    }

    for(i = 0; i < sizeof(openlight_utf8); i++)
    {
        if(openlight_utf8[i] != text[i])
            break;
    }

    if(i == sizeof(openlight_utf8))
    {
        update_relay_status(5, 1);
        return;
    }

    for(i = 0; i < sizeof(closelight_utf8); i++)
    {
        if(closelight_utf8[i] != text[i])
            break;
    }

    if(i == sizeof(closelight_utf8))
    {
        update_relay_status(5, 0);
        return;
    }

}
int32_t baidu_ai_voice2text_query(ai_auth_info_t  *auth_info)     //音频转文字    对文字进行 utf-8编码
{
//#define HTTP_MAX_HOST_LEN   (256)
//#define HTTP_RESP_MAX_LEN   (512)

    int ret = -1, length = 0;//, len1 = 0, len2 = 0;
    char *response_buf = NULL, *pH = NULL;
    //get iot-id and iot-token from response
    //const char *presrc = NULL;
    char ai_host[HTTP_MAX_HOST_LEN] = {0,};
    httpclient_t httpclient;
    httpclient_data_t httpclient_data;

    memset(&httpclient, 0, sizeof(httpclient_t));
    memset(&httpclient_data, 0, sizeof(httpclient_data_t));

    response_buf = (char *) aliyun_iot_memory_malloc(HTTP_RESP_MAX_LEN);
    if (NULL == response_buf) {
        ALIOT_LOG_ERROR("malloc http response buf failed!");
        return ERROR_MALLOC;
    }
    memset(response_buf, 0, HTTP_RESP_MAX_LEN);

    httpclient_data.post_content_type = "audio/pcm;rate=16000";
    httpclient_data.post_buf = NULL;//keep the raw face data, encode with base64 when sending
    httpclient_data.post_buf_len = 0;
    httpclient_data.response_buf = response_buf;
    httpclient_data.response_buf_len = HTTP_RESP_MAX_LEN;

    httpclient.header = "Connection: keep-alive\r\n";
    snprintf(ai_host,
             HTTP_MAX_HOST_LEN,
             "%s?dev_pid=1536&cuid=%s&token=%s",
             ai_voice2text_host, auth_info->userid, auth_info->token);

    ret = ai_httpclient_voice2text(auth_info, &httpclient, (char *)ai_host, 443, (char *)aliyun_iot_ca_get(), HTTPCLIENT_POST, &httpclient_data);    //音频转文字
    //ret = ai_httpclient_voice2text(auth_info, &httpclient, (char *)ai_host, 80, NULL, HTTPCLIENT_POST, &httpclient_data);
    if (NULL != response_buf)                                 //  response_buf  转换后的文字信息
    {
        if(ret == SUCCESS_RETURN)
        {
            pH = strstr(response_buf, "result" );
            if(pH)
            {
                pH += 10;//skip result":["
                length = 0;
                while(*(pH + length) != '"')
                {
                    length++;
                }
                if(length)
                {
                    //uart1_send(pH, length); //display the UTF-8 result
                    send_and_encode_text_event(pH, length);    //send the translated text to queue          //对文字进行   utf-8编码       向队列中发送消息   audio_cfg.ai_queue
                    update_light_status(pH, length);                     //control relay                   //一个简单的声控例程
                }
            }
        }
        aliyun_iot_memory_free(response_buf);
    }

    return ret;
}
aliot_err_t ai_httpclient_text2audio_process_header(char *buf, int *restype, int* conten_tlen)
{
    p_dbg_enter;
    char *ptr = NULL;
    int len = 0;
    ptr = strstr(buf, "Content-Length");
    if(ptr == NULL)
    {
        ALIOT_LOG_ERROR("can not find Content-Length");
        return FAIL_RETURN;
    }

    while(*ptr < '0' || *ptr > '9' ) ptr++;
    while(*ptr >= '0' && *ptr <= '9')
    {
        len = len * 10 + (*ptr - '0');
        ptr++;
    }
    *conten_tlen = len;
    ALIOT_LOG_ERROR("Content-Length = [%d]", len);

    ptr = strstr(buf, "audio/mp3");

    *restype = (ptr == NULL) ? 0 : 1;

    return SUCCESS_RETURN;
}
aliot_err_t ai_httpclient_text2audio_recv_response(httpclient_t *client, httpclient_data_t *client_data)     //文字转语音的  回应的数据进行处理，dac输出
{   p_dbg_enter;
    int recflag = 0, ret = ERROR_HTTP_CONN, recvlen = 0, recvindex = 0;
    char buf[HTTPCLIENT_CHUNK_SIZE] = { 0 };
    char * pH = NULL;
    int content_len = 0, content_type = 0;
    //recflag: 1->already recvd the whole http header
    client_data->response_content_len = 0;
    do
    {
        if (client->net.handle < 0) {
            return ret;
        }

        recvlen = HTTPCLIENT_CHUNK_SIZE - 1 - recvindex;
        ret = client->net.read(&client->net, buf + recvindex, recvlen, 10000);
        if(ret > 0)
        {
            buf[recvindex + ret] = 0;
            ALIOT_LOG_ERROR("recvlen=[%d], ret = [%d]\n", recvlen, ret);
            if(recflag == 0)
            {
                //this is a first data piece recved, we assume it contains http header "Content-Type" and "Content-Length"
                if(content_len == 0)
                {
                    if(ai_httpclient_text2audio_process_header(buf, &content_type, &content_len) != SUCCESS_RETURN)
                    {
                        break;
                    }
                    if(content_type == 0 || content_len <= 0)
                    {
                        break;
                    }
                }

                pH = strstr(buf, "\r\n\r\n");
                if(pH == NULL)
                {
                    ALIOT_LOG_ERROR("NO header ending found\n");
                    if(ret >= 4)
                    {
                        memcpy(buf, &buf[ret - 4], 3);
                        recvindex = 3;
                    }
                }
                else
                {
                    pH += strlen("\r\n\r\n");
                    MUTE_OFF;
                    dac_open();
                    play_mp3_stream((uint8_t*)pH, buf + ret - pH);              //对返回回来的数据进行播放   pH：存放数据的地址
                    recflag = 1;
                    dac_close();
                    MUTE_ON;

                    content_len -= (buf + ret - pH);
                }
            }
            else
            {
                pH = buf;
                MUTE_OFF;
                dac_open();
                play_mp3_stream((uint8_t*)pH, buf + ret - pH);                //对返回回来的数据进行播放   pH：存放数据的地址
                content_len -= (buf + ret - pH);
                dac_close();
                MUTE_ON;
            }

            if(content_len > 0)
            {   //full data get, try one more chance
                continue;
            }
            return SUCCESS_RETURN;
        }
        else if(ret == 0) {
            ALIOT_LOG_ERROR("recv TIMEOUT ret = [%d]\n", ret);
            //return HTTP_RETRIEVE_MORE_DATA;     //timeout, need to read data later
            aliyun_iot_pthread_taskdelay(500);
            //continue;
            break;
        }
        else {
            ALIOT_LOG_ERROR("recv ERROR ret = [%d]\n", ret);
            break;
        }
    } while(1);

    return ret;
}

int ai_httpclient_text2audio(ai_auth_info_t  *auth_info, httpclient_t *client, char *url, int port, char *ca_crt, int method, httpclient_data_t *client_data)     // 文字转语音的请求，响应数据的处理
{
    p_dbg_enter;
    int ret = ERROR_HTTP_CONN;
    //char * post_buf = NULL, *rawdata = NULL;
    //int  length = 0, len1 = 0, len2 = 0, rawlen = 0;
    char port_str[6] = { 0 };
    char host[HTTPCLIENT_MAX_HOST_LEN] = { 0 };

    httpclient_parse_host(url, host, sizeof(host));
    snprintf(port_str, sizeof(port_str), "%d", port);

    ALIOT_LOG_DEBUG("connect host:%s, port:%s", host, port_str);

    aliyun_iot_net_init(&client->net, host, port_str, ca_crt);

    ret = httpclient_connect(client);                                            //连接网络
    if(0 != ret)
    {
        ALIOT_LOG_ERROR("httpclient_connect return ret = [%d]", ret);
        httpclient_close(client);

        return ret;
    }
    ret = ai_httpclient_voice_send_request(client, url, method, client_data);     //文字转语音的请求
    if(0 != ret) {
        ALIOT_LOG_ERROR("httpclient_send_request is error,ret = %d", ret);

        httpclient_close(client);
        return ret;
    }
    printf("\r\n “text to audio”请求成功 \r\n");
    init_mp3_format();                                                   //对一些数据进行空间分配
    reset_mp3_stat();                                                    //复位一些结构体

    ret = ai_httpclient_text2audio_recv_response(client, client_data);     //文字转语音的  回应的数据进行处理，dac输出
    if (0 != ret) {
        ALIOT_LOG_ERROR("httpclient_recv_response is error,ret = %d", ret);
        deinit_mp3_format();
        httpclient_close(client);
        printf("\r\n “text to audio”失败 \r\n");
        return ret;
    }
    printf("\r\n “text to audio”响应成功 \r\n");
    deinit_mp3_format();

    httpclient_close(client);               //关闭连接
    return ret;
}

int32_t baidu_ai_text2audio(ai_auth_info_t  *auth_info, char *data)  //文字转语音     urlcode转语音       data：传入的是 url
{
    p_dbg_enter;

    int ret = -1, index = 0;//, len1 = 0, len2 = 0;
    //char *post_buf = NULL;
    //get iot-id and iot-token from response
    int type = 0; //voice type, default is 0.
    //const char *pvalue = NULL, *presrc = NULL;
    char ai_host[HTTP_MAX_HOST_LEN] = {0,};
    httpclient_t httpclient;
    httpclient_data_t httpclient_data;

    memset(&httpclient, 0, sizeof(httpclient_t));
    memset(&httpclient_data, 0, sizeof(httpclient_data_t));

    //httpclient_data.post_content_type = "audio/pcm;rate=16000";
    httpclient_data.post_buf = NULL;//keep the raw face data, encode with base64 when sending
    httpclient_data.post_buf_len = 0;
    httpclient_data.response_buf = NULL;
    httpclient_data.response_buf_len = 0;

    //check the baidu RESTFul document for voice type
    if(data[0] == '0' || data[0] == '1' || data[0] == '3' || data[0] == '4')
    {
        index = 1;
        type = data[0] - '0';
    }
    //httpclient.header = "Connection: keep-alive\r\n";
    snprintf(ai_host,
             HTTP_MAX_HOST_LEN,
             "%s?tex=%s&lan=zh&cuid=%s&ctp=1&per=%d&tok=%s",
             ai_text2voice_host, data + index, auth_info->userid, type, auth_info->token);

    ret = ai_httpclient_text2audio(auth_info, &httpclient, (char *)ai_host, 443, (char *)aliyun_iot_ca_get(), HTTPCLIENT_GET, &httpclient_data);   // 文字转语音的请求，响应数据的处理
    return ret;
}

#if 0
int32_t ai_audio_get_token(void)     //获取token
{
#define TOCKEN "24.099c21adb4e057716f2f35c762f02bbd.2592000.1527523382.282335-11172081"

    int32_t ret = 0;
    auth_info.api_key = APIKEY;
    auth_info.secret_key = SECRETKEY;
    auth_info.target_face_gid = FACE_GID;
    auth_info.target_face_uid = FACE_UID;
    memset(auth_info.token, 0, TOKEN_LEN);
    memcpy(auth_info.token, TOCKEN, strlen(TOCKEN));

    ret = baidu_ai_audio_auth(&auth_info);        //获取token

    return 0;
    /*
    if(ret == SUCCESS_RETURN)
    {
        ret = baidu_ai_face_verify(&auth_info, (u8_t*)facedata, sizeof(facedata));
    }

    while(1)
    {
        aliyun_iot_pthread_taskdelay(2000);
    }
    return 0;
    */
}
#endif
void ai_voice_task(void* arg)         //语音处理的任务
{
    int32_t ret = 0;
    char * data = 0;
    aliyun_iot_common_log_set_level(ALIOT_LOG_LEVEL_DEBUG);   //觉得打印的信息  debug  info error  等等

    ALIOT_LOG_DEBUG("start ai_voice_task");
    aliyun_iot_pthread_taskdelay(20000);
    do
    {
#if BAIDU_AI_FACE_ENABLE
        if(auth_info.token[0] == 0)
        {
            aliyun_iot_pthread_taskdelay(5000);
            ALIOT_LOG_DEBUG("wait face task to get token");
            continue;
        }
#else
        ret = ai_get_token();     //token  的获取
        if(ret != SUCCESS_RETURN)
        {
            aliyun_iot_pthread_taskdelay(10000);
            ALIOT_LOG_DEBUG("ai_get_token ret=[%d]", ret);
            continue;
        }
#endif
        while(1)
        {   
			p_dbg_enter_task;
            //get text or recording event  from queue
            if( get_voice_ai_event(&data) < 0)     //等待消息队列中数据的到来
            {
                continue;
            }
            //if it is recording event, send audio data to AI server
            if(data[0] == 0)                              //接收到的是音频文件
            {
                ret = baidu_ai_voice2text_query(&auth_info);
                if(ret != SUCCESS_RETURN)
                {
                    ALIOT_LOG_DEBUG("baidu_ai_face_verify ret=[%d]", ret);
                }
            }
            else                                       //接收到的是文字信息
            {   //if it text data, send to text data AI server
                ALIOT_LOG_DEBUG("Get text data [%s]", data);
                ret = baidu_ai_text2audio(&auth_info, data);
                mem_free(data);
            }
            p_dbg_exit_task;
        }
    } while(1);

}
void baidu_ai_voice_client(void)
{
    sys_thread_new("AI audio client", ai_voice_task, 0, TASK_IOT_CLIENT_STK_SIZE, TASK_IOT_CLIENT_PRIO);
}
#endif

#if 0   //demo for https client with mbedtls
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"

#include <string.h>

#define SERVER_PORT "443"
#define SERVER_NAME "www.easynetworking.cn"
#define GET_REQUEST "GET / HTTP/1.1\r\n\r\n"

#define DEBUG_LEVEL 1

static void my_debug( void *ctx, int level,
                      const char *file, int line,
                      const char *str )
{
    ((void) level);
    MBEDTLS_SSL_DEBUG_MSG( NULL, ("%s:%04d: %s", file, line, str ));
}

void https_client_demo(void* arg)
{
    int ret, len;
    mbedtls_net_context server_fd;
    uint32_t flags;
    unsigned char buf[1024];
    const char *pers = "ssl_client1";

    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_x509_crt cacert;

#if defined(MBEDTLS_DEBUG_C)
    mbedtls_debug_set_threshold( DEBUG_LEVEL );
#endif

    /*
     * 0. Initialize the RNG and the session data
     */
    mbedtls_net_init( &server_fd );
    mbedtls_ssl_init( &ssl );
    mbedtls_ssl_config_init( &conf );
    mbedtls_x509_crt_init( &cacert );
    mbedtls_ctr_drbg_init( &ctr_drbg );

    MBEDTLS_SSL_DEBUG_MSG( NULL, ("\n  . Seeding the random number generator..." ));

    mbedtls_entropy_init( &entropy );
    if( ( ret = mbedtls_ctr_drbg_seed( &ctr_drbg, mbedtls_entropy_func, &entropy,
                                       (const unsigned char *) pers,
                                       strlen( pers ) ) ) != 0 )
    {
        MBEDTLS_SSL_DEBUG_MSG( NULL, ( " failed\n  ! mbedtls_ctr_drbg_seed returned %d\n", ret ));
        goto exit;
    }

    MBEDTLS_SSL_DEBUG_MSG(NULL,  (" ok\n" ));

    /*
     * 0. Initialize certificates
     */
    MBEDTLS_SSL_DEBUG_MSG( NULL, ("  . Loading the CA root certificate ...") );

    ret = mbedtls_x509_crt_parse( &cacert, (const unsigned char *) mbedtls_test_cas_pem,
                                  mbedtls_test_cas_pem_len );
    if( ret < 0 )
    {
        MBEDTLS_SSL_DEBUG_MSG( NULL,  (" failed\n  !  mbedtls_x509_crt_parse returned -0x%x\n\n", -ret ));
        goto exit;
    }

    MBEDTLS_SSL_DEBUG_MSG( NULL,  (" ok (%d skipped)\n", ret) );

    /*
     * 1. Start the connection
     */
    MBEDTLS_SSL_DEBUG_MSG( NULL,  ("  . Connecting to tcp/%s/%s...", SERVER_NAME, SERVER_PORT) );
    if( ( ret = mbedtls_net_connect( &server_fd, SERVER_NAME,
                                     SERVER_PORT, MBEDTLS_NET_PROTO_TCP ) ) != 0 )
    {
        MBEDTLS_SSL_DEBUG_MSG( NULL,  (" failed\n  ! mbedtls_net_connect returned %d\n\n", ret ));
        goto exit;
    }

    MBEDTLS_SSL_DEBUG_MSG( NULL,  (" ok\n" ));

    /*
     * 2. Setup stuff
     */
    MBEDTLS_SSL_DEBUG_MSG( NULL,  ("  . Setting up the SSL/TLS structure..." ));
    if( ( ret = mbedtls_ssl_config_defaults( &conf,
                MBEDTLS_SSL_IS_CLIENT,
                MBEDTLS_SSL_TRANSPORT_STREAM,
                MBEDTLS_SSL_PRESET_DEFAULT ) ) != 0 )
    {
        MBEDTLS_SSL_DEBUG_MSG( NULL,  (" failed\n  ! mbedtls_ssl_config_defaults returned %d\n\n", ret) );
        goto exit;
    }

    MBEDTLS_SSL_DEBUG_MSG( NULL,  (" ok\n" ));

    /* OPTIONAL is not optimal for security,
     * but makes interop easier in this simplified example */
    mbedtls_ssl_conf_authmode( &conf, MBEDTLS_SSL_VERIFY_OPTIONAL );
    mbedtls_ssl_conf_ca_chain( &conf, &cacert, NULL );
    mbedtls_ssl_conf_rng( &conf, mbedtls_ctr_drbg_random, &ctr_drbg );
    mbedtls_ssl_conf_dbg( &conf, my_debug, NULL );

    if( ( ret = mbedtls_ssl_setup( &ssl, &conf ) ) != 0 )
    {
        MBEDTLS_SSL_DEBUG_MSG( NULL,  (" failed\n  ! mbedtls_ssl_setup returned %d\n\n", ret ));
        goto exit;
    }

    if( ( ret = mbedtls_ssl_set_hostname( &ssl, SERVER_NAME ) ) != 0 )
    {
        MBEDTLS_SSL_DEBUG_MSG( NULL, ( " failed\n  ! mbedtls_ssl_set_hostname returned %d\n\n", ret ));
        goto exit;
    }

    mbedtls_ssl_set_bio( &ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL );

    /*
     * 4. Handshake
     */
    MBEDTLS_SSL_DEBUG_MSG( NULL,  ("  . Performing the SSL/TLS handshake...\n" ));

    while( ( ret = mbedtls_ssl_handshake( &ssl ) ) != 0 )
    {
        if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
        {
            MBEDTLS_SSL_DEBUG_MSG( NULL,  (" failed\n  ! mbedtls_ssl_handshake returned -0x%x\n\n", -ret ));
            goto exit;
        }
    }

    MBEDTLS_SSL_DEBUG_MSG(NULL,  (" ok\n" ));

    /*
     * 5. Verify the server certificate
     */
    MBEDTLS_SSL_DEBUG_MSG( NULL,  ("  . Verifying peer X.509 certificate..." ));

    /* In real life, we probably want to bail out when ret != 0 */
    if( ( flags = mbedtls_ssl_get_verify_result( &ssl ) ) != 0 )
    {
        char vrfy_buf[512];

        MBEDTLS_SSL_DEBUG_MSG( NULL,  (" failed\n" ));

        mbedtls_x509_crt_verify_info( vrfy_buf, sizeof( vrfy_buf ), "  ! ", flags );

        MBEDTLS_SSL_DEBUG_MSG( NULL,  ("%s\n", vrfy_buf ));
    }
    else
        MBEDTLS_SSL_DEBUG_MSG( NULL,  (" ok\n" ));

    /*
     * 3. Write the GET request
     */
    MBEDTLS_SSL_DEBUG_MSG( NULL,  ("  > Write to server:" ));
    len = sprintf( (char *) buf, GET_REQUEST );

    while( ( ret = mbedtls_ssl_write( &ssl, buf, len ) ) <= 0 )
    {
        if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
        {
            MBEDTLS_SSL_DEBUG_MSG( NULL,  (" failed\n  ! mbedtls_ssl_write returned %d\n\n", ret) );
            goto exit;
        }
    }

    len = ret;
    MBEDTLS_SSL_DEBUG_MSG( NULL,  (" %d bytes written\n\n%s", len, (char *) buf) );

    /*
     * 7. Read the HTTP response
     */
    MBEDTLS_SSL_DEBUG_MSG( NULL,  ("  < Read from server:" ));
    do
    {
        len = sizeof( buf ) - 1;
        memset( buf, 0, sizeof( buf ) );
        ret = mbedtls_ssl_read( &ssl, buf, len );

        if( ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE )
            continue;

        if( ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY )
            break;

        if( ret < 0 )
        {
            MBEDTLS_SSL_DEBUG_MSG( NULL,  ("failed\n  ! mbedtls_ssl_read returned %d\n\n", ret ));
            break;
        }

        if( ret == 0 )
        {
            MBEDTLS_SSL_DEBUG_MSG( NULL,  ("\n\nEOF\n\n" ));
            break;
        }

        len = ret;
        MBEDTLS_SSL_DEBUG_MSG( NULL,  (" %d bytes read:\n\n%s", len, (char *) buf ));
    }
    while( 1 );

    mbedtls_ssl_close_notify( &ssl );

exit:

#ifdef MBEDTLS_ERROR_C
    if( ret != 0 )
    {
        char error_buf[100];
        mbedtls_strerror( ret, error_buf, 100 );
        MBEDTLS_SSL_DEBUG_MSG( NULL,  ("Last error was: %d - %s\n\n", ret, error_buf ));
    }
#endif

    mbedtls_net_free( &server_fd );

    mbedtls_x509_crt_free( &cacert );
    mbedtls_ssl_free( &ssl );
    mbedtls_ssl_config_free( &conf );
    mbedtls_ctr_drbg_free( &ctr_drbg );
    mbedtls_entropy_free( &entropy );

    while(1) aliyun_iot_pthread_taskdelay(200);

    return;
}
#endif

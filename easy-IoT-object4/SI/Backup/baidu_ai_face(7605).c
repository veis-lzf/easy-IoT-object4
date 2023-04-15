/***************************************
* face recongnition based on Baidu AI platform
* control the devie(a light) if the face matchs
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
#include <debug.h>

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
#include "baidu_ai_face.h"
#include "camera.h"
#include "drivers.h"

//#define APIKEY "OWU3mDwGpFDDFmFilRSWUDEg"
//#define SECRETKEY "bZYgXv4UjU6kGH5WCd1vikuW38L6XMG4"

#define APIKEY "nDFoAGd7oRro6gUG6GQn82Wa"
#define SECRETKEY "DKaFaSLY7kFa3DNqHNNFear6u87CGaC3"

#define FACE_GID "test_group_1"
#define FACE_UID "test_user_1"

const static char *ai_atuh_host = "https://aip.baidubce.com/oauth/2.0/token";
const static char *ai_verify_host = "https://aip.baidubce.com/rest/2.0/image-classify/v2/advanced_general";

ai_auth_info_t auth_info;

//iamge data get by camera
extern int CAMERA_MODE;
extern uint8_t *image_data;
extern int  cemera_rawlen ;
extern wait_event_t client_pic_event;
extern int camera_mode( int  CAMERA_MODE);
extern void netcamera_task(void );
wait_event_t client_camera_event = NULL;            //�Ƿ��������ʶ��ģʽ

extern aliot_err_t aliyun_iot_common_base64encode(const uint8_t *data, uint32_t inputLength, uint32_t outputLenMax,uint8_t *encodedData,uint32_t *outputLength);
extern int32_t baidu_ai_text2audio(ai_auth_info_t  *auth_info, char *data); // ����ת����

NETCAMERA_HandleTypeDef   my_netcemera;
//''��ʼʶ��''utf8���벢תurlcode��ֵ
//http://www.mytju.com/classcode/tools/encode_utf8.asp
char *beginhint = "%E5%BC%80%E5%A7%8B%E8%AF%86%E5%88%AB\0";

//urlencode a utf8 string
u8_t * url_encode_utf8_text(char * data, int len)                             //utf8����  תurlcode��ֵ
{
    int i = 0, flag = 0, ret = 0;
    u8_t ch;
    u8_t *uart_url_data = NULL;
    char hex[] = "0123456789ABCDEF";
    ALIOT_LOG_INFO("url_encode_utf8_text len: [%d]", len);

    uart_url_data = mem_calloc(3, len + 1);
    if(uart_url_data == NULL)    return NULL;
    //switch utf-8 to urlencode here

    for(i = 0; i < len; i++)
    {
        ch = data[i];
        if((ch >= '0' && ch <= '9') ||
                (ch >= 'a' && ch <= 'z') ||
                (ch >= 'A' && ch <= 'Z') ||
                ch == '-' || ch == '_' || ch == '.' || ch == '~')
        {
            uart_url_data[flag++] = ch;
        }
        else if (ch == ' ')
        {
            uart_url_data[flag++] = '+';
        }
        else
        {
            uart_url_data[flag++] += '%';
            uart_url_data[flag++] += hex[ch / 16];
            uart_url_data[flag++] += hex[ch % 16];
        }
    }

    uart_url_data[flag++] = 0;
    ALIOT_LOG_INFO("url_encode_utf8_text urlencode:%s", uart_url_data);

    return uart_url_data;

}

int32_t baidu_ai_auth(ai_auth_info_t  *auth_info)
{
#define HTTP_POST_MAX_LEN   (512)
#define HTTP_RESP_MAX_LEN   (512)
    int ret = -1, length;
    char *post_buf = NULL, *response_buf = NULL;
    //get iot-id and iot-token from response
    //
    const char *pvalue = NULL;
    //char port_str[6];

    httpclient_t httpclient;
    httpclient_data_t httpclient_data;

    memset(&httpclient, 0, sizeof(httpclient_t));

    memset(&httpclient_data, 0, sizeof(httpclient_data_t));


    post_buf = (char *) aliyun_iot_memory_malloc(HTTP_POST_MAX_LEN);   //��post�������ݰ�����һ���ռ�
    if (NULL == post_buf) {
        ALIOT_LOG_ERROR("malloc http post buf failed!");
        printf("\r\n    ����ռ����ʧ��   \r\n");
        return ERROR_MALLOC;
    }
    memset(post_buf, 0, HTTP_POST_MAX_LEN);  //��post_buf��οռ���ڴ�ȫ��д��0

    ret = snprintf(post_buf,    //��post���ݵĲ������������ɣ�grant_type   client_id  client_secret ��
                   HTTP_POST_MAX_LEN,
                   "grant_type=client_credentials&client_id=%s&client_secret=%s",
                   auth_info->api_key,
                   auth_info->secret_key);  //���ɱ������(...)����format��ʽ�����ַ�����Ȼ���临�Ƶ�str��


    if ((ret < 0) || (ret >= HTTP_POST_MAX_LEN)) {              //���HTTP�������û�����
        ALIOT_LOG_ERROR("http message body is too long");
        ret = -1;
        printf("\r\n    post�������ʧ��   \r\n");
        goto do_exit;
    }

    ALIOT_LOG_DEBUG("http content:%s\n\r", post_buf);  //��ӡ��־����������

    ret = strlen(post_buf);

    response_buf = (char *) aliyun_iot_memory_malloc(HTTP_RESP_MAX_LEN);
    if (NULL == response_buf) {
        ALIOT_LOG_ERROR("malloc http response buf failed!");
        printf("\r\n    ��Ӧ�ռ����ʧ��   \r\n");
        return ERROR_MALLOC;
    }
    memset(response_buf, 0, HTTP_RESP_MAX_LEN);

    httpclient_data.post_content_type = "application/json; charset=UTF-8";
    httpclient_data.post_buf = post_buf;
    httpclient_data.post_buf_len = ret;
    httpclient_data.response_buf = response_buf;
    httpclient_data.response_buf_len = HTTP_RESP_MAX_LEN;

    ret = aliyun_iot_common_post(      //  post  ��������    ��������   ������Ӧ
              &httpclient,
              (char *)ai_atuh_host,    //"https://aip.baidubce.com/oauth/2.0/token";
              443,
              (char *)aliyun_iot_ca_get(),  //��ȡca_crtb��CA֤�飩
              &httpclient_data);

    ALIOT_LOG_DEBUG("http response:%s", httpclient_data.response_buf);  //��ӡ��־����Ӧ����

    //if(SUCCESS_RETURN == ret)
    if(1)
    {
        pvalue = strstr(httpclient_data.response_buf, "\"access_token\":\"");

        if(pvalue) {
            pvalue += strlen("\"access_token\":\"");
            for(length = 0; length < TOKEN_LEN - 1; length++) {
                if(pvalue[length] == '\"') {
                    break;
                }
                auth_info->token[length] = pvalue[length];   //��tokenд��
            }

            auth_info->token[length] = '\0';
            ret = SUCCESS_RETURN;
            printf("\r\n    token��ϳɹ�   \r\n");
        }
        else
        {   printf("\r\n   ERROR_HTTP   \r\n");
            ret = ERROR_HTTP;
            printf("\r\n  ret==%d \r\n", ret);
        }

        ALIOT_LOG_DEBUG("BAIDU AI tocken:[%s]\n\r", auth_info->token);  //��ӡ��־��token
    }

    else
    {
        ALIOT_LOG_DEBUG("BAIDU AI tocken:GET ERROR\n\r");      //��ӡ��־��GETʧ��
    }

do_exit:  //HTTP�������û����ɣ����ͷſռ�

    if (NULL != post_buf) {
        aliyun_iot_memory_free(post_buf);
    }

    if (NULL != response_buf) {
        aliyun_iot_memory_free(response_buf);
    }

    return ret;
}

int32_t ai_get_token(void)      //��ȡtoken
{
    //#define TOCKEN "24.c806f1a74fd72e4f51c2460770c506f4.2592000.1526486235.282335-11097443"
    p_dbg_enter;
#define CPU_ID              ((unsigned int*)0x1fff7a10)
    int32_t ret = 0;
    auth_info.api_key = APIKEY;
    auth_info.secret_key = SECRETKEY;
    auth_info.target_face_gid = FACE_GID;
    auth_info.target_face_uid = FACE_UID;
    memset(auth_info.token, 0, TOKEN_LEN);
    memset(auth_info.userid, 0, TOKEN_LEN);
    //memcpy(auth_info.token, TOCKEN, strlen(TOCKEN));

    snprintf(auth_info.userid, TOKEN_LEN, "baidu_ai_user_%x", *CPU_ID);    //�����ߵ�IDд��


    ret = baidu_ai_auth(&auth_info);                  //��ȡtoken

    return ret;
}
#if BAIDU_AI_FACE_ENABLE

int ai_image_httpclient_send_header(httpclient_t *client, char *url, int method, httpclient_data_t *client_data)                           //����ͷ��
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
        i = client_data->retrieve_len;
        ext_len = client_data->post_buf_len;
        //calculate again for the Content-Length, since the image data should be urlencode
        while(i < client_data->post_buf_len)
        {
            if(client_data->post_buf[i] == '/' || client_data->post_buf[i] == '+' || client_data->post_buf[i] == '=')
            {
                ext_len += 2;
            }

            i++;
        }
        snprintf(buf, sizeof(buf), "Content-Length: %d\r\n", ext_len);
        httpclient_get_info(client, send_buf, &len, buf, strlen(buf));

        snprintf(buf, sizeof(buf), "Connection: keep-alive\r\n");
        httpclient_get_info(client, send_buf, &len, buf, strlen(buf));

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
    if (ret > 0) {
        ALIOT_LOG_DEBUG("Written %d bytes", ret);
    }
    else if (ret == 0) {
        ALIOT_LOG_ERROR("ret == 0,Connection was closed by server");
        return ERROR_HTTP_CLOSED; /* Connection was closed by server */
    }
    else {
        ALIOT_LOG_ERROR("Connection error (send returned %d)", ret);
        return ERROR_HTTP_CONN;
    }

    return SUCCESS_RETURN;
}

int ai_image_httpclient_send_userdata(httpclient_t *client, httpclient_data_t *client_data)      //�������д������
{
    int ret = 0;
    int len = 0, send_len = 0, image_index = 0;
#define MAX_SEND_LEN 476
    char senddata[MAX_SEND_LEN + 4];
    char *rawdata = NULL;

    if (client_data->post_buf && client_data->post_buf_len) {
        rawdata = client_data->post_buf;
        image_index = client_data->retrieve_len;
        while(len < client_data->post_buf_len)
        {
            //convert the picture to urlcode, for base64 picture, only contains '/','+','=' that should be converted.
            if(len < image_index) {
                senddata[send_len++] = rawdata[len];
            }
            else if(rawdata[len] == '/') {
                senddata[send_len++] = '%';
                senddata[send_len++] = '2';
                senddata[send_len++] = 'F';
            }
            else if(rawdata[len] == '+') {
                senddata[send_len++] = '%';
                senddata[send_len++] = '2';
                senddata[send_len++] = 'B';
            }
            else if(rawdata[len] == '=') {
                senddata[send_len++] = '%';
                senddata[send_len++] = '3';
                senddata[send_len++] = 'D';
            }
            else {
                senddata[send_len++] = rawdata[len];
            }

            len += 1;
            if( len == client_data->post_buf_len || send_len >= MAX_SEND_LEN) {

                ret = client->net.write(&client->net, senddata, send_len, 5000);     //�������д������
                if (ret > 0) {
                    ALIOT_LOG_DEBUG("Written %d bytes", ret);
                    send_len = 0;
                }
                else if (ret == 0) {
                    ALIOT_LOG_ERROR("ret == 0,Connection was closed by server");
                    return ERROR_HTTP_CLOSED; /* Connection was closed by server */
                }
                else {
                    ALIOT_LOG_ERROR("Connection error (send returned %d)", ret);
                    return ERROR_HTTP_CONN;
                }
            }
        }
    }
    return SUCCESS_RETURN;
}

aliot_err_t ai_image_httpclient_send_request(httpclient_t *client, char *url, int method, httpclient_data_t *client_data)     //����ͷ��      �������д������
{
    int ret = ERROR_HTTP_CONN;

    if (client->net.handle < 0)
    {
        return ret;
    }

    ret = ai_image_httpclient_send_header(client, url, method, client_data);               //����ͷ��
    if (ret != 0)
    {
        ALIOT_LOG_ERROR("httpclient_send_header is error,ret = %d", ret);
        return ret;
    }

    if (method == HTTPCLIENT_POST || method == HTTPCLIENT_PUT)
    {
        ret = ai_image_httpclient_send_userdata(client, client_data);                      //�������д������
    }

    return ret;
}

void play_image_recog_result(char *result)   // ��ͼƬ��Ϣ���в���
{
    char *num = strstr(result, "result"); // ���ұ�ʶ�ַ���
    char *key = NULL;
	char *data = NULL;
    int value = 0;
    do
    {
        if(num == NULL) { // û���ҵ����ݷ��صĽ����ʼ��ʶ
            break;
        }
        num += strlen("result\":[");
        if(num == NULL || *num == 0) {
            break;
        }

        key = strstr(num, "keyword");
        if(key)
        {
            key += strlen("keyword\":");
            while(*key != '"') key++;
            key++;
            while(*(key + value) != '"') value++;

            //send_and_encode_text_event(key, value);
            printf("\r\n���ǣ�%s\r\n", key);

            data = (char *)url_encode_utf8_text(key, value);         //utf8���벢תurlcode��ֵ    data��  urlcode������ֵ
            if(data) {
                baidu_ai_text2audio(&auth_info, data);           //����ת����    �������data��  urlcode������ֵ
                mem_free(data);
            }
        }

        return;
    } while(0);

    return;
}
int ai_image_httpclient_common(ai_auth_info_t  *auth_info, httpclient_t *client, char *url, int port, char *ca_crt, int method, httpclient_data_t *client_data)
{
    int ret = ERROR_HTTP_CONN;
    int b;
    char * post_buf = NULL, *rawdata = NULL;
    int  length = 0, len1 = 0, len2 = 0, rawlen = 0;
    char port_str[6] = { 0 };
    char host[HTTPCLIENT_MAX_HOST_LEN] = { 0 };

    httpclient_parse_host(url, host, sizeof(host));
    snprintf(port_str, sizeof(port_str), "%d", port);

    ALIOT_LOG_DEBUG("connect host:%s, port:%s", host, port_str);

    aliyun_iot_net_init(&client->net, host, port_str, ca_crt);

    ret = httpclient_connect(client);
    if(0 != ret)
    {
        ALIOT_LOG_ERROR("httpclient_connect return ret = [%d]", ret);
        httpclient_close(client);
        printf("\r\n ****���Ӵ��� *****\r\n");
        return ret;
    }
    IND2_ON;
    do
    {
        printf("\r\n ****ʶ���ӳɹ����ȴ�ͼƬ���ݵĵ��� *****\r\n");
        b = wait_event_timeout(client_camera_event, 0);  //�����ȴ��ź���   ����ͷģʽ   ��������  Ĭ��Ϊlcd��ʾģʽ
        netcamera_task();

		 //ͼƬû�е���
        if(image_data == NULL) {
            ALIOT_LOG_ERROR("get NULL face picture");
            aliyun_iot_pthread_taskdelay(4000);
            printf("\r\n ****û�н��յ���ͼƬ *****\r\n");
            break;
        }
        IND2_OFF;
        rawlen = cemera_rawlen;
        rawdata = (char *)(image_data );

        if(rawdata) {
            length = rawlen * 4 / 3 + 64;
            //now the https is connected, we can allocate memory for base64 picture
            post_buf = (char *) mem_malloc(length);
            if (NULL == post_buf) {
                ALIOT_LOG_ERROR("malloc http post buf len [%d]failed!", length);
                ALIOT_LOG_ERROR("mem pool1 remain:%d/%d", mem_get_free(), mem_get_size());
                ALIOT_LOG_ERROR("mem pool2 remain:%d/%d", mem_get_free2(), mem_get_size2());
                ret = -1;
                break;
            }

            memset(post_buf, 0, length);
            //len1 = snprintf(post_buf, length, "top_num=1&uid=%s&images=", auth_info->target_face_uid);
            len1 = snprintf(post_buf, length, "image=");
            if ((len1 < 0) || (len1 >= length)) {
                ALIOT_LOG_ERROR("http message body is too long");
                ret = -2;
                break;
            }

            ret = aliyun_iot_common_base64encode(rawdata, rawlen, length - len1, &post_buf[len1], &len2);   //base64encode����
            if ((ret < 0) ) {
                ALIOT_LOG_ERROR("base64encode error");
                ret = -3;
                break;
            }

            client_data->post_buf = post_buf;
            client_data->post_buf_len = len1 + len2;
            client_data->retrieve_len = len1; //reuse retrieve_len to point to image start
        }

        ret = ai_image_httpclient_send_request(client, url, method, client_data);   //����ͼƬ������ٶ�ƽ̨   ��������  ��ͷ��+���ݣ�  ����������
        if(0 != ret) {
            ALIOT_LOG_ERROR("httpclient_send_request is error,ret = %d", ret);
            break;
        }

        //ret = ai_httpclient_recv_response(client, client_data);
        ret = httpclient_recv_response_content(client, client_data);      //������Ӧ�������   client_data
        if (0 != ret) {
            ALIOT_LOG_ERROR("httpclient_recv_response is error,ret = %d", ret);
            break;
        }
        ALIOT_LOG_ERROR("IMAGE RECONG, result = [%s]", client_data->response_buf);
    } while(0);

    printf("\r\n ****�ͷ��ڴ� *****\r\n");
    printf("mem pool1 remain:%d/%dbytes\r\n", mem_get_free(), mem_get_size());
    printf("mem pool2 remain:%d/%dbytes\r\n", mem_get_free2(), mem_get_size2());

    mem_free(post_buf);
    client_data->post_buf = NULL;
    client_data->response_buf_len = 0;

    mem_free(image_data);
    image_data = NULL;
    cemera_rawlen = 0;

    printf("mem pool1 remain:%d/%dbytes\r\n", mem_get_free(), mem_get_size());
    printf("mem pool2 remain:%d/%dbytes\r\n", mem_get_free2(), mem_get_size2());
    httpclient_close(client);

    aliyun_iot_pthread_taskdelay(1000);
    return ret;
}

int32_t baidu_ai_image_verify(ai_auth_info_t  *auth_info)         //ͼ��ʶ��
{

    int ret = -1, length = 0;//, len1 = 0, len2 = 0;
    char *post_buf = NULL, *response_buf = NULL;
    //get iot-id and iot-token from response
    //int type = 0;
    //const char *pvalue = NULL, *presrc = NULL;
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

    httpclient_data.post_content_type = "application/x-www-form-urlencoded";
    httpclient_data.post_buf = NULL;//keep the raw face data, encode with base64 when sending
    httpclient_data.post_buf_len = 0;
    httpclient_data.response_buf = response_buf;
    httpclient_data.response_buf_len = HTTP_RESP_MAX_LEN;

    snprintf(ai_host, HTTP_MAX_HOST_LEN, "%s?access_token=%s", ai_verify_host, auth_info->token);

    ret = ai_image_httpclient_common(auth_info, &httpclient, (char *)ai_host, 443, (char *)aliyun_iot_ca_get(), HTTPCLIENT_POST, &httpclient_data);  //����ƽ̨�Ͷ�ƽ̨��Ӧ�����ݴ���

    if(ret == 0)
    {
        play_image_recog_result(response_buf);    //��ͼƬ��Ϣ���в���
        aliyun_iot_pthread_taskdelay(1000);
    }
    if (NULL != response_buf)
    {
        aliyun_iot_memory_free(response_buf);            //�Խ��տռ�ĵ��ͷ�
    }
    return ret;
}



//wait_event_t client_camera_mode_event = NULL;       //lcdģʽ�����ϴ�ģʽ

void ai_image_task(void* arg)
{
    int32_t ret = 0, b = 1;
    aliyun_iot_common_log_set_level(ALIOT_LOG_LEVEL_INFO);   //g_iotLogLevel==1;
    ALIOT_LOG_INFO("start ai_image_task");                    //��־
    aliyun_iot_pthread_taskdelay(10000);        //˯����ʱ  Ҳ���ǰѸ�����������������Ӧ��ʱ��
    client_camera_event = init_event();         //����һ����ֵ�ź���    �Ƿ��������ʶ��ģʽ
    do
    {
        ret = ai_get_token();//��ȡtoken
        if(ret != SUCCESS_RETURN) {
            aliyun_iot_pthread_taskdelay(10000);
            ALIOT_LOG_ERROR("ai_get_token ret=[%d]", ret);
            printf("\r\n token��ȡʧ�� \r\n");
            continue;
        }
		
        while(1) {
            p_dbg_enter_task;
            ret = baidu_ai_image_verify(&auth_info);      //ͼ��ʶ��
            if(ret != SUCCESS_RETURN) {
                aliyun_iot_pthread_taskdelay(5000);    //˯����ʱ  Ҳ���ǰѸ�����������������Ӧ��ʱ��
                ALIOT_LOG_ERROR("baidu_ai_image_verify ret=[%d]", ret);
                printf("ͼ��ʶ��ʧ��\r\n");
            }
            aliyun_iot_pthread_taskdelay(2000);    //˯����ʱ  Ҳ���ǰѸ�����������������Ӧ��ʱ��
            p_dbg_exit_task;
        }
    } while(1);

}
void baidu_ai_image_client(void)
{
    sys_thread_new("baidu AI client", ai_image_task, 0, TASK_AI_CLIENT_STK_SIZE, TASK_AI_CLIENT_PRIO);
}
#endif


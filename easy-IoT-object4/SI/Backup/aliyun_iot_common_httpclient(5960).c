/* Copyright (C) 2012 mbed.org, MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
 #define DEBUG
#include <string.h>
#include "aliyun_iot_platform_datatype.h"
#include "aliyun_iot_common_error.h"
#include "aliyun_iot_common_log.h"
#include "aliyun_iot_common_httpclient.h"
#include "aliyun_iot_platform_network.h"
#include "aliyun_iot_common_net.h"
#include "debug.h"
#if 0
#define HTTPCLIENT_MIN(x,y) (((x)<(y))?(x):(y))
#define HTTPCLIENT_MAX(x,y) (((x)>(y))?(x):(y))

#define HTTPCLIENT_AUTHB_SIZE     128

#define HTTPCLIENT_CHUNK_SIZE     512
#define HTTPCLIENT_SEND_BUF_SIZE  512

#define HTTPCLIENT_MAX_HOST_LEN   64
#define HTTPCLIENT_MAX_URL_LEN    512

#if defined(MBEDTLS_DEBUG_C)
#define DEBUG_LEVEL 2
#endif

static int httpclient_parse_host(char *url, char *host, uint32_t maxhost_len);
static int httpclient_parse_url(const char *url, char *scheme, uint32_t max_scheme_len, char *host, uint32_t maxhost_len, int *port, char *path, uint32_t max_path_len);
static int httpclient_tcp_send_all(int sock_fd, char *data, int length);
static int httpclient_conn(httpclient_t *client);
static int httpclient_recv(httpclient_t *client, char *buf, int min_len, int max_len, int *p_read_len);
static int httpclient_retrieve_content(httpclient_t *client, char *data, int len, httpclient_data_t *client_data);
static int httpclient_response_parse(httpclient_t *client, char *data, int len, httpclient_data_t *client_data);
#endif
static void httpclient_base64enc(char *out, const char *in)
{
    const char code[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
    int i = 0, x = 0, l = 0;

    for (; *in; in++)
    {
        x = x << 8 | *in;
        for (l += 8; l >= 6; l -= 6)
        {
            out[i++] = code[(x >> (l - 6)) & 0x3f];
        }
    }
    if (l > 0)
    {
        x <<= 6 - l;
        out[i++] = code[x & 0x3f];
    }
    for (; i % 4;)
    {
        out[i++] = '=';
    }
    out[i] = '\0';
}

int httpclient_conn(httpclient_t *client)
{
    
    p_dbg_enter;
    if(0 != client->net.connect(&client->net))
    {
        ALIOT_LOG_ERROR("httpclient_conn error");
        return ERROR_HTTP_CONN;
    }
    p_dbg_exit;
    return SUCCESS_RETURN;
}

int httpclient_parse_url(const char *url, char *scheme, uint32_t max_scheme_len, char *host, uint32_t maxhost_len, int *port, char *path, uint32_t max_path_len)
{
    char *scheme_ptr = (char *) url;
    char *host_ptr = (char *) strstr(url, "://");
    uint32_t host_len = 0;
    uint32_t path_len;
    //char *port_ptr;
    char *path_ptr;
    char *fragment_ptr;

    if (host_ptr == NULL)
    {
        ALIOT_LOG_ERROR("Could not find host");
        return ERROR_HTTP_PARSE; /* URL is invalid */
    }

    if (max_scheme_len < host_ptr - scheme_ptr + 1)
    { /* including NULL-terminating char */
        ALIOT_LOG_ERROR("Scheme str is too small (%u >= %u)", max_scheme_len, (uint32_t)(host_ptr - scheme_ptr + 1));
        return ERROR_HTTP_PARSE;
    }
    memcpy(scheme, scheme_ptr, host_ptr - scheme_ptr);
    scheme[host_ptr - scheme_ptr] = '\0';

    host_ptr += 3;

    *port = 0;
	
    path_ptr = strchr(host_ptr, '/');
    if (host_len == 0)
    {
        host_len = path_ptr - host_ptr;
    }

    if (maxhost_len < host_len + 1)
    { /* including NULL-terminating char */
        ALIOT_LOG_ERROR("Host str is too small (%d >= %d)", maxhost_len, host_len + 1);
        return ERROR_HTTP_PARSE;
    }
    memcpy(host, host_ptr, host_len);
    host[host_len] = '\0';

    fragment_ptr = strchr(host_ptr, '#');
    if (fragment_ptr != NULL)
    {
        path_len = fragment_ptr - path_ptr;
    }
    else
    {
        path_len = strlen(path_ptr);
    }

    if (max_path_len < path_len + 1)
    { /* including NULL-terminating char */
        ALIOT_LOG_ERROR("Path str is too small (%d >= %d)", max_path_len, path_len + 1);
        return ERROR_HTTP_PARSE;
    }
    memcpy(path, path_ptr, path_len);
    path[path_len] = '\0';

    return SUCCESS_RETURN;
}

int httpclient_parse_host(char *url, char *host, uint32_t maxhost_len)
{
    char *host_ptr = (char *) strstr(url, "://");
    uint32_t host_len = 0;
    char *path_ptr;

    if (host_ptr == NULL)
    {
        ALIOT_LOG_ERROR("Could not find host");
        return ERROR_HTTP_PARSE; /* URL is invalid */
    }
    host_ptr += 3;

    path_ptr = strchr(host_ptr, '/');
    if (host_len == 0)
    {
        host_len = path_ptr - host_ptr;
    }

    if (maxhost_len < host_len + 1)
    { /* including NULL-terminating char */
        ALIOT_LOG_ERROR("Host str is too small (%d >= %d)", maxhost_len, host_len + 1);
        return ERROR_HTTP_PARSE;
    }
    memcpy(host, host_ptr, host_len);
    host[host_len] = '\0';

    return SUCCESS_RETURN;
}

int httpclient_get_info(httpclient_t *client, char *send_buf, int *send_idx, char *buf, uint32_t len) /* 0 on success, err code on failure */
{
    int ret;
    int cp_len;
    int idx = *send_idx;

    if (len == 0)
    {
        len = strlen(buf);
    }

    do
    {
        if ((HTTPCLIENT_SEND_BUF_SIZE - idx) >= len)
        {
            cp_len = len;
        }
        else
        {
            cp_len = HTTPCLIENT_SEND_BUF_SIZE - idx;
        }

        memcpy(send_buf + idx, buf, cp_len);
        idx += cp_len;
        len -= cp_len;

        if (idx == HTTPCLIENT_SEND_BUF_SIZE)
        {
//            if (client->remote_port == HTTPS_PORT)
//            {
//                WRITE_IOT_ERROR_LOG("send buffer overflow");
//                return ERROR_HTTP;
//            }
            //ret = httpclient_tcp_send_all(client->handle, send_buf, HTTPCLIENT_SEND_BUF_SIZE);
            ret = client->net.write(&client->net, send_buf, HTTPCLIENT_SEND_BUF_SIZE, 5000);
            if (ret)
            {
                return (ret);
            }
        }
    } while (len);

    *send_idx = idx;
    return SUCCESS_RETURN;
}

void httpclient_set_custom_header(httpclient_t *client, char *header)
{
    client->header = header;
}

int httpclient_basic_auth(httpclient_t *client, char *user, char *password)
{
    if ((strlen(user) + strlen(password)) >= HTTPCLIENT_AUTHB_SIZE)
    {
        return ERROR_HTTP;
    }
    client->auth_user = user;
    client->auth_password = password;
    return SUCCESS_RETURN;
}

int httpclient_send_auth(httpclient_t *client, char *send_buf, int *send_idx)
{
    char b_auth[(int) ((HTTPCLIENT_AUTHB_SIZE + 3) * 4 / 3 + 1)];
    char base64buff[HTTPCLIENT_AUTHB_SIZE + 3];

    httpclient_get_info(client, send_buf, send_idx, "Authorization: Basic ", 0);
    sprintf(base64buff, "%s:%s", client->auth_user, client->auth_password);
    ALIOT_LOG_DEBUG("bAuth: %s", base64buff) ;
    httpclient_base64enc(b_auth, base64buff);
    b_auth[strlen(b_auth) + 1] = '\0';
    b_auth[strlen(b_auth)] = '\n';
    ALIOT_LOG_DEBUG("b_auth:%s", b_auth) ;
    httpclient_get_info(client, send_buf, send_idx, b_auth, 0);
    return SUCCESS_RETURN;
}

int httpclient_tcp_send_all(int sock_fd, char *data, int length)
{
    int written_len = 0;

    while (written_len < length)
    {
        int ret = aliyun_iot_network_send(sock_fd, data + written_len, length - written_len, IOT_NET_FLAGS_DEFAULT);
        if (ret > 0)
        {
            written_len += ret;
            continue;
        }
        else if (ret == 0)
        {
            return 0;
        }
        else
        {
            ALIOT_LOG_ERROR("run aliyun_iot_network_send error,errno = %d",aliyun_iot_get_errno());
            return -1; /* Connnection error */
        }
    }

    return written_len;
}

int httpclient_send_header(httpclient_t *client, char *url, int method, httpclient_data_t *client_data)              //����ͷ����Ϣ 
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

int httpclient_send_userdata(httpclient_t *client, httpclient_data_t *client_data)           
{
    int ret = 0;

    if (client_data->post_buf && client_data->post_buf_len)
    {
        ALIOT_LOG_DEBUG("client_data->post_buf_len:%d", client_data->post_buf_len);
        {
            //ret = httpclient_tcp_send_all(client->handle, (char *)client_data->post_buf, client_data->post_buf_len);
            ret = client->net.write(&client->net, (char *)client_data->post_buf, client_data->post_buf_len, 5000);
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
        }
    }

    return SUCCESS_RETURN;
}

int httpclient_recv(httpclient_t *client, char *buf, int min_len, int max_len, int *p_read_len) /* 0 on success, err code on failure */
{
    int ret = 0;
    uint32_t readLen = 0;
    IOT_NET_FD_ISSET_E result = IOT_NET_FD_NO_ISSET;

    while (readLen < max_len)
    {
        buf[readLen] = '\0';
        if (readLen < min_len)
        {
            //wait to read HTTP respond data
            ret = client->net.read(&client->net, buf + readLen, min_len - readLen, 10000);
        } else {
            //read the rest data in TCP buffer (with little wait time)
            ret = client->net.read(&client->net, buf + readLen, max_len - readLen, 2000);

            if (ret < 0) {
                int32_t err = aliyun_iot_get_errno();
                if (err == EAGAIN_IOT || err == EWOULDBLOCK_IOT) {
                    ALIOT_LOG_ERROR("http recv error");
                    break;
                }
            }
        }

        if (ret > 0) {
            readLen += ret;
        } else if (ret == 0) {
            break;
            //ALIOT_LOG_ERROR("recv TIMEOUT ret = [%d]\n",ret);
            //aliyun_iot_pthread_taskdelay(500);
            //continue;
        } else {
            ALIOT_LOG_ERROR("Connection error (recv returned %d)", ret);
            *p_read_len = readLen;
            return ERROR_HTTP_CONN;
        }
    }

    ALIOT_LOG_DEBUG("Read %d bytes", readLen);
    *p_read_len = readLen;
    buf[readLen] = '\0';

    return SUCCESS_RETURN;
}

int httpclient_retrieve_content(httpclient_t *client, char *data, int len, httpclient_data_t *client_data)
{
    int count = 0;
    int templen = 0;
    int crlf_pos;
    /* Receive data */
    ALIOT_LOG_DEBUG("Receiving data:%s", data);
    client_data->is_more = TRUE_IOT;

    if (client_data->response_content_len == -1 && client_data->is_chunked == FALSE_IOT)
    {
        while (TRUE_IOT)
        {
            int ret, max_len;
            if (count + len < client_data->response_buf_len - 1)
            {
                memcpy(client_data->response_buf + count, data, len);
                count += len;
                client_data->response_buf[count] = '\0';
            }
            else
            {
                memcpy(client_data->response_buf + count, data, client_data->response_buf_len - 1 - count);
                client_data->response_buf[client_data->response_buf_len - 1] = '\0';
                return HTTP_RETRIEVE_MORE_DATA;
            }

            max_len = HTTPCLIENT_MIN(HTTPCLIENT_CHUNK_SIZE - 1, client_data->response_buf_len - 1 - count);
            ret = httpclient_recv(client, data, 1, max_len, &len);

            /* Receive data */
            ALIOT_LOG_DEBUG("data len: %d %d", len, count);

            if (ret == ERROR_HTTP_CONN)
            {
                ALIOT_LOG_DEBUG("ret == ERROR_HTTP_CONN");
                return ret;
            }

            if (len == 0)
            {/* read no more data */
                ALIOT_LOG_DEBUG("no more len == 0");
                client_data->is_more = FALSE_IOT;
                return SUCCESS_RETURN;
            }
        }
    }

    while (TRUE_IOT)
    {
        uint32_t readLen = 0;

        if (client_data->is_chunked && client_data->retrieve_len <= 0)
        {
            /* Read chunk header */
            bool foundCrlf;
            int n;
            do
            {
                foundCrlf = FALSE_IOT;
                crlf_pos = 0;
                data[len] = 0;
                if (len >= 2)
                {
                    for (; crlf_pos < len - 2; crlf_pos++)
                    {
                        if (data[crlf_pos] == '\r' && data[crlf_pos + 1] == '\n')
                        {
                            foundCrlf = TRUE_IOT;
                            break;
                        }
                    }
                }
                if (!foundCrlf)
                { /* Try to read more */
                    if (len < HTTPCLIENT_CHUNK_SIZE)
                    {
                        int new_trf_len, ret;
                        ret = httpclient_recv(client, data + len, 0, HTTPCLIENT_CHUNK_SIZE - len - 1, &new_trf_len);
                        len += new_trf_len;
                        if (ret == ERROR_HTTP_CONN)
                        {
                            return ret;
                        }
                        else
                        {
                            continue;
                        }
                    }
                    else
                    {
                        return ERROR_HTTP;
                    }
                }
            } while (!foundCrlf);
            data[crlf_pos] = '\0';
            n = sscanf(data, "%x", &readLen);/* chunk length */
            client_data->retrieve_len = readLen;
            client_data->response_content_len += client_data->retrieve_len;
            if (n != 1)
            {
                ALIOT_LOG_ERROR("Could not read chunk length");
                return ERRO_HTTP_UNRESOLVED_DNS;
            }

            memmove(data, &data[crlf_pos + 2], len - (crlf_pos + 2)); /* Not need to move NULL-terminating char any more */
            len -= (crlf_pos + 2);

            if (readLen == 0)
            {
                /* Last chunk */
                client_data->is_more = FALSE_IOT;
                ALIOT_LOG_DEBUG("no more (last chunk)");
                break;
            }
        }
        else
        {
            readLen = client_data->retrieve_len;
        }

        ALIOT_LOG_DEBUG("Retrieving %d bytes, len:%d", readLen, len);

        do
        {
            templen = HTTPCLIENT_MIN(len, readLen);
            if (count + templen < client_data->response_buf_len - 1)
            {
                memcpy(client_data->response_buf + count, data, templen);
                count += templen;
                client_data->response_buf[count] = '\0';
                client_data->retrieve_len -= templen;
            }
            else
            {
                memcpy(client_data->response_buf + count, data, client_data->response_buf_len - 1 - count);
                client_data->response_buf[client_data->response_buf_len - 1] = '\0';
                client_data->retrieve_len -= (client_data->response_buf_len - 1 - count);
                return HTTP_RETRIEVE_MORE_DATA;
            }

            if (len > readLen)
            {
                ALIOT_LOG_DEBUG("memmove %d %d %d\n", readLen, len, client_data->retrieve_len);
                memmove(data, &data[readLen], len - readLen); /* chunk case, read between two chunks */
                len -= readLen;
                readLen = 0;
                client_data->retrieve_len = 0;
            }
            else
            {
                readLen -= len;
            }

            if (readLen)
            {
                int ret;
                int max_len = HTTPCLIENT_MIN(HTTPCLIENT_CHUNK_SIZE - 1, client_data->response_buf_len - 1 - count);
                ret = httpclient_recv(client, data, 1, max_len, &len);
                if (ret == ERROR_HTTP_CONN)
                {
                    return ret;
                }
            }
        } while (readLen);

        if (client_data->is_chunked)
        {
            if (len < 2)
            {
                int new_trf_len, ret;
                /* Read missing chars to find end of chunk */
                ret = httpclient_recv(client, data + len, 2 - len, HTTPCLIENT_CHUNK_SIZE - len - 1, &new_trf_len);
                if (ret == ERROR_HTTP_CONN)
                {
                    return ret;
                }
                len += new_trf_len;
            }
            if ((data[0] != '\r') || (data[1] != '\n'))
            {
                ALIOT_LOG_ERROR("Format error, %s", data); /* after memmove, the beginning of next chunk */
                return ERRO_HTTP_UNRESOLVED_DNS;
            }
            memmove(data, &data[2], len - 2); /* remove the \r\n */
            len -= 2;
        }
        else
        {
            ALIOT_LOG_DEBUG("no more(content-length)\n");
            client_data->is_more = FALSE_IOT;
            break;
        }

    }

    return SUCCESS_RETURN;
}

int httpclient_response_parse(httpclient_t *client, char *data, int len, httpclient_data_t *client_data)
{
    int crlf_pos;
    char *crlf_ptr = strstr(data, "\r\n");

    client_data->response_content_len = -1;
    
    if (crlf_ptr == NULL)
    {
        ALIOT_LOG_ERROR("\r\n not found");
        return ERRO_HTTP_UNRESOLVED_DNS;
    }

    crlf_pos = crlf_ptr - data;
    data[crlf_pos] = '\0';

    /* Parse HTTP response */
    if (sscanf(data, "HTTP/%*d.%*d %d %*[^\r\n]", &(client->response_code)) != 1)
    {
        /* Cannot match string, error */
        ALIOT_LOG_ERROR("Not a correct HTTP answer : %s\n", data);
        return ERRO_HTTP_UNRESOLVED_DNS;
    }

    if ((client->response_code < 200) || (client->response_code >= 400))
    {
        /* Did not return a 2xx code; TODO fetch headers/(&data?) anyway and implement a mean of writing/reading headers */
        ALIOT_LOG_WARN("Response code %d", client->response_code);
    }

    ALIOT_LOG_DEBUG("Reading headers%s", data);

    memmove(data, &data[crlf_pos + 2], len - (crlf_pos + 2) + 1); /* Be sure to move NULL-terminating char as well */
    len -= (crlf_pos + 2);

    client_data->is_chunked = FALSE_IOT;

    /* Now get headers */
    while (TRUE_IOT)
    {
        char key[32];
        char value[32];
        int n;

        key[31] = '\0';
        value[31] = '\0';

        crlf_ptr = strstr(data, "\r\n");
        if (crlf_ptr == NULL)
        {
            if (len < HTTPCLIENT_CHUNK_SIZE - 1)
            {
                int new_trf_len, ret;
                ret = httpclient_recv(client, data + len, 1, HTTPCLIENT_CHUNK_SIZE - len - 1, &new_trf_len);
                len += new_trf_len;
                data[len] = '\0';
                ALIOT_LOG_DEBUG("Read %d chars; In buf: [%s]", new_trf_len, data);
                if (ret == ERROR_HTTP_CONN)
                {
                    return ret;
                }
                else
                {
                    continue;
                }
            }
            else
            {
                ALIOT_LOG_DEBUG("header len > chunksize");
                return ERROR_HTTP;
            }
        }

        crlf_pos = crlf_ptr - data;
        if (crlf_pos == 0)
        { /* End of headers */
            memmove(data, &data[2], len - 2 + 1); /* Be sure to move NULL-terminating char as well */
            len -= 2;
            break;
        }

        data[crlf_pos] = '\0';

        n = sscanf(data, "%31[^:]: %31[^\r\n]", key, value);
        if (n == 2)
        {
            ALIOT_LOG_DEBUG("Read header : %s: %s", key, value);
            if (!strcmp(key, "Content-Length"))
            {
                sscanf(value, "%d", &(client_data->response_content_len));
                client_data->retrieve_len = client_data->response_content_len;
            }
            else if (!strcmp(key, "Transfer-Encoding"))
            {
                if (!strcmp(value, "Chunked") || !strcmp(value, "chunked"))
                {
                    client_data->is_chunked = TRUE_IOT;
                    client_data->response_content_len = 0;
                    client_data->retrieve_len = 0;
                }
            }
            memmove(data, &data[crlf_pos + 2], len - (crlf_pos + 2) + 1); /* Be sure to move NULL-terminating char as well */
            len -= (crlf_pos + 2);

        }
        else
        {
            ALIOT_LOG_ERROR("Could not parse header");
            return ERROR_HTTP;
        }
    }

    return httpclient_retrieve_content(client, data, len, client_data);
}

aliot_err_t httpclient_connect(httpclient_t *client)
{   p_dbg_enter;
    int ret = ERROR_HTTP_CONN;

    client->net.handle = -1;

    ret = httpclient_conn(client);
//    if (0 == ret)
//    {
//        client->remote_port = HTTP_PORT;
//    }
  p_dbg_exit;
    return ret;
}

aliot_err_t httpclient_send_request(httpclient_t *client, char *url, int method, httpclient_data_t *client_data)
{
    int ret = ERROR_HTTP_CONN;

    if (client->net.handle < 0)
    {
        return ret;
    }

    ret = httpclient_send_header(client, url, method, client_data);
    if (ret != 0)
    {
        ALIOT_LOG_ERROR("httpclient_send_header is error,ret = %d",ret);
        return ret;
    }

    if (method == HTTPCLIENT_POST || method == HTTPCLIENT_PUT)
    {
        ret = httpclient_send_userdata(client, client_data);             
    }

    return ret;
}

aliot_err_t httpclient_get_content_len(char *buf, int* conten_tlen)
{
    char *ptr = NULL;
    int len = 0;
    ptr = strstr(buf, "Content-Length");
    if(ptr == NULL)
    {
        ALIOT_LOG_ERROR("can not find Content-Length");
        return SUCCESS_RETURN;
    }
    
    while(*ptr < '0' || *ptr > '9' ) ptr++;
    while(*ptr >= '0' && *ptr <= '9')
    {
        len = len*10+ (*ptr - '0');
        ptr++;
    }
    *conten_tlen = len;
    ALIOT_LOG_ERROR("Content-Length = [%d]", len);
    
    return SUCCESS_RETURN;
}

//receive http response, you can only call this function when there are "Content-Length" in the header
aliot_err_t httpclient_recv_response_content(httpclient_t *client, httpclient_data_t *client_data)
{
    int recflag = 0, ret = ERROR_HTTP_CONN, recvlen = 0, recvindex = 0;
    char buf[HTTPCLIENT_CHUNK_SIZE] = { 0 };
    char * pH = NULL;
    int content_len = 0;
    //recflag: 1->already recvd the whole http header
    client_data->response_content_len = 0;
    do
   {
        if (client->net.handle < 0)
        {
            return ret;
        }
        
        recvlen = HTTPCLIENT_CHUNK_SIZE - 1 - recvindex;               //HTTPCLIENT_CHUNK_SIZE           512
        ret = client->net.read(&client->net, buf + recvindex, recvlen , 10000);    //�ӽ��հٶ�ƽ̨��������        //10���ڵĽ���
        if(ret > 0)
        {
            buf[recvindex+ret] = 0;
            ALIOT_LOG_ERROR("recvlen=[%d], ret = [%d]\n",recvlen, ret);
            if(recflag == 0)
            {
                //this is a first data piece recved, we assume it contains http header "Content-Type" and "Content-Length"
                if(content_len == 0)
                {
                    if(httpclient_get_content_len(buf, &content_len) != SUCCESS_RETURN)
                    {
                        break;
                    }
                    if(content_len < 0)
                    {
                        ALIOT_LOG_ERROR("HTTP content-length error\n");
                        break;
                    }
                }
                
                pH = strstr(buf, "\r\n\r\n");
                if(pH == NULL)
                {
                    ALIOT_LOG_ERROR("NO header ending found\n");
                    if(ret >= 14) //copy last 13bytes in case it is "Content-Lengt"
                    {
                        memcpy(buf, &buf[ret-14], 13);
                        recvindex = 13;
                    }
                    continue;
                }
                else
                {
                    pH += strlen("\r\n\r\n");
                    recflag = 1;
                    
                    content_len -= (buf+ ret + recvindex - pH);
                    while(*pH != 0)
                    {
                        client_data->response_buf[client_data->response_content_len++] = *pH;      //���յ����ݱ�����response_buf
                        //need to check the buffer overflow
                        pH++;
                    }
                    client_data->response_buf[client_data->response_content_len] = 0;
                    recvindex = 0;
                }
            }
            else
            {
                    pH = buf;
                    content_len -= (buf+ ret - pH);
                    while(*pH != 0)
                    {
                        client_data->response_buf[client_data->response_content_len++] = *pH;
                        //need to check the buffer overflow
                        pH++;
                    }
                    client_data->response_buf[client_data->response_content_len] = 0;
            }

            if(content_len > 0)
            {  //full data get, try one more chance
                continue;
            }

            return SUCCESS_RETURN;
        }
        else if(ret == 0)
        {
            ALIOT_LOG_ERROR("recv TIMEOUT ret = [%d]\n",ret);
            //return HTTP_RETRIEVE_MORE_DATA;     //timeout, need to read data later
            aliyun_iot_pthread_taskdelay(500);
            continue;
        }
        else
        {
            ALIOT_LOG_ERROR("recv ERROR ret = [%d]\n",ret);
            break;
        }
    }while(1);
    
    return ret;
}

aliot_err_t httpclient_recv_response(httpclient_t *client, httpclient_data_t *client_data)
{
    int reclen = 0, ret = ERROR_HTTP_CONN;
    char buf[HTTPCLIENT_CHUNK_SIZE] = { 0 };

    if (client->net.handle < 0)
    {
        return ret;
    }

    if (client_data->is_more)
    {
        client_data->response_buf[0] = '\0';
        ret = httpclient_retrieve_content(client, buf, reclen, client_data);
    }
    else
    {
        ret = httpclient_recv(client, buf, 1, HTTPCLIENT_CHUNK_SIZE - 1, &reclen);
        if (ret != 0)
        {
            return ret;
        }

        buf[reclen] = '\0';

        if (reclen)
        {
            ALIOT_LOG_DEBUG("reclen:%d, buf:", reclen);
            ALIOT_LOG_DEBUG("%s", buf);
            ret = httpclient_response_parse(client, buf, reclen, client_data);
        }
    }

    return ret;
}

void httpclient_close(httpclient_t *client)
{

    if (client->net.handle >= 0)
    {
        client->net.disconnect(&client->net);
    }
    client->net.handle = -1;
}

int httpclient_common(httpclient_t *client, char *url, int port, char *ca_crt, int method, httpclient_data_t *client_data)     //��������    ��������   ������Ӧ    
{
    int ret = ERROR_HTTP_CONN;
    char port_str[6] = { 0 };
    char host[HTTPCLIENT_MAX_HOST_LEN] = { 0 };

    httpclient_parse_host(url, host, sizeof(host));  //
    snprintf(port_str, sizeof(port_str), "%d", port);
    
    ALIOT_LOG_DEBUG("host:%s, port:%s", host, port_str);
  
    aliyun_iot_net_init(&client->net, host, port_str, ca_crt);
   
    ret = httpclient_connect(client);                                       //��������
  
    if(0 != ret)
    {
        ALIOT_LOG_ERROR("httpclient_connect is error,ret = %d",ret);   
        httpclient_close(client);
        return ret;
    }

    ret = httpclient_send_request(client, url, method, client_data);           //��������
    if(0 != ret)
    {
        ALIOT_LOG_ERROR("httpclient_send_request is error,ret = %d",ret);
        httpclient_close(client);
        return ret;
    }

    ret = httpclient_recv_response(client, client_data);                          //������Ӧ                   
    if (0 != ret)
    {
        ALIOT_LOG_ERROR("httpclient_recv_response is error,ret = %d",ret);
        httpclient_close(client);
        return ret;
    }

    httpclient_close(client);
    return ret;
}

int aliyun_iot_common_get_response_code(httpclient_t *client)
{
    return client->response_code;
}

aliot_err_t aliyun_iot_common_post(        //һ��post��������ݸ�ʽ   //��������    ��������   ������Ӧ 
        httpclient_t *client,
        char *url,
        int port,
        char *ca_crt,
        httpclient_data_t *client_data)
{
    return httpclient_common(client, url, port, ca_crt, HTTPCLIENT_POST, client_data);
}


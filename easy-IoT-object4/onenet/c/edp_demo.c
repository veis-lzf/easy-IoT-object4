//#include <stdio.h>
//#include <stdlib.h>
//@#include <unistd.h>
#include <string.h>
//@#include <sys/types.h>
//@#include <sys/socket.h>
//@#include <netinet/in.h>
//@#include <netdb.h> 
#define DEBUG

#include "edp_demo.h"
#include "EdpKit.h"
#include "uMEM.h"

#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/dhcp.h"

#include "tcpapp.h"
#include "wifi.h"
#include "debug.h"
#include "drivers.h"
#include "sys_misc.h"
#if LWIP_SOCKET

#if ONENET_ENABLE
/*---------------------------------------------------------------------------*/
/* Error Code                                                                */
/*---------------------------------------------------------------------------*/
#define ER_CREATE_SOCKET   -1 
#define ER_HOSTBYNAME      -2 
#define ER_CONNECT         -3 
#define ER_SEND            -4
#define ER_TIMEOUT         -5
#define ER_RECV            -6
/*---------------------------------------------------------------------------*/
/* Port Socket Function                                                           */
/*---------------------------------------------------------------------------*/
#define Socket(a,b,c)          socket(a,b,c)
#define Connect(a,b,c)         connect(a,b,c)
#define Close(a)               close(a)
#define Read(a,b,c)            read(a,b,c)
#define Recv(a,b,c,d)          recv(a, (void *)b, c, d)
#define Select(a,b,c,d,e)      select(a,b,c,d,e)
#define Send(a,b,c,d)          send(a, (const int8 *)b, c, d)
#define Write(a,b,c)           write(a,b,c)
#define GetSockopt(a,b,c,d,e)  getsockopt((int)a,(int)b,(int)c,(void *)d,(socklen_t *)e)
#define SetSockopt(a,b,c,d,e)  setsockopt((int)a,(int)b,(int)c,(const void *)d,(int)e)
#define GetHostByName(a)       gethostbyname((const char *)a)

extern void update_relay_status(int relay, int value);

#define LEDCTL_ID "ledctl"     //IO4 for PWM
#define RELAYA_ID "relaya"     //IO5
#define RELAYB_ID "relayb"     //IO6
#define RELAYC_ID "relayc"     //IO7
#define RELAYD_ID "relayd"     //IO8

typedef struct http_ctl
{
    char * str;
    int16 value;
    int16 change;
}http_ctl_t;

http_ctl_t myhttp[5]= {{LEDCTL_ID, 50, 1},
                       {RELAYA_ID, 0, 1},
                       {RELAYB_ID, 0, 1},
                       {RELAYC_ID, 0, 1},
                       {RELAYD_ID, 0, 1}};

int16 get_number(char * str)
{
    int16 ret = 0;
    int i = 0;
    if(str == NULL) return -1;
    while(*str == ' ' || *str == ':') str++;
    
    while(str[i] >= '0' && str[i] <= '9')
    {
        ret = ret*10+(str[i] - '0');
        i++;
        if(i > 5) break;
    }
    
    if(i == 0) return -1;
    
    return ret;
}

int16 update_stream(char *str, int16 num)
{
    int i = 0;
    if(str == NULL) return -1;
    
    while(*str == ' ' || *str == '"') str++;
    for(i = 0; i < 5; i++)
    {
        if(strncmp(str, myhttp[i].str, 6) == 0)
        {
            if(myhttp[i].value != num)
            {
                myhttp[i].value = num;
                myhttp[i].change = 1;
            }
            
            return 0;
        }
    }

    return -1;
}

void update_ctl(void)
{
    if(myhttp[0].change)
    {
        lamp_pwm_set(myhttp[0].value);
        p_err("ledctl change to [%d]\n", myhttp[0].value);
        //myhttp[0].change = 0;
    }
    if(myhttp[1].change)
    {
        update_relay_status(5, myhttp[1].value);
        p_err("IO5 change to [%d]\n", myhttp[1].value);
        //myhttp[1].change = 0;
    }
    if(myhttp[2].change)
    {
        update_relay_status(6, myhttp[2].value);
        p_err("IO6 change to [%d]\n", myhttp[2].value);
        //myhttp[2].change = 0;
    }
    if(myhttp[3].change)
    {
        update_relay_status(7, myhttp[3].value);
        p_err("IO7 change to [%d]\n", myhttp[3].value);
        //myhttp[3].change = 0;
    }
    if(myhttp[4].change)
    {
        update_relay_status(8, myhttp[4].value);
        p_err("IO8 change to [%d]\n", myhttp[4].value);
        //myhttp[4].change = 0;
    }

}
int32 Open(const uint8 *addr, int16 portno)
{
    int32 sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    int opt;
    
    /* create socket */
    sockfd = Socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        p_err("onenet Socket alloc ERROR\n");
        return ER_CREATE_SOCKET; 
    }
    server = GetHostByName(addr);
    if (server == NULL) {
        Close(sockfd);
        p_err("onenet DNS ERROR\n");
        return ER_HOSTBYNAME;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_len = sizeof(serv_addr);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = PP_HTONS(portno);
    //addr.sin_addr.s_addr = inet_addr(SOCK_TARGET_HOST);
    serv_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr *)server->h_addr_list[0])));
  
    if (Connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
    {
        p_err("onenet ERROR connecting\n");
        Close(sockfd);
        return ER_CONNECT;
    }

    opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(int)) < 0)
    {
        p_err("onenet setsockopt error\n");
        Close(sockfd);
        return ER_CONNECT;
    }

    return sockfd;
}

int32 DoSend(int32 sockfd, char* buffer, uint32 len)
{
    int32 total  = 0;
    int32 n = 0;
    //int32 tmp = 0;
    while (len != total)
    {
        //tmp = len - total;
        //if(tmp > 512) tmp = 512;
        
        n = Send(sockfd,buffer + total,len - total,0);
        //n = Send(sockfd, buffer + total, tmp, 0);
        if (n <= 0)
        {
            //fprintf(stderr, "ERROR writing to socket\n");
            return n;
        }
        total += n;
    }
    return total;
}

int recv_func(int arg)
{
    int sockfd = arg;
    int error = 0;
    int n, rtn;
    uint8 mtype, jsonorbin;
    char buffer[1024];
    RecvBuffer* recv_buf = NewBuffer();
    EdpPacket* pkg;
    
    char* src_devid;
    char* push_data;
    uint32 push_datalen;

    cJSON* save_json;
    char* save_json_str;

    cJSON* desc_json;
    char* desc_json_str;
    char* save_bin; 
    uint32 save_binlen;
    int ret = 0;
    char *json_ack = NULL;
    char *cmdid = NULL;
    uint16 cmdid_len = 0;
    char* req = NULL;
    uint32 req_len = 0;
    int16 num = -1;
    do
    {
        memset(buffer, 0, 1024);
        n = Recv(sockfd, buffer, 1024, 0);
        if (n <= 0)
        {
            p_err("onenet recv error, bytes: %d\n", n);
            error = -1;
            break;
        }
        
        //p_err("onenet recv from server, bytes: %d\n", n);
        
        WriteBytes(recv_buf, buffer, n);
        while (1)
        {   
            if ((pkg = GetEdpPacket(recv_buf)) == 0)
            {
                //p_err("onenet need more bytes...\n");
                break;
            }
        
            mtype = EdpPacketType(pkg);
            switch(mtype)
            {
                case CONNRESP:
                    rtn = UnpackConnectResp(pkg);
                    p_err("onenet recv connect resp, rtn: %d\n", rtn);
                    break;
                case PUSHDATA:
                    UnpackPushdata(pkg, &src_devid, &push_data, &push_datalen);
                    p_err("onenet recv push data, src_devid: %s, push_data: %s, len: %d\n", 
                            src_devid, push_data, push_datalen);
                    uMEM_Free(src_devid);
                    uMEM_Free(push_data);
                    break;
                case SAVEDATA:
                    if (UnpackSavedata(pkg, &src_devid, &jsonorbin) == 0)
                    {
                        if (jsonorbin == 0x01) 
                        {/* json */
                            ret = UnpackSavedataJson(pkg, &save_json);
                            save_json_str=cJSON_Print(save_json);
                            //p_err("recv save data json, ret = %d, src_devid: %s, json: %s\n", 
                            //        ret, src_devid, save_json_str);
                            uMEM_Free(save_json_str);
                            cJSON_Delete(save_json);
                        }
                        else if (jsonorbin == 0x02)
                        {/* bin */
                            UnpackSavedataBin(pkg, &desc_json, (uint8**)&save_bin, &save_binlen);
                            desc_json_str=cJSON_Print(desc_json);
                            p_err("recv save data bin, src_devid: %s, desc json: %s, bin: %s, binlen: %d\n", 
                                    src_devid, desc_json_str, save_bin, save_binlen);
                            uMEM_Free(desc_json_str);
                            cJSON_Delete(desc_json);
                            uMEM_Free(save_bin);
                        }
                        uMEM_Free(src_devid);
                    }
                    break;
                case SAVEACK:
                    
                    UnpackSavedataAck(pkg, &json_ack);
                    p_err("save json ack = %s\n", json_ack);
                    free(json_ack);
                    break;
                case PINGRESP:
                    UnpackPingResp(pkg); 
                    p_err("recv ping resp\n");
                    break;
                case CMDREQ:
                    if(UnpackCmdReq(pkg, &cmdid, &cmdid_len, &req, &req_len) != 0)
                    {
                        break;
                    }
                    p_err("recv CMDREQ cmd[%s]cmdlen[%d], req[%s]reqlen[%d]\n", cmdid, cmdid_len, req, req_len);
                    num = get_number(req+7);
                    if(num < 0)
                    {
                        uMEM_Free(cmdid);
                        uMEM_Free(req);
                        break;
                    }

                    update_stream(req, num);
                    update_ctl();
                    uMEM_Free(cmdid);
                    uMEM_Free(req);
                    break;
                default:
                    p_err("recv failed...type[%x]\n", mtype);
                    break;
            }
            DeleteBuffer(&pkg);
        }
    }while(0);
    
    DeleteBuffer(&recv_buf);

    return error;
}

/*remember system time*/
//static unsigned int sys_timer = 0;
//static char send_buf[512];

#define DEV_ID "76633"                         //change to your device-ID
#define API_KEY "oIMo6LR2GC58vYkM8qbnGxOWzgkA" //change to your API-Key

extern uint8_t *image_data;
extern wait_event_t client_pic_event;
extern uint8_t onenet_temp;
extern uint8_t onenet_humi;
typedef struct onenet_ctl
{
    u32_t image_peroid;
    u32_t image_tick;
    u32_t temp_peroid;
    u32_t temp_tick;
    u32_t humi_peroid;
    u32_t humi_tick;
    u32_t light_peroid;
    u32_t light_tick;
    u32_t alarm_peroid;
    u32_t alarm_tick;
}onenet_ctl_t;
onenet_ctl_t my_onenet_ctl;

//init interval for every stream
void init_onenet_prama(void)
{
    my_onenet_ctl.image_tick = sys_now();
    my_onenet_ctl.image_peroid = 6000;       //upload image every 6s
    my_onenet_ctl.temp_tick = sys_now();
    my_onenet_ctl.temp_peroid = 4000;        //upload temperature every 4s
    my_onenet_ctl.humi_tick = sys_now();
    my_onenet_ctl.humi_peroid = 4000;        //upload humidity every 4s
    my_onenet_ctl.light_tick = sys_now();
    my_onenet_ctl.light_peroid = 2000;       //upload light every 2s
    my_onenet_ctl.alarm_tick = sys_now();
    my_onenet_ctl.alarm_peroid = 2000;       //upload port status every 2s
}

int write_func(int arg)
{
    //unsigned int now = sys_now();
    int sockfd = arg;
    int image_len = 0;
    EdpPacket* send_pkg = NULL;
    cJSON *desc_json;
    int32 ret = 0, flag = 0;
    uint32_t data = 0;
    //char text[25] = {0};
    char text_bin[]="{\"ds_id\": \"image\"}";
    int i = 0;
    /*push data every 2 Seconds*/
    //if(now - sys_timer < 2000)
    //{
    //    return 0;
    //}

    //print_button();
    //sys_timer = now;

    //send_buf[0] = 0;  
    //strcat(send_buf,"{");
    //strcat(send_buf,"\"datastreams\": [");
    //strcat(send_buf,"{");
    //strcat(send_buf,"\"id\": \"sys_time\",");
    //strcat(send_buf,"\"datapoints\": [");
    //strcat(send_buf,"{");
    //sprintf(text,   "\"value\": \"%d\"", sys_timer/1000);
    //strcat(send_buf,text);
    //strcat(send_buf,"}");
    //strcat(send_buf,"]");
    //strcat(send_buf,"}");
    //strcat(send_buf,"]");
    //strcat(send_buf,"}");

    //save_json=cJSON_Parse(send_buf);
    //if(NULL == save_json)
    //{
     //   p_err("cJSON_Parse error\n");
     //   return -1;
    //}
    //old data send method
    //send_pkg = PacketSavedataJson(DEV_ID, save_json, kTypeFullJson); //send to myself, and OneNet save data
    //cJSON_Delete(save_json);
    //0.1) update ledctl and relay to onenet
    for(i = 0; i < 5; i++)
    {
      if(myhttp[i].change)
      {
        send_pkg = PacketSavedataInt(kTypeFullJson, DEV_ID, myhttp[i].str, (int)myhttp[i].value, 0, NULL);
        if(NULL == send_pkg)
        {
            p_err("PacketSavedataInt [%s] error\n", myhttp[i].str);
            return -1;
        }
    
        ret = DoSend(sockfd, send_pkg->_data, send_pkg->_write_pos);
        p_err("write_func DoSend: [%s][%d], ret = %d\n", myhttp[i].str, myhttp[i].value, ret);
        DeleteBuffer(&send_pkg);
        send_pkg = NULL;
            
        flag++;
        myhttp[i].change = 0;
      }
    }

    //(1) send current temprature to onenet. 
    if(sys_now()- my_onenet_ctl.temp_tick >= my_onenet_ctl.temp_peroid)
    {
        send_pkg = PacketSavedataInt(kTypeFullJson, DEV_ID, "temp", (int)onenet_temp, 0, NULL);
        if(NULL == send_pkg)
        {
            p_err("PacketSavedataInt temp error\n");
            return -1;
        }

        ret = DoSend(sockfd, send_pkg->_data, send_pkg->_write_pos);
        p_err("write_func DoSend: TEMP[%d], ret = %d\n", onenet_temp, ret);
        DeleteBuffer(&send_pkg);
        send_pkg = NULL;
        
        flag++;
        my_onenet_ctl.temp_tick = sys_now();
    }

    //(2) send current humility to onenet. 
    if(sys_now()- my_onenet_ctl.humi_tick >= my_onenet_ctl.humi_peroid)
    {
        send_pkg = PacketSavedataInt(kTypeFullJson, DEV_ID, "humi", (int)onenet_humi, 0, NULL);
        if(NULL == send_pkg)
        {
            p_err("PacketSavedataInt humi error\n");
            return -1;
        }

        ret = DoSend(sockfd, send_pkg->_data, send_pkg->_write_pos);
        p_err("write_func DoSend: HUMI[%d], ret = %d\n", onenet_humi, ret);
        DeleteBuffer(&send_pkg);
        send_pkg = NULL;

        flag++;
        my_onenet_ctl.humi_tick = sys_now();
    }

    //(3) send current light to onenet. 
    if(sys_now()- my_onenet_ctl.light_tick >= my_onenet_ctl.light_peroid)
    {
        data = get_BH1750_value();
        send_pkg = PacketSavedataInt(kTypeFullJson, DEV_ID, "light", (int)data, 0, NULL);
        if(NULL == send_pkg)
        {
            p_err("PacketSavedataInt light error\n");
            return -1;
        }

        ret = DoSend(sockfd, send_pkg->_data, send_pkg->_write_pos);
        p_err("write_func DoSend: LIGHT[%d], ret = %d\n", data, ret);
        DeleteBuffer(&send_pkg);
        send_pkg = NULL;

        flag++;
        my_onenet_ctl.light_tick = sys_now();
    }
    
    //(4) send current alarm status to onenet. 
    if(sys_now()- my_onenet_ctl.alarm_tick >= my_onenet_ctl.alarm_peroid)
    {
        data = get_alarm_value();
        send_pkg = PacketSavedataInt(kTypeFullJson, DEV_ID, "alarm", (int)data, 0, NULL);
        if(NULL == send_pkg)
        {
            p_err("PacketSavedataInt alarm error\n");
            return -1;
        }

        ret = DoSend(sockfd, send_pkg->_data, send_pkg->_write_pos);
        p_err("write_func DoSend: ALARM[%d], ret = %d\n", data, ret);
        DeleteBuffer(&send_pkg);
        send_pkg = NULL;

        flag++;
        my_onenet_ctl.alarm_tick = sys_now();
    }
    //(5) try to send a picture to onenet
    if(sys_now()- my_onenet_ctl.image_tick >= my_onenet_ctl.image_peroid)
    {
//        start_capture_img();    //start the pic capture
        wait_event_timeout(client_pic_event, 3000);
        if(image_data == NULL)
        {
            p_err("wait image fail\n");
            //stop_capture_img();
            my_onenet_ctl.image_tick = sys_now();
            //return 0;
        }
        else
        {
            image_len = *((int*)image_data);
            desc_json=cJSON_Parse(text_bin);
            send_pkg = PacketSavedataBin(NULL, desc_json, (const uint8*)(image_data + 4), image_len);
            mem_free(image_data);
            image_data = NULL;
    
            cJSON_Delete(desc_json);
            if(NULL == send_pkg)
            {
                p_err("PacketSavedataBin PIC error\n");
                return -1;
            }

            ret = DoSend(sockfd, send_pkg->_data, send_pkg->_write_pos);
            p_err("write_func DoSend: PIC, ret = %d\n", ret);
            DeleteBuffer(&send_pkg);

            flag++;
            my_onenet_ctl.image_tick = sys_now();
        }
    }

    if(flag == 0)
    {
        sys_msleep(200);
    }
    return ret;
}

int client_process_func(void* arg)
{
    int sockfd = *(int*)arg;
    fd_set readset;
    fd_set writeset;
    int i, maxfdp1;
    
    for (;;)
    {
        maxfdp1 = sockfd+1;
    
        /* Determine what sockets need to be in readset */
        FD_ZERO(&readset);
        FD_ZERO(&writeset);
        FD_SET(sockfd, &readset);
        FD_SET(sockfd, &writeset);

        i = select(maxfdp1, &readset, &writeset, 0, 0);
        
        if (i == 0)
            continue;
        
        if (FD_ISSET(sockfd, &readset))
        {
            /* This socket is ready for reading*/
            if (recv_func(sockfd) < 0)
                    break;
        }
           /* This socket is ready for writing*/
        if (FD_ISSET(sockfd, &writeset))
        {
            if (write_func(sockfd) < 0)
                    break;
        }
        
    }

    return -1;
}

#define SERVER_ADDR "jjfaedp.hedevice.com"    //OneNet EDP server addr
#define SERVER_PORT 876                       //OneNet EDP server port

void edp_demo(void * arg)
{
    int sockfd, n, ret;
    
    EdpPacket* send_pkg;
    //init memory first
    
    //sys_timer = sys_now();
    init_onenet_prama();
    update_ctl();
    
    while(1)
    {
        /* create a socket and connect to server */
        sockfd = Open(SERVER_ADDR, SERVER_PORT);
        if (sockfd < 0)
        {
            p_err("onenet open error\n");
            continue;
        }
    
        /* connect to server */
        send_pkg = PacketConnect1(DEV_ID, API_KEY);

        p_dbg("send connect to server, bytes: %d\n", send_pkg->_write_pos);
        ret=DoSend(sockfd, send_pkg->_data, send_pkg->_write_pos);
        p_dbg("send connect Over, ret: %d\n", ret);
        DeleteBuffer(&send_pkg);
        p_dbg("DeleteBuffer Out\n");
        client_process_func((void*)&sockfd);

        /*recv error happen, just close socket*/
        Close(sockfd);
        p_dbg("socket error, close socket and try again\n");

    }
    
    //return 0;
}

void init_onenet_edpclient(void)
{
    thread_create(edp_demo, 0, TASK_IOT_CLIENT_PRIO, 0, TASK_IOT_CLIENT_STK_SIZE, "edp_demo");
    p_dbg("[task]onenet client created");
}

#endif
#endif


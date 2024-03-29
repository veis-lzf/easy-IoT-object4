#define DEBUG
#include <api.h>
#include <lwip/api.h>
#include <lwip/inet.h>
#include <lwip/sockets.h>
#include "html.h"
#include "web_cfg.h"
#include "debug.h"
#include "wifi.h"
#include  "usr_cfg.h"
#include "app.h"
#include "sys_misc.h"
#include "drivers.h"



static char g_buf[1024];
extern int netcam_init(void);
extern  u8 post_state;


extern int set_ipaddr(struct netif *p_netif, struct ip_addr *ipaddr, struct ip_addr *netmask, struct ip_addr *gw, struct ip_addr *dns) ;
extern struct netif *p_netif;
extern u8 interface_ret;

#define web_thread_port          80
#define web_thread_prio          5


#define   admin_name     "admin"
#define   admin_pwd      "123456789"

#define   sniper         "sniper?"  //判断接收到是数据  sniper?   favicon.ico

#define   favicon         "favicon.ico"


WEB_CFG  web_cfg_ser;

#define  ap_name   web_cfg_server->ap.essid
#define  ap_pwd    web_cfg_server->ap.key

#define  sta_name  web_cfg_server->sta.essid
#define  sta_pwd   web_cfg_server->sta.key


u8 post_state = 0;
char str[100] = "";
char my_str[20] = "";
char my_str1[20] = "";

int web_server_thread_id;
char str_1[5] = "user=";
char str_2[4] = "pwd=";
char *str_ap_name;
char *str_ap_pwd;
char *str_sta_name;
char *str_sta_pwd;



/*-----------------------------------------------------------------------------------*/
int  handle_user_rev(char *g_buf, char * str);
int  handle_pwd_rev (char *g_buf, char *str);
int admin (char *g_buf );
int sdid_set (char *g_buf );

/*-----------------------------------------------------------------------------------*/
int send_link(int client, char * html)
{
    g_buf[0] = 0;
    if (send(client, html, strlen((char *)html), 0) < 0)
    {
        closesocket(client);
        return -1;
    }
    return 0;
}

void web_server_thread(void *parameter)
{
    static u8 my_web_flag = 0;
    int on, my_ret ;
    int srv_sock = -1;
    static u8 state = 0;
    char *buf;
    u16_t buflen;
    err_t err;
    struct sockaddr_in addr;
    socklen_t sock_len = sizeof(struct sockaddr_in);

    srv_sock = socket(AF_INET, SOCK_STREAM, 0);     //创建一个socket，使用AF_INET
    if (srv_sock < 0)
    {
        p_dbg("netcam: create server socket failed due to (%s)\n",
               strerror(errno));
        goto exit;
    }

    memset(&addr, 0, sock_len);                     //清空数组
    addr.sin_family = AF_INET;                      //
    addr.sin_port = htons(web_thread_port);         //htons:判断大小端
    addr.sin_addr.s_addr = INADDR_ANY;              //任意地址

    /* ignore "socket already in use" errors */
    on = 1;
    setsockopt(srv_sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));        //获取或者设置与某个套接字关联的选 项。
    setsockopt(srv_sock, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));

    if (bind(srv_sock, (struct sockaddr *)&addr, sock_len) != 0)
    {
        p_dbg("netcam: bind() failed due to (%s)\n",
               strerror(errno));
        goto exit;
    }
    if (listen(srv_sock, 5) != 0)               //5：内核为此套接口排队的最大连接个数
    {
        p_dbg("netcam: listen() failed due to (%s)\n",
               strerror(errno));
        goto exit;
    }
    while (1)
    {

        p_dbg("等待连接\r\n");

        if(my_web_flag == 3)
        {
            p_dbg("路由设置成功\r\n");
            LCD_Init();
			Show_Str(200,130,150,24,(unsigned char *)"智能识别器",24,0);
			Show_Str(10,280,300,24,(unsigned char *)"WIFI设置",24,0);  
			Show_Str(380,280,150,24,(unsigned char *)"识别模式",24,0);	

            interface_ret = 0;
            send_work_event(MODE_DETECT_CHANGE_STA);
            my_web_flag = 0;
        }

        struct sockaddr_in client_addr;
        int client = accept(srv_sock, (struct sockaddr *)&client_addr, &sock_len);      //在请求队列中获取一个新建立的连接
        if (client < 0)
            continue;
		// 接收到浏览器的请求数据
        if (recv(client, g_buf, sizeof(g_buf) - 1, 0)) {
            if(strstr(g_buf, favicon) != NULL) {
                closesocket(client);
                sleep(50);
                continue  ;
            }
            if(my_web_flag == 0) {
                buf = strstr(g_buf, sniper);                 //判断接收的数据是什
                memcpy(g_buf, buf, 100);
                g_buf[100] = '\0';

                my_ret = admin(g_buf);
                if(my_ret == 0) {
                    send_link(client, (char *)html3);
                    my_web_flag = 1;
                    closesocket(client);
                    sleep(10);
                    continue  ;
                }
                send_link(client, (char *)html1); //重新返回原来的页面
            }
            else if (my_web_flag == 1) {
                buf = strstr(g_buf, sniper);                 //判断接收的数据是什么
                memcpy(g_buf, buf, 100);
                g_buf[100] = '\0';

                sdid_set(g_buf);
                send_link(client, (char *)html6);
                my_web_flag = 3;
            }
        }
        closesocket(client);
        sleep(10);
    }

exit:
    if (srv_sock >= 0) closesocket(srv_sock);

}
/*-----------------------------------------------------------------------------------*/
int  handle_user_rev(char *g_buf, char *str_1)
{
    char *buf;
    char a;
    p_dbg("    进入用户\r\n");
    p_dbg("\r\n 2222222:%s  \r\n", g_buf);
    buf = strstr(g_buf, "user=");                 //判断接收的数据是什么
    buf = buf + 5;
    memcpy(g_buf, buf, 50);
    g_buf[50] = '\0';
    p_dbg("\r\n 33333:%s \r\n", g_buf);
    sscanf(g_buf, "%[^&]", str);
    p_dbg("\r\n4444:%s  \r\n", str);
    return strcmp(str, admin_name);
}

/*-----------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------*/
int  handle_pwd_rev (char *g_buf, char *str_2 )
{
    char *buf;

    p_dbg("    进入密码\r\n");

    p_dbg("\r\n0000001:%s  \r\n", g_buf);
    buf = strstr(g_buf, str_2);                 //判断接收的数据是什么
    buf = buf + 4;
    memcpy(g_buf, buf, 50);
    g_buf[50] = '\0';

    p_dbg("\r\n0000001:%s  %d\r\n", g_buf, post_state);
    sscanf(g_buf, "%[^ ]", str);
    p_dbg("\r\n0000001:%s  \r\n", str);
    return strcmp(str, admin_pwd);
}

/*-----------------------------------------------------------------------------------*/
int admin (char *g_buf )
{   
	int my_ret;

    my_ret = handle_user_rev(g_buf, str_1);
    if(my_ret != 0) {
        p_dbg("\r\n 用户名错误\r\n");
        return -1 ;
    }
    p_dbg("\r\n 用户名正确\r\n");
    my_ret = handle_pwd_rev(g_buf, str_2);
    if(my_ret != 0) {
        p_dbg("\r\n 密码错误\r\n");
        return -1 ;
    }
    p_dbg("\r\n 密码正确\r\n");
    return 0;
}
/*-----------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------*/
int sdid_set (char *g_buf )
{   
	int my_ret;
    char *buf;
    char * b;
    char * a;

    memset(str, 0, strlen(my_str));
    memset(str, 0, strlen(my_str1));

    p_dbg("建立ap:%s, 密码:%s", web_cfg.ap.essid, web_cfg.ap.key);
    buf = strstr(g_buf, "user=");                 //判断接收的数据是什么
    buf = buf + 5;
    memcpy(g_buf, buf, 50);
    g_buf[50] = '\0';

    sscanf(g_buf, "%[^&]", my_str);

    my_str[strlen(my_str)] = '\0';
    a = (char *)mem_malloc(strlen(my_str) + 5);
    if(a == 0) {
        p_dbg("\r\n 申请失败\r\n");
    }
	
    p_dbg("\r\n 账号=%s\r\n", my_str);
    str_sta_name = my_str;
    p_dbg("\r\n 账号=%s\r\n", str_sta_name);

    buf = strstr(g_buf, str_2);                 //判断接收的数据是什么
    buf = buf + strlen(str_2);

    memcpy(g_buf, buf, 50);
    g_buf[50] = '\0';

    sscanf(g_buf, "%[^ ]", my_str1);

    my_str1[strlen(my_str1)] = '\0';
    p_dbg("\r\n 密码=%s\r\n", my_str1);

    str_sta_pwd = my_str1;

    p_dbg("\r\n 密码=%s\r\n", str_sta_pwd);
    p_dbg("\r\n********\r\n");
    p_dbg("\r\n 账号=%s\r\n", str_sta_name);
    p_dbg("\r\n 密码=%s\r\n", str_sta_pwd);

    default_web_cfg(&web_cfg);
    save_web_cfg(&web_cfg);

    return 0;
}
/*-----------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------*/
void web_server_thread(void* p);

void web_server_init(void)
{
    thread_create(web_server_thread,
                  0,
                  TASK_IMG_SEND_PRIO,
                  0,
                  TASK_IMG_SEND_STACK_SIZE,
                  "web_server_thread");
}
/*-----------------------------------------------------------------------------------*/
/* -------------------------------------------------------------------------- */

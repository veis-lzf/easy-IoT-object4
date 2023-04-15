#define DEBUG

#include "drivers.h"
#include "app.h"
#include "api.h"
#include "sha1.h"
#include "cfg80211.h"
#include "defs.h"
#include "type.h"
#include "types.h"


#include "lwip\sockets.h"
#include "lwip\netif.h"
#include "lwip\dns.h"
#include "lwip\api.h"
#include "lwip\tcp.h"
#include "lwip\dhcp.h"
#include "lwip\ip_addr.h"

#include "sys_misc.h"

#include "web_cfg.h"

#include "usr_cfg.h"
#include "test.h"

struct iot_work_struct  iot_work;

//int local_udp_server = -1, local_udp_client = -1;


//extern char command;
//extern mutex_t socket_mutex;
//extern unsigned char debug_s;

extern void handle_cmd(void);
extern int set_ipaddr(struct netif *p_netif, struct ip_addr *ipaddr, struct ip_addr *netmask, struct ip_addr *gw, struct ip_addr *dns) ;
int auto_get_ip(struct netif *p_netif);
/*
 *发送事件到主线程
 *
*/
void send_work_event(uint32_t event)        //释放二值信号量，唤醒对应的任务  
{
	iot_work.event_flag |= event;
	if(iot_work.event)
		wake_up(iot_work.event);
}

/*
	将本地ip设为默认值，ap模式下
*/
void set_default_ip()
{
	struct ip_addr ipaddr;
	struct ip_addr netmask;
	struct ip_addr gw;
	struct ip_addr dns;

	gw.addr = web_cfg.wifi_ip.gw;
	ipaddr.addr = web_cfg.wifi_ip.ip;
	netmask.addr = web_cfg.wifi_ip.msk;
	dns.addr = web_cfg.wifi_ip.dns;

	if(p_netif)
	set_ipaddr(p_netif, &ipaddr, &netmask, &gw, &dns);
}
#if 0
//创建一个本地UDP客户端，可以用它来发送任何数据到UDP服务器
//也可以用来发送UDP广播数据
void init_local_udp_client()
{
    int n = 1;
	local_udp_client = socket(AF_INET, SOCK_DGRAM, 0);
    if(local_udp_client == -1)
			p_err_fun;
		
    setsockopt(local_udp_client, SOL_SOCKET, SO_BROADCAST, (char *) &n, sizeof(n));
}
int udp_broadcast(uint16_t port, uint8_t *p_data, int len)
{
	struct sockaddr_in addr;
	//p_dbg_enter;
	//p_dbg("enter %s\n", __FUNCTION__);
	addr.sin_family = AF_INET;
	addr.sin_len = sizeof(struct sockaddr_in);
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr("255.255.255.255");

	mutex_lock(socket_mutex);
	len = sendto(local_udp_client, p_data, len, 0, (struct sockaddr*)&addr, sizeof(struct sockaddr));
	mutex_unlock(socket_mutex);
	//p_dbg("ret:%d", len);
	//p_dbg_exit;

	return len;
}

int udp_local_send(void *buff, int len)
{
	return udp_broadcast(12531, (uint8_t *)buff, len);
}

int bond_host(struct bond_req *req)
{
    int i;
    uint8_t empty_mac[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
    for(i = 0;i < MAX_BOND_NUM; i++)
    {
        if(memcmp(web_cfg.work_cfg.bond[i].mac, req->mac, 12) == 0)
            break;
    }

    if(i != MAX_BOND_NUM) //already bond
    {
        goto sucess; //over write
    };

    for(i = 0;i < MAX_BOND_NUM; i++)
    {
        if(memcmp(web_cfg.work_cfg.bond[i].mac, empty_mac, 12) == 0)
            break;
    }

    if(i == MAX_BOND_NUM) //full
        return -2;

sucess:
    if(i < MAX_BOND_NUM)
    {
        memcpy(&web_cfg.work_cfg.bond[i], req, sizeof(struct bond_req));
        return 0;
    }

    return -1;
}

void handle_local_udp_recv(uint8_t *data, int len)
{
    struct local_pkg_head *head = (struct local_pkg_head*)data;

    if(head->dir != MSG_DIR_APP2DEV)
    {
        p_err_fun;
        return;
    }
    switch(head->msg_id)
    {
        case LOCAL_MSG_DISC_REQ:
        {
            struct disc_res *res;
            struct local_pkg_head *res_head = (struct local_pkg_head *)mem_calloc(sizeof(struct local_pkg_head) + sizeof(struct disc_res), 1);
            if(res_head)
            {
            
                res_head->dir = MSG_DIR_DEV2APP;
                res_head->msg_id = LOCAL_MSG_DISC_RES;
                res_head->data_len = sizeof(struct disc_res);
                res = (struct disc_res *)(res_head + 1);

                memcpy(res->mac, iot_work.self_sn.mac, 12);
                memcpy(res->sn, iot_work.self_sn.sn, 32);
                
                udp_local_send((void*)res_head, sizeof(struct local_pkg_head) + sizeof(struct disc_res));

                mem_free(res_head);
            }
        }
        break;
        case LOCAL_MSG_BOND_REQ:
        if(head->data_len == sizeof(struct bond_req))
        {
            int ret;
            uint8_t res_buff[sizeof(struct local_pkg_head) + 4];
            struct local_pkg_head *res_head = (struct local_pkg_head *)res_buff;
            
            struct bond_req *req= (struct bond_req*)(head + 1);
            ret = bond_host(req);
            if(ret)
            {
                ret = 1;
            }

            res_head->dir = MSG_DIR_DEV2APP;
            res_head->msg_id = LOCAL_MSG_BOND_RES;
            res_head->data_len = 4;
            udp_local_send((void*)res_head, sizeof(struct local_pkg_head) + 4);

        }
        break;
        default:
        break;
    }
}
#endif

/*
 *主工作线程
 *
*/

//void init_select_server(void);

extern u8 interface_ret;

u8 wifi_ok=1;
u8 wifi_ret=1;

void main_process()
{
//  int ret;
    uint32_t pending_event;
    memset(&iot_work, 0, sizeof(struct iot_work_struct));
    iot_work.event = init_event();

    p_dbg("main_process func");
    
       { p_dbg("Connect to:%s, PW:%s", web_cfg.sta.essid, web_cfg.sta.key);
        wifi_set_mode(MODE_STATION);
        indicate_led_twink_start(10000); //D4 10S 闪烁一次，表示WIFI目前工作在STA模式
        wifi_connect(web_cfg.sta.essid, web_cfg.sta.key);     // 连接wifi    wifi的名字   和密码

        if (is_wifi_connected())
        {
            p_dbg("wifi connect ok");
            if(web_cfg.wifi_ip.auto_get){    
                auto_get_ip(p_netif);
            }
           

        }

    }
    
     IND3_ON;

    while(1)
    {
        //等待事件通知，1s超时
        wait_event_timeout(iot_work.event, 1000);
        pending_event = iot_work.event_flag;
        iot_work.event_flag = 0;

      if(pending_event & INTERFACE_MODE_SHIBIE)
        {
            printf("\r\n****** 进入识别模式 *******\r\n");
//          LCD_Init();		      //初始化lcd屏幕 
//          LCD_Clear(WHITE);
//            delay_ms(50);
             LCD_Init();
        
            rgb565_test();
        }
        

#if SUPPORT_WIFI
        
        
      if(pending_event & MODE_DETECT_CHANGE_AP)
        {
             wifi_ok=0;
             LCD_Init();
             IND1_OFF; 
        
             show_pic_init(2);
             Show_Str(100,120,300,24,"打开手机浏览器，",24,0);	
             Show_Str(150,150,300,24,"输入192.168.0.37",24,0);
             Show_Str(200,180,200,24,"配置路由信息:",24,0);             

            struct ip_addr ipaddr;
            struct ip_addr netmask;
            struct ip_addr gw;
            struct ip_addr dns;
            p_dbg("建立ap:%s, 密码:%s", web_cfg.ap.essid, web_cfg.ap.key);
            //D4 20S 闪烁一次，表示WIFI目前工作在AP模式
            indicate_led_twink_start(20000);
            wifi_set_mode(MODE_AP);
            wifi_ap_cfg(web_cfg.ap.essid, web_cfg.ap.key, KEY_WPA2, web_cfg.ap.channel, 4);

            gw.addr = web_cfg.wifi_ip.gw;
            ipaddr.addr = web_cfg.wifi_ip.ip;
            netmask.addr = web_cfg.wifi_ip.msk;
            dns.addr = web_cfg.wifi_ip.dns;
            if(p_netif)
             set_ipaddr(p_netif, &ipaddr, &netmask, &gw, &dns);
        }
        
        
        if(pending_event & MODE_DETECT_CHANGE_STA)
        {   wifi_ret=1;
            p_dbg("Connect to:%s, PW:%s", web_cfg.sta.essid, web_cfg.sta.key);
            wifi_set_mode(MODE_STATION);
            indicate_led_twink_start(10000); //D4 10S 闪烁一次，表示WIFI目前工作在STA模式
            wifi_connect(web_cfg.sta.essid, web_cfg.sta.key);     // 连接wifi    wifi的名字   和密码

            if (is_wifi_connected())
            {
                p_dbg("wifi connect ok");
                
                if(web_cfg.wifi_ip.auto_get){
                    auto_get_ip(p_netif);
                }
                
                      
            }
        }


#endif
        
        if(pending_event & INTERFACE_MODE_MOST)              //主界面 
        {    
            printf("\r\n****** 进入主界面模式 *******\r\n");
//             LCD_Init();
             DCMI_Stop();
//             LCD_Clear(WHITE);
//             delay_ms(50);
             LCD_Init();
             show_pic_init(1);
             Show_Str(200,130,150,24,"智能识别器",24,0);
             Show_Str(10,280,300,24,"WIFI设置:",24,0);	
             Show_Str(380,280,100,24,"识别模式:",24,0); 
             interface_ret=0; 
               
        }
         if(wifi_ok){
        p_dbg("Connect to:%s, PW:%s", web_cfg.sta.essid, web_cfg.sta.key);
        wifi_set_mode(MODE_STATION);
        indicate_led_twink_start(10000); //D4 10S 闪烁一次，表示WIFI目前工作在STA模式
        wifi_connect(web_cfg.sta.essid, web_cfg.sta.key);     // 连接wifi    wifi的名字   和密码

        if (is_wifi_connected())
        {
            p_dbg("wifi connect ok");
            if(web_cfg.wifi_ip.auto_get){    
                auto_get_ip(p_netif);
            }
           

        }
       
          
    }
    }
}


#if 0
int is_led_ctrl_cmd(char *buff)
{
    if ((buff[0] == 0xaa) && (buff[1] == 0x55) && (buff[2] == 1 || buff[2] == 2))
        return 1;

    return 0;
}

int is_udp_socket(int num)
{
    struct lwip_sock *sock;

    sock = get_socket(num);
    if (!sock || !sock->conn)
    {
        return 0;
    }

    if (sock->conn->type == NETCONN_UDP)
    {
        return 1;
    }

    return 0;

}

int is_dhcp_socket(int num)
{
    uint16_t tmp_port = 0;
    struct lwip_sock *sock;

    sock = get_socket(num);
    if (!sock || !sock->conn)
    {
        return 0;
    }

    if (sock->conn->type == NETCONN_UDP)
    {
        tmp_port = sock->conn->pcb.udp->local_port; //server

        if (tmp_port == DHCP_SERVER_PORT)
            return 1;
    }
    return 0;
}
/*
int is_onenet_socket(int num)
{
    uint16_t tmp_port = 0;
    struct lwip_sock *sock;

    sock = get_socket(num);
    if (!sock || !sock->conn)
    {
        return 0;
    }

    if (sock->conn->type == NETCONN_TCP)
    {
        tmp_port = sock->conn->pcb.tcp->remote_port; //server

        if (tmp_port == 876)
            return 1;
    }
    return 0;

}
*/
int is_web_socket(int num)
{
    uint16_t tmp_port = 0;
    struct lwip_sock *sock;

    sock = get_socket(num);
    if (!sock || !sock->conn)
    {
        return 0;
    }

    if (sock->conn->type == NETCONN_TCP)
    {
        tmp_port = sock->conn->pcb.tcp->local_port; //server

        if (tmp_port == 80)
            return 1;
    }
    return 0;
}


/*
 *selcet模式接收数据的例子，可以监控多个socket的数据接收
 *local_udp_server会负责广播数据的处理
 */
DECLARE_MONITOR_ITEM("tcp totol recv", tcp_totol_recv);
#define TCP_RCV_SIZE 1024

void select_thread(void *arg)
{
    int i, size, retval, select_size;
    struct lwip_sock *sock;
    char *tcp_rcv_buff;
    struct timeval tv;
    fd_set rfds;
    local_udp_server = udp_create_server(12531);
    if(local_udp_server == -1)
        p_err_fun;

    tcp_rcv_buff = (char*)mem_malloc2(TCP_RCV_SIZE + 4);
    while (1)
    {
        FD_ZERO(&rfds);
        //add any fd(TCP or TCP) into this set, to read data
        FD_SET(local_udp_server, &rfds);
        
        select_size = local_udp_server + 1;


        tv.tv_sec = 5;
        tv.tv_usec = 0;
        retval = select(select_size, &rfds, NULL, NULL, &tv);
        if ((retval ==  - 1) || (retval == 0))
        {
            sleep(50); //内存错误
        }
        else
        {
            if (retval)
            {
                for (i = 0; i < select_size; i++)
                {
                    if (FD_ISSET(i, &rfds))
                    {
                        struct sockaddr remote_addr;
                        mutex_lock(socket_mutex);

                        #if 1   //使用recvfrom接收
                        retval = sizeof(struct sockaddr);
                        size = recvfrom(i, tcp_rcv_buff, TCP_RCV_SIZE, MSG_DONTWAIT, &remote_addr, (socklen_t*) &retval);
                        /*  
                        p_dbg("rcv from:%d,%d,%d,%d,%d,%d\n",udp_remote_addr.sa_data[0],remote_addr.sa_data[1],
                        remote_addr.sa_data[2],
                        remote_addr.sa_data[3],
                        remote_addr.sa_data[4],
                        remote_addr.sa_data[5]  );*/
                        #else //使用recv接收
                        size = recv(i, tcp_rcv_buff, TCP_RCV_SIZE, MSG_DONTWAIT);
                        #endif
                        mutex_unlock(socket_mutex);
                        if (size ==  - 1)
                        {
                            if (errno == EWOULDBLOCK || errno == ENOMEM) 
                            {
                                //出现这两种情况不代表socket断开,所以不能关闭socket,不过很少出现这种情况:)
                                p_err("tcp_recv err:%d,%d\n", i, errno);
                                sleep(10);
                            }
                            else
                            {
                                p_err("tcp_recv fatal err:%d,%d\n", i, errno);
                                close_socket(i);
                                FD_CLR(i, &rfds);
                            }
                            continue;
                        }
                        if (size == 0)
                        //0代表连接已经关闭
                        {
                            if (errno != 0)
                            {
                                p_err("tcp_client_recv %d err1:%d\n", i, errno);
                                close_socket(i);
                            }
                            else
                                p_err("rcv 0 byte?\n");
                            continue;
                        }

                        //p_dbg("socket:%d rcv:%d\n", i, size);
                        ADD_MONITOR_VALUE(tcp_totol_recv, size);

                        if(local_udp_server == i)
                        {
                            handle_local_udp_recv((unsigned char*)tcp_rcv_buff, size);
                        }
                        else
                            handle_test_recv(i, (unsigned char*)tcp_rcv_buff, size);
                    } //end of if(FD_ISSET(i,&rfds))
                } //end of for(i = 0; i < select_size; i++)
            }
        }
    }
}

void init_select_server(void)
{
    thread_create(select_thread, 0, TASK_TCP_RECV_PRIO, 0, TASK_TCP_RECV_STACK_SIZE, "tcp_recv_thread");
}
#endif

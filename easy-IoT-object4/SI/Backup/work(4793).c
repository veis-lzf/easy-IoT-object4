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
 *�����¼������߳�
 *
*/
void send_work_event(uint32_t event)        //�ͷŶ�ֵ�ź��������Ѷ�Ӧ������  
{
	iot_work.event_flag |= event;
	if(iot_work.event)
		wake_up(iot_work.event);
}

/*
	������ip��ΪĬ��ֵ��apģʽ��
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

/*
 *�������߳�
 *
*/
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
    { 
       p_dbg("Connect to:%s, PW:%s", web_cfg.sta.essid, web_cfg.sta.key);
        wifi_set_mode(MODE_STATION);
        indicate_led_twink_start(10000); //D4 10S ��˸һ�Σ���ʾWIFIĿǰ������STAģʽ
        wifi_connect(web_cfg.sta.essid, web_cfg.sta.key);     // ����wifi    wifi������   ������

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
        //�ȴ��¼�֪ͨ��1s��ʱ
        wait_event_timeout(iot_work.event, 1000);
        pending_event = iot_work.event_flag;
        iot_work.event_flag = 0;

      	if(pending_event & INTERFACE_MODE_SHIBIE)
      	{
        	printf("\r\n****** ����ʶ��ģʽ *******\r\n");
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
            Show_Str(100,120,300,24,"���ֻ��������",24,0);	
            Show_Str(150,150,300,24,"����192.168.0.37",24,0);
            Show_Str(200,180,200,24,"����·����Ϣ:",24,0);             

            struct ip_addr ipaddr;
            struct ip_addr netmask;
            struct ip_addr gw;
            struct ip_addr dns;
            p_dbg("����ap:%s, ����:%s", web_cfg.ap.essid, web_cfg.ap.key);
            //D4 20S ��˸һ�Σ���ʾWIFIĿǰ������APģʽ
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
        {   
        	wifi_ret = 1;
            p_dbg("Connect to:%s, PW:%s", web_cfg.sta.essid, web_cfg.sta.key);
            wifi_set_mode(MODE_STATION);
            indicate_led_twink_start(10000); //D4 10S ��˸һ�Σ���ʾWIFIĿǰ������STAģʽ
            wifi_connect(web_cfg.sta.essid, web_cfg.sta.key);     // ����wifi    wifi������   ������

            if (is_wifi_connected())
            {
                p_dbg("wifi connect ok");
                
                if(web_cfg.wifi_ip.auto_get){
                    auto_get_ip(p_netif);
                } 
            }
        }

#endif
        
        if(pending_event & INTERFACE_MODE_MOST)              //������ 
        {    
            printf("\r\n****** ����������ģʽ *******\r\n");
            DCMI_Stop();
            LCD_Init();
            show_pic_init(1);
            Show_Str(200,130,150,24,"����ʶ����",24,0);
            Show_Str(10,280,300,24,"WIFI����:",24,0);	
            Show_Str(380,280,100,24,"ʶ��ģʽ:",24,0); 
            interface_ret=0; 
               
        }
         if(wifi_ok)
		 {
        	p_dbg("Connect to:%s, PW:%s", web_cfg.sta.essid, web_cfg.sta.key);
        	wifi_set_mode(MODE_STATION);
        	indicate_led_twink_start(10000); //D4 10S ��˸һ�Σ���ʾWIFIĿǰ������STAģʽ
        	wifi_connect(web_cfg.sta.essid, web_cfg.sta.key);     // ����wifi    wifi������   ������

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

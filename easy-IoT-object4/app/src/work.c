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
 *主工作线程
 *
*/
extern u8 interface_ret;
u8 wifi_ok=1;
u8 wifi_ret=1;

void main_process()
{
    uint32_t pending_event;
	
	p_dbg("main_process func");
    memset(&iot_work, 0, sizeof(struct iot_work_struct));
    iot_work.event = init_event();
	IND3_ON;

	// 进入主循环
    while(1)
    {
        // 等待事件通知，1s超时
        wait_event_timeout(iot_work.event, 1000);
        pending_event = iot_work.event_flag;
        iot_work.event_flag = 0;

      	if(pending_event & INTERFACE_MODE_SHIBIE)
      	{
        	p_dbg("****** 进入识别模式 *******");
          	rgb565_test();
			
      	}
        
#if SUPPORT_WIFI
      if(pending_event & MODE_DETECT_CHANGE_AP)
      {
            DCMI_Stop();
            LCD_Clear(WHITE);
            Show_Str(100,120,300,24,"打开手机浏览器，",24,0);	
            Show_Str(150,150,300,24,"输入192.168.0.37",24,0);
            Show_Str(200,180,200,24,"配置路由信息:",24,0);             

            struct ip_addr ipaddr;
            struct ip_addr netmask;
            struct ip_addr gw;
            struct ip_addr dns;
            p_dbg("建立ap:%s, 密码:%s", web_cfg.ap.essid, web_cfg.ap.key);
            //D1 500ms 闪烁一次，表示WIFI目前工作在AP模式
            indicate_led_twink_start(500);
            wifi_set_mode(MODE_AP);
            wifi_ap_cfg(web_cfg.ap.essid, web_cfg.ap.key, KEY_WPA2, web_cfg.ap.channel, 4);

            gw.addr = web_cfg.wifi_ip.gw;
            ipaddr.addr = web_cfg.wifi_ip.ip;
            netmask.addr = web_cfg.wifi_ip.msk;
            dns.addr = web_cfg.wifi_ip.dns;
            if(p_netif)
            	set_ipaddr(p_netif, &ipaddr, &netmask, &gw, &dns);
			
			indicate_led_twink_stop();
        }
        
        
        if(pending_event & MODE_DETECT_CHANGE_STA)
        {   
        	wifi_ret = 1;
            p_dbg("Connect to:%s, PW:%s", web_cfg.sta.essid, web_cfg.sta.key);
            wifi_set_mode(MODE_STATION);
            indicate_led_twink_start(1000); //D1 1S 闪烁一次，表示WIFI目前工作在STA模式
            wifi_connect(web_cfg.sta.essid, web_cfg.sta.key);     // 连接wifi    wifi的名字   和密码

            if (is_wifi_connected())
            {
                p_dbg("wifi connect ok");
                
                if(web_cfg.wifi_ip.auto_get){
                    auto_get_ip(p_netif);
                }
				indicate_led_twink_stop();
				IND1_ON;
            }
        }
#endif
        if(pending_event & INTERFACE_MODE_MOST) // 主界面 
        {    
            p_dbg("****** 进入主界面模式 *******");
            DCMI_Stop();
            LCD_Clear(WHITE);
			Show_Str(200,130,150,24,(unsigned char *)"智能识别器",24,0);
			Show_Str(10,280,300,24,(unsigned char *)"WIFI设置",24,0);  
			Show_Str(380,280,150,24,(unsigned char *)"识别模式",24,0);	
            interface_ret  = 0;
		#if SUPPORT_WIFI
			if (!is_wifi_connected()) // 每次返回主界面都检查WIFI是否断开连接
				wifi_ok = 1;
		#endif
        }
		
		#if SUPPORT_WIFI
	        if(wifi_ok) // 如果WIFI连接失败，重新连接
			{
				send_work_event(MODE_DETECT_CHANGE_STA);
	    	}
		#endif
    }
}

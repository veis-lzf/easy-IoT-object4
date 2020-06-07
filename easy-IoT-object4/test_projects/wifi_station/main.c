#define DEBUG

#include "drivers.h"
#include "app.h"
#include "api.h"

#include "test.h"
#include "dhcpd.h"
#include "web_cfg.h"
#include <cctype>

#include "cfg80211.h"
#include "defs.h"
#include "type.h"
#include "types.h"

#include "moal_sdio.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "wpa.h"

#include "moal_main.h"
#include "moal_uap.h"
#include "moal_uap_priv.h"
#include "mlan_ioctl.h"

/*
* 版本说明在doc目录
*/
#define VIRSION		"V2.4"

int read_firmware(u32 offset, u32 len, u8 * pbuf)
{
	p_err_miss;
	return 0;
}

char command;
int dbg_level = 1;
int monitor_enable = 0;

struct netif *p_netif = (struct netif *)1;

/*
*空函数,避免编译错误
*/
void send_work_event(uint32_t event)
{

}

/*
*空函数,避免编译错误
*/
void show_tcpip_info(struct netif *p_netif)
{
}

/*
*空函数,避免编译错误
*/
void dev_monitor_task(void *arg)
{
	while(1)
		sleep(1000);
}

/*
*这是wifi接收数据的接口,如果有客户端连接到本ap,会发送tcpip层的数据包过来
*这里以16进制打印
*/
void ethernetif_input(struct netif *netif,void *p_buf,int size)
{
	p_dbg("recv %d byte", size);
	dump_hex("data", p_buf, size);
}

void main_thread(void *pdata)
{
	int ret;
	#ifdef DEBUG
	RCC_ClocksTypeDef RCC_ClocksStatus;
	#endif
	
	driver_misc_init(); //初始化一些杂项(驱动相关)
	usr_gpio_init(); //初始化GPIO
	//IND3_ON; //点亮LED
#ifdef OS_UCOS
#ifdef UCOS_V3
	OSStatTaskCPUUsageInit((OS_ERR*)&ret);
#else
	OSStatInit(); //初始化UCOS状态
#endif
#endif
	uart1_init(); //初始化串口1
	uart3_init();

	//打印MCU总线时钟
#ifdef DEBUG
	RCC_GetClocksFreq(&RCC_ClocksStatus);
	p_dbg("SYSCLK_Frequency:%d,HCLK_Frequency:%d,PCLK1_Frequency:%d,PCLK2_Frequency:%d,ADCCLK_Frequency:%d\n", 
		RCC_ClocksStatus.SYSCLK_Frequency, 
		RCC_ClocksStatus.HCLK_Frequency, 
		RCC_ClocksStatus.PCLK1_Frequency, 
		RCC_ClocksStatus.PCLK2_Frequency, 
		0);
#endif

/*
*WIFI例程之station,连接路由器
*
*/

	init_work_thread();	//初始化工作线程
	
	ret = init_wifi();//初始化WIFI芯片
	if(ret == 0)
	{
		//init_monitor(); //初始化monitor模块,必须在init_wifi之后调用
	
	}else{
		p_err("init wifi faild!"); 
	}
	p_dbg("设置WIFI模式为STATION");
	wifi_set_mode(MODE_STATION);
	p_dbg("开始连接到路由器");
	ret = wifi_connect("TP-LINK_2AA6", 	//路由器ssid
		"zsl540535" 	//路由器密码
		);
	if(ret == 0)
		p_dbg("WIFI连接成功");
	else
		p_dbg("WIFI连接失败");

	//IND3_OFF; 
	
	//获取本地MAC地址
	{
		uint8_t mac[6];
		struct station_info sinfo;
		wifi_get_mac(mac);
		dump_hex("本地MAC", mac, 6);
	}
	
	while(1)
	{
		sleep(1000);
	}
}

int main(void)
{
#ifdef OS_UCOS
#ifdef UCOS_V3
	OS_ERR err;
	OSInit(&err);
#else
	OSInit();
#endif
#endif
	_mem_init(); //初始化内存分配
	msg_q_init();//
	thread_create(main_thread, 0, TASK_MAIN_PRIO, 0, TASK_MAIN_STACK_SIZE, "main_thread");
#ifdef OS_UCOS
#ifdef UCOS_V3
	OSStart(&err);
#else
	OSStart();
#endif
#endif
#ifdef OS_FREE_RTOS
	vTaskStartScheduler();
#endif
	return 0;
}

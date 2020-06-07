#define DEBUG

#include "drivers.h"
#include "app.h"
#include "api.h"

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
	//p_dbg("recv %d byte", size);
	//dump_hex("data", p_buf, size);
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
*WIFI例程之创建AP
*这里以16进制打印
*/

	init_work_thread();	//初始化工作线程
	
	ret = init_wifi();//初始化WIFI芯片
	if(ret == 0)
	{
		//init_monitor(); //初始化monitor模块,必须在init_wifi之后调用
	
	}else{
		p_err("init wifi faild!"); 
	}

	wifi_set_mode(MODE_AP);
	wifi_ap_cfg("xrf_ap", 	//ssid
		"12345678", 	//key
		KEY_WPA2, 		//加密方式,可选值KEY_NONE, KEY_WEP, KEY_WPA, KEY_WPA2
		6, 	//频道,可选1-13
		4		//最多允许的客户端1-6
		);
	
	//IND3_OFF; 
	//下面每3秒钟查询一次客户端情况
	while(1)
	{
		int i;
		int rssi = 0xffffff00;
		mlan_ds_sta_list *p_list;

		p_list = (mlan_ds_sta_list *)mem_calloc(sizeof(mlan_ds_sta_list), 1);
		if(p_list){
			wifi_get_sta_list(p_list);
			p_dbg("已连接设备数:%d", p_list->sta_count);
			for(i = 0; i < p_list->sta_count; i++)
			{
				p_dbg("sta %d", i);
				rssi = rssi | p_list->info[i].rssi;
				p_dbg("	rssi %d", rssi);
				dump_hex("	mac", p_list->info[i].mac_address, 6);

				//通过mac地址,我们可以获取station的更多信息
				{
					struct station_info sinfo;
		
					ret = wifi_get_stats(p_list->info[i].mac_address, &sinfo);
					if(ret == 0)
						p_dbg("wifi stats, rssi:%d", sinfo.signal);
				}
			}
			
			mem_free(p_list);
		}
		p_dbg("\r\n\r\n\r\n");

		sleep(3000);
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

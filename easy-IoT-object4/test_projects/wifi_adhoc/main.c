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

const uint8_t test_buff[] = {
//下面是一个广播包	
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 	//这是目的地址
	0x40, 0x8d, 0x5c, 0x36, 0xee, 0x06, 	//这是源地址,这里应该填本地低mac地址,这里就随意填一个
	0x08, 0x06,  //IP报文类型,这里为ARP包
	1,2,3,4,5,6		//这里随便放点数据
};

void mac_data_xmit()
{
	struct sk_buff *tx_skb;
	struct pbuf *q;
	char *p802x_hdr;
	
	if(!is_hw_ok())
		return;

	tx_skb = alloc_skb(sizeof(test_buff) + MLAN_MIN_DATA_HEADER_LEN + sizeof(mlan_buffer), 0);
	if(tx_skb == 0)
		return;

	skb_reserve(tx_skb, MLAN_MIN_DATA_HEADER_LEN + sizeof(mlan_buffer));
	
	p802x_hdr = (char*)tx_skb->data;

	memcpy(p802x_hdr, test_buff, sizeof(test_buff));

	tx_skb->len = sizeof(test_buff);
	p802x_hdr = (char*)tx_skb->data;

	//dump_hex("s", tx_skb->data, tx_skb->len);

	if(wifi_dev && wifi_dev->netdev_ops)
		wifi_dev->netdev_ops->ndo_start_xmit((void*)tx_skb, wifi_dev);
	
	return;
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
*WIFI例程之adhoc,自组织对等网,不需要AP就能实现两个(或多个)设备间通信)
*
*需要两块板子才能测试,或者一块板子+电脑的WIFI网卡也能连接
*两块板子分别上电后会自动组网,并且每隔两秒给对方发送一个数据包,这里没有用到tcpip协议栈
*所以数据包内容不必完全按照tcpip协议
*/

	init_work_thread();	//初始化工作线程
	
	ret = init_wifi();//初始化WIFI芯片
	if(ret == 0)
	{
		//init_monitor(); //初始化monitor模块,必须在init_wifi之后调用
	
	}else{
		p_err("init wifi faild!"); 
	}
	p_dbg("设置WIFI模式为adhoc");
	wifi_set_mode(MODE_STATION);
	wifi_set_mode(MODE_ADHOC);
	wifi_join_adhoc("test_adhoc", //网络名称
	"1234567890abc", 	//密码,注意密码长度只能5或13字节
	6);

	//IND3_OFF; 
	
	while(1)
	{
		sleep(2000);
		mac_data_xmit();
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

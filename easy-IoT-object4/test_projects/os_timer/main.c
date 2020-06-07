#define DEBUG

#include "drivers.h"
#include "app.h"
#include "api.h"

#include "test.h"
#include "dhcpd.h"
#include "web_cfg.h"
#include <cctype>


#define VIRSION		"T1.0"

char command;
int dbg_level = 1;

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

timer_t timer1, timer2;

void test_timer_cb1(void *pdata)
{
	p_dbg("定时器1回调");
	
}

void test_timer_cb2(void *pdata)
{
	p_dbg("定时器2回调");
}


void main_thread(void *pdata)
{
	int cnt = 0;
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
*os测试例程之软件定时器
*
*
*创建两个定时器,一个一次性定时器,一个循环定时器
*这里请注意init_timer_tasklet这个函数,详情请看函数原型(位于task.c)
*/
	init_timer_tasklet();		

	timer1 = timer_setup(1000, 	//定时时间
			0, 	//只定时一次
			test_timer_cb1,	//回调函数
			"参数1"); //参数
	if(!timer1)
		p_err("timer1 创建失败");
	else
		p_dbg("timer1 创建成");
	
	timer2 = timer_setup(2000, 	//定时时间
			1, 	//循环定时触发
			test_timer_cb2,	//回调函数
			"参数2"); //参数
	if(!timer1)
		p_err("timer2 创建失败");
	else
		p_dbg("timer2 创建成");
	
	add_timer(timer1);
	add_timer(timer2);
	
	sleep(10000);	//等待10s
	p_dbg("修改timer2 定时时间成1秒");
	del_timer(timer2);   //del_timer是停止定时器,timer_free是删除释放定时器
	mod_timer(timer2, 1000); //修改定时时间为1s
	while(1)
	{
		sleep(2000);
		//p_dbg("%s %d", __FUNCTION__, cnt++);
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

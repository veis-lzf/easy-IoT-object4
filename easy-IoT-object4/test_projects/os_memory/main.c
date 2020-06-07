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

void test_thread1(void *pdata)
{
	int cnt = 0;
	p_dbg("进入线程:%s", (char*)pdata);	
	sleep(500); //延时500ms
	while(cnt < 100)
	{
		sleep(1000);
		p_dbg("%s %d", __FUNCTION__, cnt++);  //打印函数名和计数
	}
	p_dbg("退出线程:%s", (char*)pdata);	
	thread_exit(thread_myself());
}

void test_thread2(void *pdata)
{
	int cnt = 0;
	p_dbg("进入线程:%s", (char*)pdata);	
	
	while(cnt < 100)
	{
		sleep(1000);
		p_dbg("%s %d", __FUNCTION__, cnt++);  //打印函数名和计数
	}
	p_dbg("退出线程:%s", (char*)pdata);	
	thread_exit(thread_myself());
}

void main_thread(void *pdata)
{
	int cnt = 0;
	char *p_m;
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
*os测试例程之内存管理
*
*
*
*
*/
//分配一块150byte的内存, 请注意实际分配的时候是4byte对其的
	p_m = mem_malloc(150);
	
	p_dbg("内存区1,总量:%d, 剩余:%d", mem_get_size(), mem_get_free());
	p_dbg("内存区2,总量:%d, 剩余:%d", mem_get_size2(), mem_get_free2());
	
//下面分配一块1000byte的内存,然后释放
	p_m = mem_malloc(1000);
	if(!p_m)
		p_err("no memory");
	else{
		
		p_dbg("start show memory");
		mem_slide_check(1);	//打印内存分配情况, 我们可以看到上面分配的两块内存
		p_dbg("end show memory");
		
		p_dbg("free it");
		mem_free(p_m);
	}
	

//这里测试内存溢出检测
//分配一个64byte的数组,然后对数组中的65byte赋值,看看串口会打印什么信息
	p_m = mem_malloc(64);
	memset(p_m, 4, 65);
	
	while(1)
	{
		sleep(1000);
		
		p_dbg("%s %d", __FUNCTION__, cnt++);
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

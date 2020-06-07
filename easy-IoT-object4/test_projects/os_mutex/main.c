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

mutex_t test_mutex = 0;

void test_thread1(void *pdata)
{
	int cnt = 0;
	p_dbg("进入线程:%s", (char*)pdata);	
	while(cnt < 100)
	{
		sleep(10);  //线程1的优先级高一些,我们设定每10ms打印一次
		mutex_lock(test_mutex);  
		p_dbg("[1234567890]");  //打印函数名和计数
		mutex_unlock(test_mutex);
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
		mutex_lock(test_mutex);    //锁住,你可以将这里的锁注释掉看看效果
		p_dbg("<abcdefghijklmnopqrstuvwxyz>");  //打印函数名和计数
		mutex_unlock(test_mutex);  //解锁
	}
	p_dbg("退出线程:%s", (char*)pdata);	
	thread_exit(thread_myself());
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
*os测试例程之互斥
*
*
*创建两个线程并以最快的速度循环打印一串字符,打印100次秒后退出线程
*
*/
	test_mutex = mutex_init("名字"); //创建一个互斥变量
	if(!test_mutex)
		p_err("test_mutex 创建失败");
	else
		p_dbg("test_mutex 创建成");

	cnt = thread_create(test_thread1, 
		"参数1", 	
		TASK_MAIN_PRIO + 1, 	//线程优先级,请注意ucos2环境下每个线程的优先级不能相同
		0, 	//线程堆栈指针,为0代表自动分配
		256, //堆栈大小
		"线程名称1");
	if(cnt < 0)
		p_err("线程 1 创建失败");
	else
		p_dbg("线程 1 创建成功,线程ID:%d", cnt);
		
	cnt = thread_create(test_thread2, 
		"参数2", 
		TASK_MAIN_PRIO + 2, 
		0, 
		256, 
		"线程名称2");
	if(cnt < 0)
		p_err("线程 2 创建失败");
	else
		p_dbg("线程 2 创建成功,线程ID:%d", cnt);
	
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

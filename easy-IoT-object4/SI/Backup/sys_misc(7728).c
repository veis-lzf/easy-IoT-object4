#define DEBUG

#include "drivers.h"
#include "app.h"
#include "api.h"

#include "test.h"
#include "lwip/netif.h"
#include <absacc.h>

timer_t ind1_timer;


void delay_1us()
{
	int i = 50;
	while (i--)
		;
}

void delay_us(uint32_t us)
{
	while (us--)
		delay_1us();
}
void delay_ms(uint32_t us)
{
	while (us--)
		delay_us(1000);
}


uint8_t led_value = 0;
uint8_t led_bright_value = 10;



void led_switch(uint8_t value)
{
	p_dbg("led_switch:%x", value);
	led_value = value;
	
}

//不再支持亮度
void led_bright(uint8_t value)
{
	p_dbg("led_bright:%d", value);
	if (value > 99)
		value = 99;

	led_bright_value = value / 10;
}


SWITCH_EVENT g_switch = SWITCH_EVENT_OFF;
void button_stat_callback()
{
	static uint8_t last_stat = 1;
	BUTTON_EVENT button_event = BUTTON_EVENT_IDLE;

	if (BUTTON_STAT && !last_stat)
	{
		last_stat = BUTTON_STAT;
		button_event = BUTTON_EVENT_UP;
		//p_dbg("button up");
	}

	if (!BUTTON_STAT && last_stat)
	{
		last_stat = BUTTON_STAT;
		button_event = BUTTON_EVENT_DOWN;
		//p_dbg("button down");
	}

	if (button_event == BUTTON_EVENT_UP)
	{
		if(g_switch == SWITCH_EVENT_OFF)
			g_switch = SWITCH_EVENT_ON;
		else
			g_switch = SWITCH_EVENT_ON;
#if SUPPORT_AUDIO
		//camera_button_event(g_switch);
		audio_button_event(g_switch);
#endif
	}
}


void assert_failed(uint8_t *file, uint32_t line)
{
	p_err("assert_failed in:%s,line:%d \n", file ? file : "n", line);
	while (1)
		;
}
void ind2_status_set(int on)
{
    if(on)
		IND2_ON;
	else
		IND2_OFF;
}



void ind2_timer_callback(void *arg)
{
	static bool led_stat = FALSE;

	led_stat = !led_stat;

	if(led_stat)
		IND2_ON;
	else
		IND2_OFF;
}

void ind4_timer_callback(void *arg)
{
	static bool led_stat = FALSE;

	led_stat = !led_stat;


}

void indicate_led_twink_start(int fre)
{
	// 
	del_timer(ind1_timer);
	mod_timer(ind1_timer, fre);
	
	ind1_timer = timer_setup(fre, 1, ind4_timer_callback, NULL);
	add_timer(ind1_timer);
}

void indicate_led_twink_stop()
{
	del_timer(ind1_timer);
	IND2_OFF;
}

/**
 * @brief 定义两个定时器用于led的闪烁
 */
void misc_init()
{
	ind1_timer = timer_setup(10000, 1, ind4_timer_callback, NULL);
	add_timer(ind1_timer);
}

void show_sys_info(struct netif *p_netif)
{
	p_dbg("mac:%02x-%02x-%02x-%02x-%02x-%02x", p_netif->hwaddr[0], p_netif->hwaddr[1], p_netif->hwaddr[2], p_netif->hwaddr[3], p_netif->hwaddr[4], p_netif->hwaddr[5]);
	show_tcpip_info(p_netif);
}

void soft_reset()
{
	p_err("system well reset\n");
	NVIC_SystemReset();
  	__DSB();  
	while (1);
}


#ifdef OS_UCOS
#if OS_APP_HOOKS_EN > 0u
void App_TaskCreateHook(OS_TCB *ptcb)
{
	ptcb = ptcb;
}

void App_TaskDelHook(OS_TCB *ptcb)
{
	ptcb = ptcb;
}

void App_TaskReturnHook(OS_TCB *ptcb)
{
	ptcb = ptcb;
}

void App_TCBInitHook(OS_TCB *ptcb)
{
	ptcb = ptcb;
}

void App_TaskSwHook(void){

}

void App_TimeTickHook(void){}

//uC/OS-II Stat线程中调用此函数，每100MS一次
void App_TaskStatHook()
{
	#if USE_MEM_DEBUG
	mem_slide_check(0);
	#endif
	//button_stat_callback();
}

#endif
#endif

#ifdef OS_FREE_RTOS
void vApplicationIdleHook( void )
{
	#if USE_MEM_DEBUG
	mem_slide_check(0);
	#endif
}
#endif

extern uint32_t drvdbg;
extern uint32_t drvdbg_save;
void switch_dbg(void)
{
	if(dbg_level)
	{
		p_dbg("close dbg");
	}
	dbg_level = !dbg_level;
	if(dbg_level)
	{
		p_dbg("open dbg");
#ifdef DEBUG_LEVEL1
		drvdbg = drvdbg_save;
#endif
	}
#ifdef DEBUG_LEVEL1	
	else
		drvdbg = 0;
#endif
}

void usr_gpio_init()
{
        //BUTTON status
	gpio_cfg((uint32_t)BUTTON_PORT, BUTTON_PIN, GPIO_Mode_IPU);
	
	//LED2
	gpio_cfg((uint32_t)IND1_GROUP, IND1_PIN, GPIO_Mode_Out_PP);
    gpio_cfg((uint32_t)IND2_GROUP, IND2_PIN, GPIO_Mode_Out_PP);
    gpio_cfg((uint32_t)IND3_GROUP, IND3_PIN, GPIO_Mode_Out_PP);
    IND1_OFF;
    IND2_OFF;
    IND2_OFF;
    
    gpio_cfg((uint32_t)MUTE_PORT_GROUP, MUTE_PIN, GPIO_Mode_Out_PP);
    MUTE_ON;
	
	//init 4 relays
	gpio_cfg((uint32_t)IO5_GROUP, IO5_PIN, GPIO_Mode_Out_PP);
	gpio_cfg((uint32_t)IO6_GROUP, IO6_PIN, GPIO_Mode_Out_PP);
	gpio_cfg((uint32_t)IO7_GROUP, IO7_PIN, GPIO_Mode_Out_PP);
	gpio_cfg((uint32_t)IO8_GROUP, IO8_PIN, GPIO_Mode_Out_PP);

	//init the relay on board

	

}

void update_relay_status(int relay, int value)
{
    switch(relay)
    {

		case 6:
			if(value > 0)
				GPIO_SET(IO6_GROUP, IO6_PIN);
			else
				GPIO_CLR(IO6_GROUP, IO6_PIN);
			break;
		case 7:
			if(value > 0)
				GPIO_SET(IO7_GROUP, IO7_PIN);
			else
				GPIO_CLR(IO7_GROUP, IO7_PIN);
			break;
		case 8:
			if(value > 0)
				GPIO_SET(IO8_GROUP, IO8_PIN);
			else
				GPIO_CLR(IO8_GROUP, IO8_PIN);
			break;
		default:
			p_dbg("[ERR] unknow relay number[%d]", relay);
			break;
	}

}

//get button status
int get_alarm_value()
{
    int stat = GPIO_STAT(GPIOD, GPIO_Pin_7); //读取按键状态(0:1)
    //p_dbg("current BUTTON VALUE: [%d]", flg); //打印按键状态
    return stat;
}

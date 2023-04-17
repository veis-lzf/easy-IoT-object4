#define DEBUG

#include "drivers.h"
#include "app.h"
#include "api.h"

#include <absacc.h>

extern char Image$$RW_IRAM1$$ZI$$Limit[];
void _mem_init(void)
{
	uint32_t malloc_start, malloc_size;

	malloc_start = (uint32_t) &Image$$RW_IRAM1$$ZI$$Limit; //ȡ�����ڴ���͵�ַ
	malloc_start = (malloc_start + 3) & (~0x03);// ALIGN TO word

	// 0x20000Ϊ�����ڴ��ܴ�С������MCUʱ��ע��
	// malloc_size = 0x20000000 + (128*1024) - malloc_start;
	// @veis20230416, stm32f427vit6 sram size=256(112+16+64+64(ccm))KB
	malloc_size = 0x20000000 + (192*1024) - malloc_start;

/*
*407��64k�ĸ���ram(��ַ0x10000000)�������䲻������DMA������ʹ�ã����Բ���������ڴ��
*�����ֶ�����ʹ��
*/
	//sys_meminit((void*)0x10000000, 0x10000, (void*)malloc_start, malloc_size);
	sys_meminit((void*)malloc_start, malloc_size);
	sys_meminit2((void*)0x10000000, 0x10000);
}

void init_systick()
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	SysTick_Config(SystemCoreClock / (OS_TICK_RATE_HZ *10));
}

void init_rng()
{
	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, ENABLE);
	RNG_Cmd(ENABLE);
}

/**
 *��һ�������
 */
uint32_t get_random(void)
{
	return RNG_GetRandomNumber();
}

void gpio_cfg(uint32_t group, uint32_t pin, uint8_t mode)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = pin;

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	if (mode == GPIO_Mode_AIN)
	{
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	}
	else if (mode == GPIO_Mode_IN_FLOATING)
	{
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	}
	else
	if (mode == GPIO_Mode_IPD)
	{
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	}
	else
	if (mode == GPIO_Mode_IPU)
	{
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	}
	else
	if (mode == GPIO_Mode_Out_OD)
	{
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	}
	else
	if (mode == GPIO_Mode_Out_PP)
	{
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	}
	else
	if (mode == GPIO_Mode_AF_OD)
	{
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	}
	else
	if (mode == GPIO_Mode_AF_PP)
	{
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	}
	else
	if (mode == GPIO_Mode_AF_IF)
	{
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	}
	else
	if (mode == GPIO_Mode_AF_IPU)
	{
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	}
	GPIO_Init((GPIO_TypeDef*)group, &GPIO_InitStructure);
}


void driver_gpio_init()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE); //�ⲿ�ж���Ҫ�õ�
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | 
		RCC_AHB1Periph_GPIOB | 
		RCC_AHB1Periph_GPIOC | 
		RCC_AHB1Periph_GPIOD | 
		RCC_AHB1Periph_GPIOE, ENABLE);
    
	gpio_cfg((uint32_t)RELAY_GROUP, RELAY_PIN, GPIO_Mode_Out_PP);
	RELAY_OFF;

}

int check_rst_stat()
{
	uint32_t stat;
	stat = RCC->CSR;
	RCC->CSR = 1L << 24; //�����λ��־

	p_err("reset:");
	if (stat &(1UL << 31))
	// �͹��ĸ�λ
	{
		p_err("low power\n");
	}
	if (stat &(1UL << 30))
	//���ڿ��Ź���λ
	{
		p_err("windw wdg\n");
	}
	if (stat &(1UL << 29))
	//�������Ź���λ
	{
		p_err("indep wdg\n");
	}
	if (stat &(1UL << 28))
	//�����λ
	{
		p_err("soft reset\n");
	}
	if (stat &(1UL << 27))
	//���縴λ
	{
		p_err("por reset\n");
	}
	if (stat &(1UL << 26))
	//rst��λ
	{
		p_err("user reset\n");
	}

	return 0;
}



/*
* TIMER5��Ϊus��ʱ�������ڲ�������ʱ�䣬
* �ں�����պ���ʪ�ȴ�������Ҫ�õ�
*
*/
void init_us_timer()
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);

	TIM_DeInit(TIM5); 

	//APB1ʱ��42M��tim5ʱ��84M������Ԥ��Ƶ��Ϊ840�� ÿMS����100����ÿ��������10us
	TIM_TimeBaseStructure.TIM_Period = 0xffffffff;
 	TIM_TimeBaseStructure.TIM_Prescaler = 840;
  	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);

	TIM_ARRPreloadConfig(TIM5, ENABLE);
	TIM_Cmd(TIM5, ENABLE);
}

uint32_t get_us_count()
{
	return TIM_GetCounter(TIM5)*10;
}

void driver_misc_init()
{
	driver_gpio_init();
	init_systick();
	init_rng();
	init_us_timer(); 
}


#define DEBUG
#include "drivers.h"
#include "bsp.h"

#include "debug.h"

//使用TIM4的PWM输出控制灯的亮度，可以在D13脚上接一个USB小台灯，观察台灯亮度的变化（台灯正级接D13，负级接地）

uint32_t Period_tot = 8000;

void lamp_pwm_init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	RCC_ClocksTypeDef RCC_ClocksStatus;
	TIM_BDTRInitTypeDef TIM_BDTRInitStructure;
	uint32_t apbclock, scaler = 0, Counter;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	
	TIM_DeInit(TIM4); 
	RCC_GetClocksFreq(&RCC_ClocksStatus);
	apbclock = RCC_ClocksStatus.PCLK2_Frequency;
	Counter = apbclock*2/(scaler + 1)/38000;
	

	GPIO_PinAFConfig(IO4_GROUP, GPIO_PinSource13, GPIO_AF_TIM4); 
	gpio_cfg((uint32_t)IO4_GROUP, IO4_PIN, GPIO_Mode_AF_PP);
	
	TIM_TimeBaseStructure.TIM_Period = Period_tot;          //一个方波周期为Period_tot个时钟
 	//TIM_TimeBaseStructure.TIM_Prescaler = 84000-1;
 	TIM_TimeBaseStructure.TIM_Prescaler = 84 -1;           //时钟预分频，该值越小，则输出方波周期越小
  	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

  	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = Period_tot/2;          //设置输出的方波占空比为1/2，代表台灯初始亮度为50
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	
	TIM_OCInitStructure.TIM_OCNPolarity=TIM_OCNPolarity_Low;        //设置互补端输出极性
    TIM_OCInitStructure.TIM_OutputNState=TIM_OutputNState_Enable;//使能互补端输出
    TIM_OCInitStructure.TIM_OCIdleState=TIM_OCIdleState_Reset;        //死区后输出状态
    TIM_OCInitStructure.TIM_OCNIdleState=TIM_OCNIdleState_Reset;//死区后互补端输出状态
    TIM_OC2Init(TIM4, &TIM_OCInitStructure);
	
	TIM_BDTRInitStructure.TIM_OSSRState = TIM_OSSRState_Disable;//运行模式下输出选择 
    TIM_BDTRInitStructure.TIM_OSSIState = TIM_OSSIState_Disable;//空闲模式下输出选择 
    TIM_BDTRInitStructure.TIM_LOCKLevel = TIM_LOCKLevel_OFF;         //锁定设置
    TIM_BDTRInitStructure.TIM_DeadTime = 0x90;                                         //死区时间设置
    TIM_BDTRInitStructure.TIM_Break = TIM_Break_Disable;                 //刹车功能使能
    TIM_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_Low;//刹车输入极性
    TIM_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Enable;//自动输出使能 
    //TIM_BDTRConfig(TIM4,&TIM_BDTRInitStructure);

    TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);
	
	TIM_ARRPreloadConfig(TIM4, ENABLE);

	TIM_Cmd(TIM4, ENABLE);
	TIM_CtrlPWMOutputs(TIM4, ENABLE);                         //开始PWM输出

}

//改变PWM占空比，实现台灯亮度调整，调整范围0~100.
void lamp_pwm_set(int stat)
{
    if(stat > 100 || stat < 0) return;
	if(stat == 0) stat = 1;
	if(stat == 100) stat = 99;
	
	TIM_SetCompare2(TIM4, Period_tot*stat/100);
}



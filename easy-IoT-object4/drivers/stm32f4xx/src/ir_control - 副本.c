#define DEBUG
#include "drivers.h"
#include "bsp.h"

#include "debug.h"

#define TEST_TIMER5

#define MAX_SEQ_NUM 256
uint32_t send_seq[MAX_SEQ_NUM], seq_pos = 0, send_pos = 0;
bool ir_send_pending = FALSE, ir_send_val = FALSE;

uint8_t ir_recv_data[16];

bool ir_dbg = 0;

static void ir_timer_callback(void);

void ir_pwm_enable()
{
	//TIM_Cmd(TIM1, ENABLE);
	TIM_SelectOCxM(TIM1, TIM_Channel_2, TIM_OCMode_PWM1);
}

void ir_pwm_disable()
{
	//TIM_Cmd(TIM1, DISABLE);
	//TIM_ForcedOC3Config(TIM1, TIM_ForcedAction_InActive);
	TIM_SelectOCxM(TIM1, TIM_Channel_2, TIM_OCMode_Active);
}

void ir_dbg_switch()
{
	ir_dbg = !ir_dbg;
	if(ir_dbg)
		p_dbg("开启红外调试打印");
	else
		p_dbg("关闭红外调试打印");
}

uint32_t Period_tot = 8000;

void init_io4_pwm()
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	RCC_ClocksTypeDef RCC_ClocksStatus;
	TIM_BDTRInitTypeDef TIM_BDTRInitStructure;
	uint32_t apbclock, scaler = 0, Counter;
	
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	
	TIM_DeInit(TIM4); 
	RCC_GetClocksFreq(&RCC_ClocksStatus);
	apbclock = RCC_ClocksStatus.PCLK2_Frequency;
	Counter = apbclock*2/(scaler + 1)/38000;
	

	GPIO_PinAFConfig(IO4_GROUP, GPIO_PinSource13, GPIO_AF_TIM4); 
	gpio_cfg((uint32_t)IO4_GROUP, IO4_PIN, GPIO_Mode_AF_PP);
	
	TIM_TimeBaseStructure.TIM_Period = Period_tot;
 	//TIM_TimeBaseStructure.TIM_Prescaler = 84000-1;
 	TIM_TimeBaseStructure.TIM_Prescaler = 84 -1;           //the only difference with IR, this make IO4 PWM at high freq
  	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

  	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = Period_tot/2;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	
	TIM_OCInitStructure.TIM_OCNPolarity=TIM_OCNPolarity_Low;        //设置互补端输出极性
        TIM_OCInitStructure.TIM_OutputNState=TIM_OutputNState_Enable;//使能互补端输出
        TIM_OCInitStructure.TIM_OCIdleState=TIM_OCIdleState_Reset;        //死区后输出状态
        TIM_OCInitStructure.TIM_OCNIdleState=TIM_OCNIdleState_Reset;//死区后互补端输出状态
	//TIM_OC3Init(TIM1, &TIM_OCInitStructure);
    TIM_OC2Init(TIM4, &TIM_OCInitStructure);
	 TIM_BDTRInitStructure.TIM_OSSRState = TIM_OSSRState_Disable;//运行模式下输出选择 
        TIM_BDTRInitStructure.TIM_OSSIState = TIM_OSSIState_Disable;//空闲模式下输出选择 
        TIM_BDTRInitStructure.TIM_LOCKLevel = TIM_LOCKLevel_OFF;         //锁定设置
        TIM_BDTRInitStructure.TIM_DeadTime = 0x90;                                         //死区时间设置
        TIM_BDTRInitStructure.TIM_Break = TIM_Break_Disable;                 //刹车功能使能
        TIM_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_Low;//刹车输入极性
        TIM_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Enable;//自动输出使能 
        //TIM_BDTRConfig(TIM4,&TIM_BDTRInitStructure);

	//TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);
	
	TIM_ARRPreloadConfig(TIM4, ENABLE);

	TIM_Cmd(TIM4, ENABLE);
	TIM_CtrlPWMOutputs(TIM4, ENABLE);       

	//ir_pwm_disable();
	//ir_pwm_enable();
	//TIM_SelectOCxM(TIM4, TIM_Channel_2, TIM_OCMode_PWM1);
	//start_timer(5000000, ir_timer_callback);
	//pause_timer();
	
	//ir_recv_enable();
}

void init_ir()
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	RCC_ClocksTypeDef RCC_ClocksStatus;
	TIM_BDTRInitTypeDef TIM_BDTRInitStructure;
	uint32_t apbclock, scaler = 0, Counter;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	
	TIM_DeInit(TIM1); 
	RCC_GetClocksFreq(&RCC_ClocksStatus);
	apbclock = RCC_ClocksStatus.PCLK2_Frequency;
	Counter = apbclock*2/(scaler + 1)/38000;

	GPIO_PinAFConfig(GPIOB, GPIO_PinSource0, GPIO_AF_TIM1); 

	gpio_cfg((uint32_t)GPIOB, GPIO_Pin_0, GPIO_Mode_AF_PP);
	
	TIM_TimeBaseStructure.TIM_Period = Period_tot;
 	TIM_TimeBaseStructure.TIM_Prescaler = 84000-1;
  	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

  	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = Period_tot/2;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	
	TIM_OCInitStructure.TIM_OCNPolarity=TIM_OCNPolarity_Low;        //设置互补端输出极性
	TIM_OCInitStructure.TIM_OutputNState=TIM_OutputNState_Enable;//使能互补端输出
	TIM_OCInitStructure.TIM_OCIdleState=TIM_OCIdleState_Reset;        //死区后输出状态
	TIM_OCInitStructure.TIM_OCNIdleState=TIM_OCNIdleState_Reset;//死区后互补端输出状态
	//TIM_OC3Init(TIM1, &TIM_OCInitStructure);
    TIM_OC2Init(TIM1, &TIM_OCInitStructure);
	TIM_BDTRInitStructure.TIM_OSSRState = TIM_OSSRState_Disable;//运行模式下输出选择 
	TIM_BDTRInitStructure.TIM_OSSIState = TIM_OSSIState_Disable;//空闲模式下输出选择 
	TIM_BDTRInitStructure.TIM_LOCKLevel = TIM_LOCKLevel_OFF;         //锁定设置
	TIM_BDTRInitStructure.TIM_DeadTime = 0x90;                                         //死区时间设置
	TIM_BDTRInitStructure.TIM_Break = TIM_Break_Disable;                 //刹车功能使能
	TIM_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_Low;//刹车输入极性
	TIM_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Enable;//自动输出使能 
	//TIM_BDTRConfig(TIM1,&TIM_BDTRInitStructure);

	//TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);
	
	TIM_ARRPreloadConfig(TIM1, ENABLE);

	TIM_Cmd(TIM1, ENABLE);
	TIM_CtrlPWMOutputs(TIM1, ENABLE);       

	//ir_pwm_disable();
	//ir_pwm_enable();
	//start_timer(5000000, ir_timer_callback);
	//pause_timer();
	
	//ir_recv_enable();
}

void ir_trig_send(void)
{
	if(send_pos < seq_pos){
		if(ir_send_val)
			ir_pwm_enable();
		else
			ir_pwm_disable();
		ir_send_val = !ir_send_val;
		//p_dbg("[%d]", send_seq[send_pos]);
		mode_timer_by_counter(send_seq[send_pos++]);
	}else{
		ir_send_pending = FALSE;
		ir_pwm_disable();
		pause_timer();
		p_dbg("ir send end");
	}
}

void change_pwm_stat(int stat)
{
    if(stat > 100 || stat < 0) return;
	if(stat == 0) stat = 1;
	if(stat == 100) stat = 99;
	
	TIM_SetCompare2(TIM1, Period_tot*stat/100);
	TIM_SetCompare2(TIM4, Period_tot*stat/100);
}

void ir_timer_callback()
{
/*
	pause_timer();
	ir_trig_send();
*/
    static int i = 20;
    change_pwm_stat(i);
	p_dbg("ir_timer_callback change [%d]", i);
	
	if(i == 20) i = 80;
	else i = 20;
	
#if 0  //for test
	if(ir_send_val)
		ir_pwm_enable();
	else
		ir_pwm_disable();
	ir_send_val = !ir_send_val;
#endif
}



//value us
void ir_fill_seq(uint32_t value)
{
	uint32_t timer_clk;
	if(seq_pos < MAX_SEQ_NUM){
		timer_clk = get_hard_timer_clk();
		send_seq[seq_pos++] = timer_clk*2/1000000*value;
	}else
		p_err("fill seq over");
}

void rst_seq_pos()
{
	seq_pos = 0;
}

void ir_fill_nec_preamble()
{
	rst_seq_pos();
	ir_fill_seq(9000);
	ir_fill_seq(4500);
}


//结束发送，需要一个下降沿
void ir_fill_nec_tail()
{
	ir_fill_seq(500);
}

void ir_fill_nec_data0()
{
	ir_fill_seq(560);
	ir_fill_seq(565);
}

void ir_fill_nec_data1()
{
	ir_fill_seq(560);
	ir_fill_seq(1685);
}


//延时码，for kelon空调
void ir_fill_nec_delay()
{
	ir_fill_seq(560);
	ir_fill_seq(8400);
}


void ir_fill_nec_byte(uint8_t value)
{
	int i = 8;
	while(i--)
	{
		if(value & 1)
			ir_fill_nec_data1();
		else
			ir_fill_nec_data0();

		value = value >> 1;
	}
}

void ir_start_send()
{
	p_dbg("ir send start");
	send_pos = 0;
	ir_send_pending = TRUE;
	ir_send_val = 1;
	ir_trig_send();
}

/*
*发送一段nec编码的红外数据
*就是一个前导后加n个字节数据
*
*/
void ir_send_nec_data(uint8_t *data, int len)
{
	int i;
	if(ir_send_pending)
		return;
	
	ir_fill_nec_preamble();
	/*
	ir_fill_nec_byte(usr_code);
	ir_fill_nec_byte(~usr_code);
	ir_fill_nec_byte(data);
	ir_fill_nec_byte(~data);*/
	for(i = 0; i < len; i++)
		ir_fill_nec_byte(data[i]);
	
	ir_fill_nec_tail();
	ir_start_send();
}

/*
*发送kelong空调控制信号
*参数具体定义请看<科龙中央空调红外编码协议表>
*
*/
void ir_send_kelon_cmd(bool wind_direct, uint8_t arefaction, 
				bool sleep, bool open_off, 
				uint8_t  wind_power, uint8_t temp, 
				uint8_t timer, uint8_t mode, 
				uint8_t time)
{
	uint8_t tmp, sum = 0;
	if(ir_send_pending)
		return;
	
	ir_fill_nec_preamble();	//发送前导
	
	ir_fill_nec_byte(0x83);	//BYTE1
	ir_fill_nec_byte(0x06);	//BYTE2

	tmp = 0;
	tmp |= (wind_direct & 0x01) << 7;
	tmp |= (arefaction & 0x07) << 4;
	tmp |= (sleep & 0x01) << 3;
	tmp |= (open_off & 0x01) << 2;
	tmp |= (wind_power & 0x03);
	ir_fill_nec_byte(tmp);	//BYTE3
	sum += tmp;
		
	tmp = 0;
	tmp |= ((temp - 18)& 0x0f) << 4;	//温度范围是18-32
	tmp |= (timer & 0x01) << 3;
	tmp |= (mode & 0x07);
	ir_fill_nec_byte(tmp);	//BYTE4
	sum += tmp;

	tmp = time;
	ir_fill_nec_byte(tmp);	//BYTE5
	sum += tmp;
	
	ir_fill_nec_byte(0);	//BYTE6
	ir_fill_nec_delay();	//延时

	ir_fill_nec_byte(0);	//BYTE7
	ir_fill_nec_byte(0);	//BYTE8
	ir_fill_nec_byte(0);	//BYTE9
	ir_fill_nec_byte(0);	//BYTE10
	ir_fill_nec_byte(0);	//BYTE11
	ir_fill_nec_byte(0);	//BYTE12
	ir_fill_nec_byte(0);	//BYTE13

	//校验和的计算方式，目前推测是(BYTE2+BYTE3+BYTE4)
	ir_fill_nec_byte(sum);	//BYTE14 校验和
	
	ir_fill_nec_tail();		//最后一个下降沿
	ir_start_send();	//开始发送
}


/*
*红外接收功能初始化，这里用TIM5作为计数
*
*/
void ir_recv_enable()
{
	EXTI_InitTypeDef   EXTI_InitStructure;
  	NVIC_InitTypeDef   NVIC_InitStructure;

	gpio_cfg((uint32_t)IR_IN_PORT_GROUP, IR_IN_PIN, GPIO_Mode_IPU);	

	
	SYSCFG_EXTILineConfig(IR_IN_SOURCE_GROUP, IR_IN_PIN_SOURCE);
	EXTI_InitStructure.EXTI_Line = IR_IN_LINE;
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  	//红外接收探头默认是高电平
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init(&EXTI_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = EXTI15_10_IRQn_Priority;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	//NVIC_EnableIRQ(EXTI9_5_IRQn);
}

/*
*关闭红外接收中断
*
*/
void ir_recv_disable()
{
	NVIC_DisableIRQ(EXTI15_10_IRQn);
}

/*
*红外接收中断处理函数
*
*/
void handle_ir_recv(uint32_t time)
{
	static int cnt = 0, cnt_ex = 0;
	static uint8_t data, bit;

	if(ir_dbg)
	printf("%d ", time/100);
	switch(cnt)
	{
		case 0:
		//if(time < 14000 && time > 13000)		// 13.5ms,
		if(time < 14000 && time > 5000)
		{
			cnt = 1;
			cnt_ex = 0;
			data = 0;
		}else
			;		// 第一个下降沿肯定是个非法值，忽略掉就行
		break;
		default:
		{
			if(time > 900 && time < 1300)		// 1.125ms
				bit = 0;
			else if(time > 2000 && time < 2500)	// 2.25ms
				bit = 0x80;
			else if(time > 8000 && time < 9000)	// 8.4ms	//科龙空调中间出现的一个延时码
			{
				cnt_ex = 0; 
				printf("^");
				break;
			}
			else{
				cnt = 0;		// 第一个下降沿也可能出现在这里
				break;
			}
			if(cnt_ex++ < 8)
			{
				data >>= 1;
				data |=  bit;
			}

			if(cnt_ex == 8)
			{
				printf("%x ", data);
				ir_recv_data[cnt - 1] = data;
				cnt++;
				cnt_ex = 0;
				data = 0;

				// 接收的数据不能超过ir_recv_data的大小
				if(cnt >= 17)
				{
					p_dbg("ir recv over");
					cnt = 0;
					break;
				}
			}
		}
		break;
	}
	
}

/*
*红外接收中断入口
*
*/
void __EXTI14_IRQHandler(void)
{

	static uint32_t last_time = 0;
	uint32_t time;
	time = get_us_count();

	//转换成us
	handle_ir_recv(time - last_time);
	last_time = time;

	
}



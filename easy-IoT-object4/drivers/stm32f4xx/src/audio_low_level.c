#define DEBUG
//#define DEBUG_TASK

#include "audio_low_level.h"
#include "drivers.h"
#include "app.h"
#include "api.h"

#include "audio/adpcm.h"
#include "audio/audio.h"
#include "debug.h"

#if SUPPORT_AUDIO

#define ADC_CAPTURE_MODE_DMA	0
#define ADC_CAPTURE_MODE_INT	1

#define ADC1_CAPTURE_MODE	ADC_CAPTURE_MODE_INT

struct AUDIO_DEC_INPUT 	dec_input;
struct AUDIO_DEC_OUTPUT dec_output;
struct AUDIO_ADC		adc;

#define ADC_RECV_STK_SIZE 	128

wait_event_t adc_rx_wait = 0;
bool	audio_record_enable = FALSE;

wait_event_t ai_audio_wait = 0;

static void adc_recv_thread(void *arg);
//#define TEST_DAC

void update_ai_event(void)
{
    if(audio_cfg.recording_to_flash == FALSE)
    {        
        p_dbg("[update_ai_event: audio OK]\r\n");
        wake_up(ai_audio_wait);                       //释放信号量ai_audio_wait，唤醒任务
    }
}

int get_ai_audio_pcm_data_len(void)
{
	int ret = wait_event_timeout(ai_audio_wait, 5000);      //等待信号量ai_audio_wait，
	if(ret == WAIT_EVENT_TIMEOUT) {
	    return 0;
	}
    if(audio_cfg.recording_to_flash) {
        p_dbg("recording is ongoing\r\n");
        return -1;
    }

    return (audio_cfg.recording_to_flash_addr - AUDIO_RECORD_BASE_ADDR);  //返回音频保存的地址
}

int get_ai_audio_pcm_data(int offset, int len, char* data)                   //获得pcm数据流
{
    int ret = 0;
    if((offset+len) >= (audio_cfg.recording_to_flash_addr - AUDIO_RECORD_BASE_ADDR))     //如果要发送的长度大于数据保存的长度
    {
        return -1;
    }
    ret = m25p80_read(AUDIO_RECORD_BASE_ADDR + offset, len, data);     //读出保存的进过压缩后的音频数据
    
    return ret;
}


//send recording event to queue
int send_voice_recording_event(void)       //发送音频
{
    static int start_recording = 0;
    int ret = msgsnd(audio_cfg.ai_queue, &start_recording);     //向队列中发送消息      audio_cfg.ai_queue 
     //faild
	if(ret) {
        p_err("send_voice_recording_event err:%d", ret);
    }

    return 0;
}
//encode and send utf-8 data to queue
int send_and_encode_text_event(char * data, int len)  //对文字进行   utf-8编码
{
    int i = 0, flag = 0, ret = 0;
    u8_t ch;
    u8_t *uart_url_data = NULL;
    char hex[] = "0123456789ABCDEF"; 
    p_dbg("send_and_encode_text_event data len: [%d]", len);
    
    uart_url_data = mem_calloc(3, len+1);
    if(uart_url_data == NULL)   
		return -1;
    //switch utf-8 to urlencode here
    
    for(i =0; i < len; i++) {
        ch = data[i];
        if((ch>= '0' && ch <= '9') ||
            (ch>= 'a' && ch <= 'z') ||
            (ch>= 'A' && ch <= 'Z') ||
            ch == '-' || ch == '_'|| ch == '.'|| ch == '~') {
            uart_url_data[flag++] = ch;
        }
        else if (ch == ' ') {
            uart_url_data[flag++] = '+';
        }
        else {
            uart_url_data[flag++] += '%';  
            uart_url_data[flag++] += hex[ch/16];  
            uart_url_data[flag++] += hex[ch%16];
        }
    }
    
    uart_url_data[flag++] = 0;
    p_dbg("encode_and_send_text_data urlencode:%s", uart_url_data);
    
    ret = msgsnd(audio_cfg.ai_queue, uart_url_data);            //向队列中发送消息   audio_cfg.ai_queue
    if(ret) {
        p_err("encode_and_send_text_data err:%d", ret);
    }

    return ret;
}

int get_voice_ai_event(char ** data)
{
	int ret  = msgrcv(audio_cfg.ai_queue, (void *)data, 0);            // 接收消息队列       audio_cfg.ai_queue
    if(ret != 0) {
        p_err_fun;
        return -1;
    }
    return 0;
}

static void DMA_DAC1Configuration(uint32_t *BufferSRC, uint32_t BufferSize);

int dac_low_level_open(void)                                                   //dac的初始化  和定时器TIM4的初始化
{   
	p_dbg_enter;
	DAC_InitTypeDef DAC_InitStruct;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
#ifdef TEST_DAC
	NVIC_InitTypeDef NVIC_InitStructure;
#endif
	RCC_ClocksTypeDef RCC_ClocksStatus;
	uint32_t apbclock, Counter;
	DAC_DeInit();

	memset(&dec_input, 0, sizeof(struct AUDIO_DEC_INPUT));
	memset(&dec_output, 0, sizeof(struct AUDIO_DEC_OUTPUT));

	 //没有分配空间
	if(!dec_output.data) {
		dec_output.data = (u8_t*)mem_calloc(AUDIO_DMA_BUFF_SIZE, 1);    //分配空间   DMA BUFF缓冲最终要播放的数据(即输入到DAC的数据)
		if(!dec_output.data) {
			goto err;
		}
	}

	if(!dec_input.data) {
		dec_input.data = (u8_t*)mem_calloc(AUDIO_DAC_INPUT_BUFF_SIZE, 1);     //  分配空间   DMA BUFF缓冲最终要播放的数据(即输入到DAC的数据)
		if(!dec_input.data) {
			goto err;
		}
	}
	
	dec_output.len = AUDIO_DMA_BUFF_SIZE;
	dec_output.play_pos = dec_output.write_pos = 0;
	dec_output.dac_tx_waitq = init_event();             //建立一个信号量     dec_output.dac_tx_waitq
	
	dec_input.buff_len = AUDIO_DAC_INPUT_BUFF_SIZE;
	dec_input.data_len = 0;
	dec_input.old_type = dec_input.type = AUDIO_TYPE_UNKOWN;
	
	RCC_GetClocksFreq(&RCC_ClocksStatus);
	apbclock = RCC_ClocksStatus.PCLK1_Frequency;
	Counter = apbclock/DEFAULT_PLAY_SAMPLERATE;

	p_dbg("set samplerate:%d, conter:%d, apbclock:%d\n", DEFAULT_PLAY_SAMPLERATE, Counter, apbclock);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC | RCC_APB1Periph_TIM4, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);


	gpio_cfg((uint32_t)DAC2_PORT, DAC2_PIN, GPIO_Mode_AIN);   //DAC2

	DAC_InitStruct.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bit0;
	DAC_InitStruct.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
	DAC_InitStruct.DAC_Trigger = DAC_Trigger_T4_TRGO;    // 定时器4触发
	DAC_InitStruct.DAC_WaveGeneration = DAC_WaveGeneration_Triangle;
	DAC_Init(DAC_Channel_2, &DAC_InitStruct);
	
	DMA_DAC1Configuration((uint32_t *)dec_output.data, AUDIO_DMA_BUFF_SIZE);
	DAC_ITConfig(DAC_Channel_2, DAC_IT_DMAUDR, ENABLE);

//timer
	TIM_TimeBaseStructure.TIM_Period = 1250;                 //67200
	TIM_TimeBaseStructure.TIM_Prescaler = 0;                 //预分频为0，定时器时钟频倍频，为84M
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	TIM_SelectOutputTrigger(TIM4, TIM_TRGOSource_Update);
	TIM_ARRPreloadConfig(TIM4, ENABLE);

	#ifdef TEST_DAC
	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
	TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = TIM4_IRQn_Priority;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	NVIC_EnableIRQ(TIM4_IRQn);
	#endif
	
	DAC_Cmd(DAC_Channel_2, ENABLE);
	
	DAC_DMACmd(DAC_Channel_2, ENABLE);

	TIM_Cmd(TIM4, ENABLE);
	return 0;

err:                                          //如果分配空间不成功，会goto到err
	if(dec_input.data)
		mem_free(dec_input.data);
	
	if(dec_output.data)
		mem_free(dec_output.data);

	dec_input.data = 0;
	dec_output.data = 0;
	return -1;
}

int dac_open(void)                                                   //dac的初始化  和定时器TIM4的初始化
{   
	p_dbg_enter;
	DAC_Cmd(DAC_Channel_2, ENABLE);
	DAC_DMACmd(DAC_Channel_2, ENABLE);
	return 0;
}

int dac_switch_samplerate(int val)
{
	RCC_ClocksTypeDef RCC_ClocksStatus;

	uint32_t apbclock, scaler = 0, Counter;
	
	p_dbg("dac change samplerate:%d\n", val);
	if(val == 0)
		return -1;
	RCC_GetClocksFreq(&RCC_ClocksStatus);
	apbclock = RCC_ClocksStatus.PCLK1_Frequency;
	
	Counter = apbclock*2/(scaler + 1)/val;
	TIM_SetAutoreload(TIM4, Counter);
	return 0;
}

void dac_low_level_close(void)
{
//dac 恢复为IO口
	gpio_cfg((uint32_t)DAC2_PORT, DAC2_PIN, GPIO_Mode_Out_PP);//

	DAC_DMACmd(DAC_Channel_2, DISABLE);
	DAC_ITConfig(DAC_Channel_2, DAC_IT_DMAUDR, DISABLE);
	DAC_Cmd(DAC_Channel_2, DISABLE);                   //DAC_Channel_2失能
	
//timer

	TIM_ITConfig(TIM4, TIM_IT_Update, DISABLE);
	TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
	TIM_Cmd(TIM4, DISABLE);
#ifdef TEST_DAC
	NVIC_DisableIRQ(TIM4_IRQn);
#endif
	NVIC_DisableIRQ(DMA1_Stream6_IRQn);

	DMA_Cmd(DMA1_Stream6, DISABLE);                          //DMA1_Stream6失能

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC | RCC_APB1Periph_TIM4, DISABLE);             //时钟失能

	if(dec_input.data)
		mem_free(dec_input.data);
	
	if(dec_output.data)
		mem_free(dec_output.data);

	dec_input.data = 0;
	dec_output.data = 0;

}

void dac_close(void)
{
	//dac 恢复为IO口
	DAC_DMACmd(DAC_Channel_2, DISABLE);
	DAC_Cmd(DAC_Channel_2, DISABLE);                   //DAC_Channel_2失能
}

void dac_close_channel(int channel)
{
	gpio_cfg((uint32_t)DAC2_PORT, DAC2_PIN, GPIO_Mode_IN_FLOATING);//DAC2
}

			  
static void DMA_DAC1Configuration(uint32_t *BufferSRC, uint32_t BufferSize)     //DAC使用的是DMA1_Stream6
{
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	DMA_ITConfig(DMA1_Stream6, DMA_IT_TC | DMA_IT_TE | DMA_IT_HT,DISABLE);
	DMA_ClearFlag(DMA1_Stream6, DMA_FLAG_TCIF6 | DMA_FLAG_TEIF6 | DMA_FLAG_HTIF6 | DMA_FLAG_DMEIF6 | DMA_FLAG_HTIF6);
	
	DMA_Cmd(DMA1_Stream6, DISABLE);

	DMA_StructInit(&DMA_InitStructure);
	DMA_InitStructure.DMA_Channel = DMA_Channel_7;
	DMA_InitStructure.DMA_PeripheralBaseAddr =(uint32_t)&(DAC->DHR12R2);
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)BufferSRC;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_BufferSize = BufferSize/2;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
 	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
	DMA_Init(DMA1_Stream6, &DMA_InitStructure);

	////使能出错中断,半完成和完成中断
	DMA_ITConfig(DMA1_Stream6, DMA_IT_TC|DMA_IT_TE|DMA_IT_HT, ENABLE);  //使能传输完成和出错中断

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream6_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = DMA1_Stream6_IRQn_Priority;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	DMA_Cmd(DMA1_Stream6, ENABLE);
}

DECLARE_MONITOR_ITEM("DMA1_Stream6_IRQHandler_cnt", DMA1_Stream6_IRQHandler_cnt) ;

void DMA1_Stream6_IRQHandler()        //DAC的DMA中断函数
{	
	enter_interrupt();

	INC_MONITOR_ITEM_VALUE(DMA1_Stream6_IRQHandler_cnt);       //DMA1_Stream6_IRQHandler_cnt++    进入DMA中断的次数

	if(DMA_GetITStatus(DMA1_Stream6, DMA_IT_TCIF6))
	{
		DMA_ClearITPendingBit(DMA1_Stream6, DMA_IT_TCIF6);
		//if(audio_cfg.pcm->sync)
		{
			dec_output.play_circle_cnt++;
			dec_output.int_flag = 1;
			dec_output.write_pos = 0;
			if(dec_output.data)
			{
				int i;
				uint16_t last_value = *((uint16_t*)(dec_output.data + AUDIO_DMA_BUFF_SIZE/2 - 2));
				for(i = 0; i < AUDIO_DMA_BUFF_SIZE/4; i++)
					*((uint16_t*)(dec_output.data + AUDIO_DMA_BUFF_SIZE/2 + i*2)) = last_value;
				//memset(dec_output.data + AUDIO_DMA_BUFF_SIZE/2, 0, AUDIO_DMA_BUFF_SIZE/2);
			}
			if(dec_output.need_wait)
				goto end_and_sched;
		}
		
	}

	if(DMA_GetITStatus(DMA1_Stream6, DMA_IT_HTIF6))
	{
		DMA_ClearITPendingBit(DMA1_Stream6, DMA_IT_HTIF6);
		//if(audio_cfg.pcm->sync)
		{
			dec_output.play_circle_cnt++;
			dec_output.int_flag = 0;
			dec_output.write_pos = 0;
			if(dec_output.data){
				//memset(dec_output.data, 0, AUDIO_DMA_BUFF_SIZE/2);
				int i;
				uint16_t last_value = *((uint16_t*)(dec_output.data + AUDIO_DMA_BUFF_SIZE - 2));
				for(i = 0; i < AUDIO_DMA_BUFF_SIZE/4; i++)
					*((uint16_t*)(dec_output.data + i*2)) = last_value;

			}
			
			if(dec_output.need_wait)
				goto end_and_sched;
		}
	}

	if(DMA_GetITStatus(DMA1_Stream6, DMA_IT_TEIF6))
	{
		DMA_ClearFlag(DMA1_Stream6,  DMA_IT_TEIF6);
		p_err("DMA_IT_TEIF6");
	}
	
	exit_interrupt(0);
	return;
	
end_and_sched:
	wake_up(dec_output.dac_tx_waitq);           //释放信号量dec_output.dac_tx_waitq    唤醒事件
	exit_interrupt(1);
}


#if  ADC1_CAPTURE_MODE == ADC_CAPTURE_MODE_DMA
static void DMA_ADC1Configuration(uint32_t *BufferDST, uint32_t BufferSize)
{
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	DMA_ITConfig(DMA2_Stream4, DMA_IT_TC | DMA_IT_TE | DMA_IT_HT,DISABLE);
	DMA_ClearFlag(DMA2_Stream4, DMA_FLAG_TCIF4 | DMA_FLAG_TEIF4 | DMA_FLAG_HTIF4 | DMA_FLAG_DMEIF4| DMA_FLAG_HTIF4);
   
	DMA_Cmd(DMA2_Stream4, DISABLE);

	DMA_StructInit(&DMA_InitStructure);
	DMA_InitStructure.DMA_Channel = DMA_Channel_0;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(ADC1->DR);
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)BufferDST;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = BufferSize/2;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;   //循环模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
	DMA_Init(DMA2_Stream4, &DMA_InitStructure);

	DMA_ITConfig(DMA2_Stream4, DMA_IT_TC|DMA_IT_HT|DMA_IT_TE, ENABLE); 

	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream4_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = DMA2_Stream0_IRQn_Priority;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	DMA_Cmd(DMA2_Stream4, ENABLE);

}
#endif


int adc_low_level_open(void)
{
	int ret;
	NVIC_InitTypeDef NVIC_InitStructure;
	ADC_InitTypeDef ADC_InitStruct;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	p_dbg_enter;
	
	ADC_DeInit();

	memset(&adc, 0, sizeof(struct AUDIO_ADC));

	adc.data = (uint8_t*)mem_malloc(AUDIO_ADC_BUFF_SIZE);
	if(!adc.data)
		goto end;
	

	adc.len = AUDIO_ADC_BUFF_SIZE;

	adc.block_data = (uint8_t*)mem_malloc(PCM_HEAD_LEN + adc.len/8);
	if(!adc.block_data)
		goto end;

	adc_rx_wait = init_event();
	if(adc_rx_wait == 0)
	{
		p_err("adc_rx_wait ==0\r\n");
		goto end;
	}
	
	thread_create(adc_recv_thread,                                           //新建一个任务adc_recv_thread
                        0,
                        TASK_ADC_RECV_PKG_PRIO,
                        0,
                        TASK_ADC_RECV_PKG_STACK_SIZE,
                        "adc recv");
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC | RCC_APB2Periph_ADC1, ENABLE); 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	
	gpio_cfg((uint32_t)ADC_PORT, ADC_PIN, GPIO_Mode_AIN);//ADC123_IN3

	ADC_InitStruct.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO;
	ADC_InitStruct.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising; //不能为none
	ADC_InitStruct.ADC_NbrOfConversion = 1;
	ADC_InitStruct.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStruct.ADC_ScanConvMode = DISABLE;
	ADC_Init(ADC1, &ADC_InitStruct);
#ifdef TEST_DAC
	NVIC_InitStructure.NVIC_IRQChannel = ADC_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = ADC_IRQn_Priority;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif	
	ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 1, ADC_SampleTime_84Cycles);
	ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);
	ADC_Cmd(ADC1, ENABLE);

	TIM_TimeBaseStructure.TIM_Period = 1250;
    	TIM_TimeBaseStructure.TIM_Prescaler = 0;	
    	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

     	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Update);
	TIM_ARRPreloadConfig(TIM3, ENABLE);                                               //ADC定时器3
	
#if ADC1_CAPTURE_MODE == ADC_CAPTURE_MODE_INT
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	//ADC_SoftwareStartConv(ADC1);

	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = TIM3_IRQn_Priority;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	NVIC_EnableIRQ(TIM3_IRQn);
#endif

#if ADC1_CAPTURE_MODE == ADC_CAPTURE_MODE_DMA                                  //adc捕获并没有设置为DMA模式，而是自增模式
	DMA_ADC1Configuration((uint32_t*)adc.data, adc.len);

	ADC_DMACmd(ADC1, ENABLE);

	ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
#endif	
	TIM_Cmd(TIM3, ENABLE);

	return 0;

end:

	p_err_fun;
	
	ret = thread_exit(TASK_ADC_RECV_PKG_PRIO);
	if(ret != 0)
		p_err("thread_exit (TASK_ADC_RECV_PKG_PRIO) err:%d", ret);

	if(adc.data)
		mem_free(adc.data);

	if(adc.block_data)
		mem_free(adc.block_data);

	adc.block_data = 0;
	adc.data = 0;
		
	return -1;
}

void adc_low_level_close(void)
{
	int ret;
	p_dbg_enter;
	gpio_cfg((uint32_t)ADC_PORT, ADC_PIN, GPIO_Mode_Out_PP);//ADC123_IN0

	//ADC_DMACmd(ADC1, DISABLE);

	ADC_Cmd(ADC1, DISABLE);
	TIM_Cmd(TIM3, DISABLE);

	DMA_Cmd(DMA2_Stream4, DISABLE);
	NVIC_DisableIRQ(DMA2_Stream4_IRQn);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, DISABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC | RCC_APB2Periph_ADC1, DISABLE);

	ret = thread_exit(TASK_ADC_RECV_PKG_PRIO);
	if(ret != 0)
		p_err("thread_exit (TASK_ADC_RECV_PKG_PRIO) err:%d", ret);

	if(adc.data)
		mem_free(adc.data);
	if(adc.block_data)
		mem_free(adc.block_data);

	adc.block_data = 0;
	adc.data = 0;
}

int adc_switch_samplerate(int val)
{
	RCC_ClocksTypeDef RCC_ClocksStatus;

	uint32_t apbclock, scaler = 0, Counter;
	p_dbg("adc change samplerate:%d\n", val);
	if(val == 0)
		return -1;
	RCC_GetClocksFreq(&RCC_ClocksStatus);
	apbclock = RCC_ClocksStatus.PCLK1_Frequency;
	
	Counter = apbclock*2/(scaler + 1)/val;

	TIM_SetAutoreload(TIM3, Counter);
	return 0;
}

void TIM3_IRQHandler(void)                         //ADC定器器中断
{
	enter_interrupt();
	
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
	{
     		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
		((uint16_t*)adc.data)[adc.recv_pos++] = ADC1->DR;
		if(adc.recv_pos == adc.len/4){
			adc.int_flag = 0;
			goto end_and_sched;
		}
		
		if(adc.recv_pos == adc.len/2){
			adc.int_flag = 1;
			adc.recv_pos = 0;
			goto end_and_sched;
		}
		
	}
	exit_interrupt(0);
	return;
end_and_sched:
	wake_up(adc_rx_wait);
	exit_interrupt(1);
}

#ifdef TEST_DAC




void TIM4_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
     }
}

void ADC_IRQHandler(void)
{
	if(ADC_GetITStatus(ADC1, ADC_IT_EOC))
	{
		ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
	}
}

#endif

void audio_button_event(int event)
{
	if(event == SWITCH_EVENT_ON)
		audio_record_enable = TRUE;
	else
		audio_record_enable = FALSE;
}


/*
 * @brief  发送音频数据
 * 被adc_recv_thread调用
 */
DECLARE_MONITOR_ITEM("audio_totol_send", audio_totol_send);
void send_audio_data_to_remote(char *buff, int len)
{
	
	//mqtt_pub(buff, len);
	ADD_MONITOR_VALUE(audio_totol_send, len);
}

DECLARE_MONITOR_ITEM("DMA2_Stream4_IRQHandler_cnt", DMA2_Stream4_IRQHandler_cnt) ;

//import from test_tcpip.c
extern void send_audio_data_to_remote(char *buff, int len);
DECLARE_MONITOR_ITEM("pcm_data_block_out_cnt", pcm_data_block_out_cnt) ;
DECLARE_MONITOR_ITEM("adc_handle_cnt", adc_handle_cnt) ;
uint16_t ever_value = 0, ever_cnt = 0;


void adc_recv_thread(void *arg)                                    //麦克风接收任务 
{
	int ret, i;
	int16_t *src_data;
	uint8_t *out_data;
//	struct DATA_CHUNK *chunk;
	struct mqtt_pkg_head *head = (struct mqtt_pkg_head *)adc.block_data;
	p_dbg_enter;
	
	head->dir = MSG_DIR_DEV2APP;
	head->msg_id = MQTT_MSG_AUDIO;
	//head->reserved = SPEEX_DEC_FRAME_SIZE;
	ai_audio_wait = init_event();
	if(ai_audio_wait == 0)
	{
		p_err("ai_audio_wait ==0\r\n");
	}
    
	while(1)
	{   p_dbg_enter_task;
		ret = wait_event_timeout(adc_rx_wait, 2000);        //等待信号 adc_rx_wait  的到来
        
		if(ret == WAIT_EVENT_TIMEOUT)                      //如果信号量  adc_rx_wait到了，进入if语句，单次退出循环
		{
			continue;
		}
		
		if(adc.int_flag == 0)
			src_data = (int16_t*)adc.data;
		else
			src_data = (int16_t*)(adc.data + adc.len/2);

		adc.pending_flag = adc.int_flag;

		out_data = adc.block_data + sizeof(struct mqtt_pkg_head);

		if(!out_data)
			continue;

		INC_MONITOR_VALUE(adc_handle_cnt);                     //  X.value++    adc_handle_cnt++

		//一个声控的例子,通过声音大小控制
		//大声对mic叫一声,可以触发一个事件
		//p_dbg("[adc_recv_thread audio len%d]", adc.len/2);
		if(1)
		{
			if(ever_cnt++ < 10)	//开机后先计算出静音下的电平(平均值)
			{
				if(!ever_value)
					ever_value = src_data[0]; 
				for(i = 0; i < adc.len/4; i++)
				{
					ever_value = (ever_value+ src_data[i])/2;
				}
				p_dbg("[ever:%d]", ever_value);
			}else{

				uint32_t range = 0;
				//计算当前的声场强度(幅度)
				for(i = 0; i < adc.len/4; i++)
				{
					if(ever_value < src_data[i])
						range += src_data[i] - ever_value;
					else
						range += ever_value - src_data[i] ;
				}
				range = range/(adc.len/4);
				//p_dbg("[range:%d]", range);

				//如果幅度大于预设值,则播放下一首歌曲(如果正在播放MP3)
				if(range > web_cfg.audio.audio_alarm)
					audio_cfg.play_next = 1;
			}
		}

		if(audio_cfg.recording || audio_cfg.recording_to_flash)      
		{
			int src_len = adc.len/4;
			int frames = src_len/SPEEX_ENC_FRAME_SIZE;

			for(i = 0; i < src_len; i++)
			{
				src_data[i]  = (src_data[i] - 2048)*(32768/2048);    //转换成有符号数并放大成16位
			}
            #if 0
			for(i = 0; i < frames; i++)
			{
				ret = iot_speex_encode((uint16_t *)src_data + i*SPEEX_ENC_FRAME_SIZE, SPEEX_ENC_FRAME_SIZE, out_data,  src_len);         //对音频数据进行编码
				//play_speex_stream(out_data, ret); 

				if(ret != SPEEX_DEC_FRAME_SIZE)
					p_err_fun;

				out_data += SPEEX_DEC_FRAME_SIZE;        //out_data进过speex压缩过的音频数据
			}
			head->reserved = SPEEX_DEC_FRAME_SIZE*frames;
           #endif
			if(audio_cfg.recording){
				//mqtt_pub(adc.block_data, SPEEX_DEC_FRAME_SIZE*frames + sizeof(struct mqtt_pkg_head));

				//发往本地局域网(直接发PCM数据)，提供给手机端和pc端测试用
				//udp_broadcast(4800, (uint8_t*)src_data, src_len*2); //广播发方式发送
				//push_data_to_ai_buffer((char*)src_data, src_len*2);
                //update_ai_event();
				//p_dbg("[audio data udp_broadcast len%d]", src_len*2);
			}

			if(audio_cfg.recording_to_flash)
			{
				m25p80_write(audio_cfg.recording_to_flash_addr, src_len*2, (char *)src_data);
				audio_cfg.recording_to_flash_addr += src_len*2;
			}

			INC_MONITOR_VALUE(pcm_data_block_out_cnt);
		}

		p_dbg_exit_task;
	}
}

DECLARE_MONITOR_ITEM("adc_sample_cnt", adc_sample_cnt) ;

void DMA2_Stream4_IRQHandler()  //adc1
{
	enter_interrupt();
   

	INC_MONITOR_VALUE(DMA2_Stream4_IRQHandler_cnt);


	if(DMA_GetITStatus(DMA2_Stream4, DMA_IT_HTIF4))
	{
		DMA_ClearFlag(DMA2_Stream4,  DMA_IT_HTIF4);
		adc.int_flag = 0;
		goto end_and_sched;
	}

	if(DMA_GetITStatus(DMA2_Stream4, DMA_IT_TCIF4))
	{
		DMA_ClearFlag(DMA2_Stream4,  DMA_IT_TCIF4);
		adc.int_flag = 1;

		goto end_and_sched;
	}

	if(DMA_GetITStatus(DMA2_Stream4, DMA_IT_TEIF4))
	{
		DMA_ClearFlag(DMA2_Stream4,  DMA_IT_TEIF4);
		p_err("DMA_IT_TEIF4");
	}

	exit_interrupt(0);
	return;
end_and_sched:
	INC_MONITOR_VALUE(adc_sample_cnt);
	wake_up(adc_rx_wait);
	exit_interrupt(1);
}

#endif



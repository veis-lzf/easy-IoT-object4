#define DEBUG
#include "drivers.h"
#include "app.h"
#include "api.h"

int dht11_int_cnt = 0, dht11_int_cnt_ex = 0, dht11_recv_cnt = 0;
static uint32_t last_time = 0;

struct _dht11_data  rcv_data;

uint8_t tmp_val;

void init_dht11(void)
{
	EXTI_InitTypeDef   EXTI_InitStructure;
  	NVIC_InitTypeDef   NVIC_InitStructure;
	
	gpio_cfg((uint32_t)DHT11_DATA_GROUP, DHT11_DATA_PIN, GPIO_Mode_IPU);	
	GPIO_SET(DHT11_DATA_GROUP, DHT11_DATA_PIN);

	SYSCFG_EXTILineConfig(DHT11_DATA_EXTI_PORT, DHT11_DATA_PIN_SOURCE);
	EXTI_InitStructure.EXTI_Line = DHT11_DATA_EXTI_LINE;
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  	
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init(&EXTI_InitStructure);


	NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = EXTI2_IRQn_Priority;

  	
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	memset(&rcv_data, 0, sizeof(struct _dht11_data));

}

void dht11_recv_disable()
{
	NVIC_DisableIRQ(EXTI2_IRQn);

}

void dht11_recv_enable()
{

	EXTI_ClearITPendingBit(EXTI_Line2);
	NVIC_EnableIRQ(EXTI2_IRQn);

}

void handle_dht11_recv(uint32_t time)
{
	uint8_t bit;
	switch(dht11_int_cnt)
	{
		case 0:
		dht11_int_cnt++;	//20-40us
		break;
		case 1:
		if(time  > 140 && time < 180){
			dht11_int_cnt = 3;
			dht11_int_cnt_ex = 0;
			dht11_recv_cnt = 0;
			tmp_val = 0;
		}else{
			dht11_int_cnt++;
		}
		break;	
		case 2:
		if(time  > 140 && time < 180){
			dht11_int_cnt = 3;
			dht11_int_cnt_ex = 0;
			dht11_recv_cnt = 0;
			tmp_val = 0;
		}else{
			dht11_int_cnt = 2;
//			p_err("2:%d", time);
			dht11_recv_disable();
		}
		break;
		case 3:
		{
			if(time  > 60 && time < 100){
				bit = 0;
				//p_dbg("3:%d", time);
			}else if(time  > 100 && time < 140){
				bit = 1;
				//p_dbg("3:%d", time);
			}else{
				//p_err("3:%d", time);
				dht11_int_cnt = 0;
				dht11_recv_disable();
			}
			tmp_val <<= 1;
			tmp_val |= bit;
			dht11_int_cnt_ex++;
			if(dht11_int_cnt_ex == 8)
			{
				dht11_int_cnt_ex = 0;

				rcv_data.data[dht11_recv_cnt++] = tmp_val;
				if(dht11_recv_cnt == 5)
				{
					//p_dbg("recv end");
					//dump_hex("a", rcv_data.data, 5);
					dht11_int_cnt = 0;

					rcv_data.valid = 1;
					dht11_recv_disable();
				}
			}
		}
		break;
		default:
			dht11_int_cnt = 0;
		break;
		
	}
}

void dht11_start_read()
{
	DHT11_OUT;
	DHT11_LOW;
	sleep(30);
	dht11_int_cnt = 0;
	dht11_int_cnt_ex = 0;
	last_time = get_us_count();
	DHT11_HIGH;
	dht11_recv_enable();
	DHT11_IN;
}

int dht11_read_data(uint8_t buff[5])
{
	if(rcv_data.valid)
	{
		rcv_data.valid = 0;
		memcpy(buff, rcv_data.data, 5);
		memset(&rcv_data, 0, sizeof(struct _dht11_data));
		return 0;
	}
	return -1;
}



void __DHT11_IRQHandler(void)
{
	uint32_t time;
	time = get_us_count();
	handle_dht11_recv(time - last_time);
	last_time = time;
}



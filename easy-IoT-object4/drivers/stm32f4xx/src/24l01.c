/*
  测试方法
	1. 需要两块开发板
	2. 接收方发送命令D,注意大写
	3. 发送方发送命令C,注意大写
	4. 发送方发送命令B,发送数据
*/

#define DEBUG
#include "drivers.h"
#include "bsp.h"

#include "debug.h"

const uint8_t NRF2401_TX_ADDRESS[NRF2401_TX_ADR_WIDTH]={0x34,0x43,0x10,0x10,0x01}; //发送地址
const uint8_t NRF2401_RX_ADDRESS[NRF2401_RX_ADR_WIDTH]={0x34,0x43,0x10,0x10,0x01}; //发送地址

bool nrf_ok = 0;
wait_event_t nrf_event;
//io模拟SPI,时钟大约3.5MHZ
static uint8_t io_spi_trans_byte(uint8_t byte)
{
	uint8_t i; 
	uint8_t Temp=0x00;
	unsigned char SDI; 
	for (i = 0; i < 8; i++)
	{
		GPIO_CLR(NRF_2401_CLK_GROUP, NRF_2401_CLK_PIN);
       
		if (byte&0x80)      
		{
			GPIO_SET(NRF_2401_MOSI_GROUP, NRF_2401_MOSI_PIN);
		}
		else
		{
			GPIO_CLR(NRF_2401_MOSI_GROUP, NRF_2401_MOSI_PIN);
		}
		byte <<= 1;  
         	GPIO_SET(NRF_2401_CLK_GROUP, NRF_2401_CLK_PIN);
	 
         	SDI = GPIO_STAT(NRF_2401_MISO_GROUP, NRF_2401_MISO_PIN);
         	Temp<<=1;
        
        	if(SDI)
         	{
              	Temp++;
         	}
         	GPIO_CLR(NRF_2401_CLK_GROUP, NRF_2401_CLK_PIN);
 	}
 
	return Temp; //返回读到miso输入的值     
   
}

uint8_t nrf_recv_buff[32];
void nrf2401_recv_thread(void *arg)
{
	uint8_t sta;
	
	while(1)
	{
		wait_event_timeout(nrf_event, 1000);

		sta=NRF24L01_Read_Reg(NRF2401_STATUS);
		NRF24L01_Write_Reg(NRF2401_WRITE_REG+NRF2401_STATUS, sta);
		//p_dbg("sta:%x", sta);

		if(sta & NRF2401_TX_FIFO_FULL)
		{
			NRF24L01_Write_Reg(NRF2401_FLUSH_TX,0xff);
			p_dbg("NRF2401_TX_FIFO_FULL");
		}

		if(sta &  NRF2401_MAX_TX )
		{
			NRF24L01_Write_Reg(NRF2401_FLUSH_TX,0xff);
			//RX_Mode();
			p_dbg("NRF2401_MAX_TX");
		}

		if(sta &  NRF2401_TX_OK )
		{
			p_dbg("NRF2401_TX_OK");
			//RX_Mode();
		}

		if(sta &  NRF2401_RX_OK  )
		{
			p_dbg("NRF2401_RX_OK");
			NRF24L01_Read_Buf(NRF2401_RD_RX_PLOAD,nrf_recv_buff,NRF2401_RX_PLOAD_WIDTH);//读取数据
			NRF24L01_Write_Reg(NRF2401_FLUSH_RX,0xff);//清除RX FIFO寄存器 
			dump_hex("data", nrf_recv_buff, 32);
		}

	}
}
							    
//初始化24L01的IO口
void NRF24L01_Init(void)
{
	EXTI_InitTypeDef   EXTI_InitStructure;
  	NVIC_InitTypeDef   NVIC_InitStructure;
	int ret;
	p_dbg_enter;
	gpio_cfg((uint32_t)NRF_2401_CLK_GROUP, NRF_2401_CLK_PIN, GPIO_Mode_Out_PP);
	gpio_cfg((uint32_t)NRF_2401_MISO_GROUP, NRF_2401_MISO_PIN, GPIO_Mode_IPU);
	gpio_cfg((uint32_t)NRF_2401_MOSI_GROUP, NRF_2401_MOSI_PIN, GPIO_Mode_Out_PP);
	gpio_cfg((uint32_t)NRF_2401_CS_GROUP, NRF_2401_CS_PIN, GPIO_Mode_Out_PP);
	gpio_cfg((uint32_t)NRF_2401_CE_GROUP, NRF_2401_CE_PIN, GPIO_Mode_Out_PP);
	gpio_cfg((uint32_t)NRF_2401_IRQ_GROUP, NRF_2401_IRQ_PIN, GPIO_Mode_IPU);

	NRF_2401_CE_LOW;	//使能24L01
	NRF_2401_CS_HIGH;	//SPI片选取消		

	ret = NRF24L01_Check();
	if(ret){
		p_dbg("2401 err");
		return;
	}

	nrf_event = init_event();

	SYSCFG_EXTILineConfig(NRF_2401_SOURCE_GROUP, NRF_2401_SOURCE_PIN);
	EXTI_InitStructure.EXTI_Line = NRF_2401_LINE;
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  	//红外接收探头默认是高电平
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init(&EXTI_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = EXTI15_10_IRQn_Priority;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	//RX_Mode();

	thread_create(nrf2401_recv_thread, 0, TASK_NRF_PRIO, 0, TASK_NRF_STACK_SIZE, "nrf2401_recv_thread");

	nrf_ok = 1;
}
//检测24L01是否存在
//返回值:0，成功;1，失败	
uint8_t NRF24L01_Check(void)
{
	uint8_t buf[5]={0XA5,0XA5,0XA5,0XA5,0XA5};
	uint8_t i;
	
	NRF24L01_Write_Buf(NRF2401_WRITE_REG+NRF2401_TX_ADDR,buf,5);//写入5个字节的地址.	
	NRF24L01_Read_Buf(NRF2401_TX_ADDR,buf,5); //读出写入的地址  
	for(i=0;i<5;i++)if(buf[i]!=0XA5)break;	 							   
	if(i!=5)return 1;//检测24L01错误	
	return 0;		 //检测到24L01
}	 	 
//SPI写寄存器
//reg:指定寄存器地址
//value:写入的值
uint8_t NRF24L01_Write_Reg(uint8_t reg,uint8_t value)
{
	uint8_t status;	
   	NRF_2401_CS_LOW;                 //使能SPI传输
  	status =io_spi_trans_byte(reg);//发送寄存器号 
  	io_spi_trans_byte(value);      //写入寄存器的值
  	NRF_2401_CS_HIGH;                 //禁止SPI传输	   
  	return(status);       			//返回状态值
}
//读取SPI寄存器值
//reg:要读的寄存器
uint8_t NRF24L01_Read_Reg(uint8_t reg)
{
	uint8_t reg_val;	    
 	NRF_2401_CS_LOW;          //使能SPI传输		
  	io_spi_trans_byte(reg);   //发送寄存器号
  	reg_val=io_spi_trans_byte(0XFF);//读取寄存器内容
  	NRF_2401_CS_HIGH;          //禁止SPI传输		    
  	return(reg_val);           //返回状态值
}	
//在指定位置读出指定长度的数据
//reg:寄存器(位置)
//*pBuf:数据指针
//len:数据长度
//返回值,此次读到的状态寄存器值 
uint8_t NRF24L01_Read_Buf(uint8_t reg,uint8_t *pBuf,uint8_t len)
{
	uint8_t status,u8_ctr;	       
  	NRF_2401_CS_LOW;           //使能SPI传输
  	status=io_spi_trans_byte(reg);//发送寄存器值(位置),并读取状态值   	   
 	for(u8_ctr=0;u8_ctr<len;u8_ctr++)pBuf[u8_ctr]=io_spi_trans_byte(0XFF);//读出数据
  	NRF_2401_CS_HIGH;       //关闭SPI传输
  	return status;        //返回读到的状态值
}
//在指定位置写指定长度的数据
//reg:寄存器(位置)
//*pBuf:数据指针
//len:数据长度
//返回值,此次读到的状态寄存器值
uint8_t NRF24L01_Write_Buf(uint8_t reg, uint8_t *pBuf, uint8_t len)
{
	uint8_t status,u8_ctr;	    
 	NRF_2401_CS_LOW;          //使能SPI传输
  	status = io_spi_trans_byte(reg);//发送寄存器值(位置),并读取状态值
  	for(u8_ctr=0; u8_ctr<len; u8_ctr++)io_spi_trans_byte(*pBuf++); //写入数据	 
  	NRF_2401_CS_HIGH;       //关闭SPI传输
  	return status;          //返回读到的状态值
}				   
//启动NRF24L01发送一次数据
//txbuf:待发送数据首地址
//返回值:发送完成状况
uint8_t NRF24L01_TxPacket(uint8_t *txbuf)
{
	uint8_t sta;
 	NRF_2401_CE_LOW;
  	NRF24L01_Write_Buf(NRF2401_WR_TX_PLOAD,txbuf,NRF2401_TX_PLOAD_WIDTH);//写数据到TX BUF  32个字节
 	NRF_2401_CE_HIGH;//启动发送	   
	while(NRF24L01_IRQ!=0);//等待发送完成
	sta=NRF24L01_Read_Reg(NRF2401_STATUS);  //读取状态寄存器的值	   
	NRF24L01_Write_Reg(NRF2401_WRITE_REG+NRF2401_STATUS,sta); //清除TX_DS或MAX_RT中断标志
	if(sta&NRF2401_MAX_TX)//达到最大重发次数
	{
		NRF24L01_Write_Reg(NRF2401_FLUSH_TX,0xff);//清除TX FIFO寄存器 
		return NRF2401_MAX_TX; 
	}
	if(sta&NRF2401_TX_OK)//发送完成
	{
		return NRF2401_TX_OK;
	}
	return 0xff;//其他原因发送失败
}
//启动NRF24L01发送一次数据
//txbuf:待发送数据首地址
//返回值:0，接收完成；其他，错误代码
uint8_t NRF24L01_RxPacket(uint8_t *rxbuf)
{
	uint8_t sta;		    							   
	sta=NRF24L01_Read_Reg(NRF2401_STATUS);  //读取状态寄存器的值    	 
	NRF24L01_Write_Reg(NRF2401_WRITE_REG+NRF2401_STATUS,sta); //清除TX_DS或MAX_RT中断标志
	if(sta&NRF2401_RX_OK)//接收到数据
	{
		NRF24L01_Read_Buf(NRF2401_RD_RX_PLOAD,rxbuf,NRF2401_RX_PLOAD_WIDTH);//读取数据
		NRF24L01_Write_Reg(NRF2401_FLUSH_RX,0xff);//清除RX FIFO寄存器 
		return 0; 
	}	   
	return 1;//没收到任何数据
}				

//该函数初始化NRF24L01到RX模式
//设置RX地址,写RX数据宽度,选择RF频道,波特率和LNA HCURR
//当CE变高后,即进入RX模式,并可以接收数据了		   
void RX_Mode(void)
{
	p_dbg_enter;
	NRF_2401_CE_LOW;	  
  	NRF24L01_Write_Buf(NRF2401_WRITE_REG+NRF2401_RX_ADDR_P0,(uint8_t*)NRF2401_RX_ADDRESS,NRF2401_RX_ADR_WIDTH);//写RX节点地址
	  
  	NRF24L01_Write_Reg(NRF2401_WRITE_REG+NRF2401_EN_AA,0x01);    //使能通道0的自动应答    
  	NRF24L01_Write_Reg(NRF2401_WRITE_REG+NRF2401_EN_RXADDR,0x01);//使能通道0的接收地址  	 
  	NRF24L01_Write_Reg(NRF2401_WRITE_REG+NRF2401_RF_CH,40);	     //设置RF通信频率		  
  	NRF24L01_Write_Reg(NRF2401_WRITE_REG+NRF2401_RX_PW_P0,NRF2401_RX_PLOAD_WIDTH);//选择通道0的有效数据宽度 	    
  	NRF24L01_Write_Reg(NRF2401_WRITE_REG+NRF2401_RF_SETUP,0x0f);//设置TX发射参数,0db增益,2Mbps,低噪声增益开启   
  	NRF24L01_Write_Reg(NRF2401_WRITE_REG+NRF2401_CONFIG, 0x0f);//配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,接收模式 
  	NRF_2401_CE_HIGH; //CE为高,进入接收模式 
}						 
//该函数初始化NRF24L01到TX模式
//设置TX地址,写TX数据宽度,设置RX自动应答的地址,填充TX发送数据,选择RF频道,波特率和LNA HCURR
//PWR_UP,CRC使能
//当CE变高后,即进入RX模式,并可以接收数据了		   
//CE为高大于10us,则启动发送.	 
void TX_Mode(void)
{									
	p_dbg_enter;
	NRF_2401_CE_LOW;	    
  	NRF24L01_Write_Buf(NRF2401_WRITE_REG+NRF2401_TX_ADDR,(uint8_t*)NRF2401_TX_ADDRESS,NRF2401_TX_ADR_WIDTH);//写TX节点地址 
  	NRF24L01_Write_Buf(NRF2401_WRITE_REG+NRF2401_RX_ADDR_P0,(uint8_t*)NRF2401_RX_ADDRESS,NRF2401_RX_ADR_WIDTH); //设置TX节点地址,主要为了使能ACK	  

  	NRF24L01_Write_Reg(NRF2401_WRITE_REG+NRF2401_EN_AA,0x01);     //使能通道0的自动应答    
  	NRF24L01_Write_Reg(NRF2401_WRITE_REG+NRF2401_EN_RXADDR,0x01); //使能通道0的接收地址  
  	NRF24L01_Write_Reg(NRF2401_WRITE_REG+NRF2401_SETUP_RETR,0x1a);//设置自动重发间隔时间:500us + 86us;最大自动重发次数:10次
  	NRF24L01_Write_Reg(NRF2401_WRITE_REG+NRF2401_RF_CH,40);       //设置RF通道为40
  	NRF24L01_Write_Reg(NRF2401_WRITE_REG+NRF2401_RF_SETUP,0x0f);  //设置TX发射参数,0db增益,2Mbps,低噪声增益开启   
  	NRF24L01_Write_Reg(NRF2401_WRITE_REG+NRF2401_CONFIG,0x0e);    //配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,接收模式,开启所有中断
	NRF_2401_CE_HIGH;//CE为高,10us后启动发送
}		  

void nrf2401_tx(uint8_t *data, int len)
{
	p_dbg_enter;

	if(!nrf_ok)
		return;
	
	//TX_Mode();

	NRF_2401_CE_LOW;
  	NRF24L01_Write_Buf(NRF2401_WR_TX_PLOAD,data,NRF2401_TX_PLOAD_WIDTH);//写数据到TX BUF  32个字节
 	NRF_2401_CE_HIGH;	//启动发送	   


	p_dbg_exit;
}

/*
*NRF中断
*
*/
void __EXTI11_IRQHandler(void)
{
	if(nrf_event)
	wake_up(nrf_event);
}



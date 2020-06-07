#define DEBUG
#include "stdarg.h"
#include "stdio.h"

#include "drivers.h"
#include "bsp.h"
#include "debug.h"

//添加

#define SlaveAddress   0x46											   //BH1750FVI模块ADDR接低，故寻址为0110011
//extern GPIO_InitTypeDef GPIO_InitStructure;
#define SCL_H    GPIO_SetBits(GPIOD,GPIO_Pin_10);					   
#define SCL_L    GPIO_ResetBits(GPIOD,GPIO_Pin_10);
#define SDA_H    GPIO_SetBits(GPIOD,GPIO_Pin_12);
#define SDA_L    GPIO_ResetBits(GPIOD,GPIO_Pin_12);

extern void delay_us(uint32_t count);
extern void delay_ms(uint32_t us);
void BH1750_Start(void);	                                           //IIC开始函数
void BH1750_SendByte(u8);											   //IIC发送字节函数
u8   BH1750_ReadSlaveAck(void);										   //读取SDA响应信号
void BH1750_WriteCommand(u8);										   //写数据命令，调用SendByte函数
float BH1750_ReadResult(void);										   //读取转换结果，浮点型输出
u8   BH1750_ReadByte(void);											   //读取转换结果，字节输出
void BH1750_SendACK(u8);											   //主机发送应答ACK及非应答
void BH1750_Stop(void);												   //IIC停止函数
void IIC_InputConfig(void);											   //I/O切换为浮空输入
void IIC_OutputConfig(void);

uint32_t BH1750FVI(void)
{
  float  light=0;										//光照强度
  uint32_t ret;
  
  BH1750_WriteCommand(0x01);						//上电初始化
  delay_ms(100);

 
  BH1750_WriteCommand(0x01);                      //通电
  BH1750_WriteCommand(0x10);                      //连续高分辨率模式
  delay_ms(100);                                 //延时输出

  light= (BH1750_ReadResult());                     //连续读出数据，存储在BUF中 

  //p_dbg("light strength raw[%f]", light);
  ret = (uint32_t)light;
  return(ret);
}


/**************************************
功能：发送起始信号
**************************************/
void BH1750_Start()
{
    IIC_OutputConfig();
    SDA_H;                                          //拉高数据线
    SCL_H;                                          //拉高时钟线
    delay_us(5);                                    //延时
    SDA_L;                                          //产生下降沿
    delay_us(5);                                    //延时
    SCL_L;                                          //拉低时钟线
}


/**************************************
功能：向IIC总线发送一个字节数据
**************************************/
void BH1750_SendByte(u8 dat)
{
    u8 i;
    IIC_OutputConfig();

    for (i=0; i<8; i++)                             //8位计数器
    {
        if(dat&0x80){ SDA_H; }    
        else        { SDA_L; }
          
        dat <<= 1;                                  //移出数据的最高位
        SCL_H;                                      //拉高时钟线
        delay_us(5);                                //延时
        SCL_L;                                      //拉低时钟线
        delay_us(5);                                //延时
    }

    BH1750_ReadSlaveAck();

}


/**************************************
功能：读取SDA应答信号
**************************************/
u8 BH1750_ReadSlaveAck()       
{
    u8 ACK;
    IIC_InputConfig();

    SCL_H;                                           //拉高时钟线
    delay_us(5);                                     //延时
    ACK=GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_12);    //读应答信号

    SCL_L;                                           //拉低时钟线
    delay_us(5);                                     //延时

    return ACK;
}

/****************************************
功能：发送一个字节的数据，调用SendByte函数
*****************************8888888****/
void BH1750_WriteCommand(u8 REG_Address)
{
    BH1750_Start();                                  //起始信号
    BH1750_SendByte(SlaveAddress);                   //发送设备地址+写信号
    BH1750_SendByte(REG_Address);                    //内部寄存器地址，请参考中文pdf22页 
    BH1750_Stop();                                   //发送停止信号
}

/*********************************
功能：读取模块转换数据，浮点型输出
*********************************/
float BH1750_ReadResult()
{
    u8  resultH=0,resultL=0;
    float result=0;

    BH1750_Start();                                  //起始信号
    BH1750_SendByte(SlaveAddress+1);                 //发送设备地址+写信号

    resultH=BH1750_ReadByte();
    BH1750_SendACK(0);
    resultL=BH1750_ReadByte();
    BH1750_SendACK(1);
    //p_dbg("light resultH[%x]resultL[%x]", resultH, resultL);
    
    //result=(resultH*pow(2,8)+resultL)/1.2;
    result = (float)(((int)resultH << 8) + resultL)/1.2;

    BH1750_Stop();                                   //发送停止信号

    return result;

}
/**************************************
功能：从IIC总线接收一个字节数据
**************************************/
u8 BH1750_ReadByte()
{
    u8 i;
    u8 dat = 0;
    IIC_OutputConfig();

    SDA_H;                                                  //使能内部上拉,准备读取数据,
    delay_us(2);

    IIC_InputConfig();
    for (i=0; i<8; i++)                                     //8位计数器
    {
        dat <<= 1;           
        SCL_H;                                              //拉高时钟线
        delay_us(5);                                        //延时
        dat |= GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_12);    //读数据               
        SCL_L;                                              //拉低时钟线
        delay_us(5);                                        //延时
    }
    return dat;
}


/**************************************
功能：发送应答信号
参数: ack (0:ACK 1:NAK)
**************************************/
void BH1750_SendACK(u8 ack)
{
    IIC_OutputConfig();

    if(ack==1) {SDA_H; }
    if(ack==0) {SDA_L; }
    SCL_H;                                       //拉高时钟线
    delay_us(5);                                 //延时
    SCL_L;                                       //拉低时钟线
    delay_us(5);                                 //延时
}


/**************************************
功能:IIC发送停止信号
**************************************/
void BH1750_Stop()
{
    IIC_OutputConfig();
    SDA_L;                                       //拉低数据线
    SCL_H;                                       //拉高时钟线
    delay_us(5);                                 //延时
    SDA_H;                                       //产生上升沿
    delay_us(5);                                 //延时
}




/***************************************
功能：SDA设置为浮空输入
****************************************/
void IIC_InputConfig()
{
    gpio_cfg((uint32_t)GPIOD, GPIO_Pin_12, GPIO_Mode_IN_FLOATING);
}

/***************************************
功能：IIC设置为推挽输出
****************************************/
void IIC_OutputConfig()
{
    gpio_cfg((uint32_t)GPIOD, GPIO_Pin_12, GPIO_Mode_Out_PP);
}

void BH1750_I2C_Init(void)
{
    //GPIO_InitTypeDef  GPIO_InitStructure; 
    /* Configure I2C1 pins: SCL and SDA */
    gpio_cfg((uint32_t)GPIOD, GPIO_Pin_10, GPIO_Mode_Out_PP);
    gpio_cfg((uint32_t)GPIOD, GPIO_Pin_12, GPIO_Mode_Out_PP);
}
uint32_t get_BH1750_value(void)
{
    return BH1750FVI();
}


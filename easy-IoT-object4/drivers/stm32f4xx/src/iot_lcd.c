//#define DEBUG
#include "debug.h"
#include "drivers.h"
#include "app.h"
#include "api.h"

#if  ENABLE_LCD == 1

__inline static void io_spi_trans_byte(u8 byte)
{
	int i; 
	for (i = 0; i < 8; i++)
	{
		if (byte&0x80)      
		{
			LCD_MOSI_HIGH;
		}
		else
		{
			LCD_MOSI_LOW;
		}
		byte <<= 1;  
		LCD_CLK_LOW;
		LCD_CLK_HIGH;
 	}
}

//向液晶屏写一个8位指令
__inline void Lcd_WriteIndex(u8 Index)
{
   //SPI 写命令时序开始
   LCD_CS_LOW;
   LCD_DC_LOW;
	 io_spi_trans_byte(Index);
   LCD_CS_HIGH;
}

//向液晶屏写一个8位数据
__inline void Lcd_WriteData(u8 Data)
{
   LCD_CS_LOW;
   LCD_DC_HIGH;
   io_spi_trans_byte(Data);
   LCD_CS_HIGH; 
}
//向液晶屏写一个16位数据
__inline void LCD_WriteData_16Bit(u16 Data)
{
   LCD_CS_LOW;
   LCD_DC_HIGH;
	 io_spi_trans_byte(Data>>8); 	//写入高8位数据
	 io_spi_trans_byte(Data); 			//写入低8位数据
   LCD_CS_HIGH; 
}

void Lcd_WriteReg(u8 Index,u8 Data)
{
	Lcd_WriteIndex(Index);
  Lcd_WriteData(Data);
}

void Lcd_Reset(void)
{
	LCD_RST_LOW;
	sleep(100);
	LCD_RST_HIGH;
	sleep(50);
}

/*************************************************
函数名：LCD_Set_Region
功能：设置lcd显示区域，在此区域写点数据自动换行
入口参数：xy起点和终点
返回值：无
*************************************************/
void Lcd_SetRegion(u16 x_start,u16 y_start,u16 x_end,u16 y_end)
{		
	
	Lcd_WriteIndex(0x2a);
	Lcd_WriteData(0x00);
	Lcd_WriteData(x_start+2);
	Lcd_WriteData(0x00);
	Lcd_WriteData(x_end+2);

	Lcd_WriteIndex(0x2b);
	Lcd_WriteData(0x00);
	Lcd_WriteData(y_start+3);
	Lcd_WriteData(0x00);
	Lcd_WriteData(y_end+3);
	
	Lcd_WriteIndex(0x2c);


}

/*************************************************
函数名：LCD_Set_XY
功能：设置lcd显示起始点
入口参数：xy坐标
返回值：无
*************************************************/
void Lcd_SetXY(u16 x,u16 y)
{
  	Lcd_SetRegion(x,y,x,y);
}

	
/*************************************************
函数名：LCD_DrawPoint
功能：画一个点
入口参数：无
返回值：无
*************************************************/
void Gui_DrawPoint(u16 x,u16 y,u16 Data)
{
/*	static u16 next_x = 0, next_y = 0;

	if(x == next_x && y == next_y)
		;
	else{
		Lcd_SetRegion(x,y, X_MAX_PIXEL - x, Y_MAX_PIXEL -y);
		next_x = x;
		next_y = y;
	}
	next_x++;
	if(x >= X_MAX_PIXEL)
	{
		next_x = 0;
		next_y += 1;
		if(next_y >= Y_MAX_PIXEL) {
			p_err_miss; //需要测试
			next_y = 0;
		}
	}*/
	Lcd_SetRegion(x,y, x + 1, y+1);
	LCD_WriteData_16Bit(Data);

}    

/*****************************************
 函数功能：读TFT某一点的颜色                          
 出口参数：color  点颜色值                                 
******************************************/
unsigned int Lcd_ReadPoint(u16 x,u16 y)
{
  //unsigned int Data;
  //Lcd_SetXY(x,y);

  //Lcd_ReadData();//丢掉无用字节
  //Data=Lcd_ReadData();
  //Lcd_WriteData(Data);
  return 0;
}
/*************************************************
函数名：Lcd_Clear
功能：全屏清屏函数
入口参数：填充颜色COLOR
返回值：无
*************************************************/
void Lcd_Clear(u16 Color)               
{	
   unsigned int i,m;
   Lcd_SetRegion(0,0,X_MAX_PIXEL-1,Y_MAX_PIXEL-1);
   Lcd_WriteIndex(0x2C);
   for(i=0;i<X_MAX_PIXEL;i++)
    for(m=0;m<Y_MAX_PIXEL;m++)
    {	
	  	LCD_WriteData_16Bit(Color);
		sleep(10);
    }   
    Lcd_SetRegion(0,0,X_MAX_PIXEL-1,Y_MAX_PIXEL-1);	
}

int init_lcd(void)
{
	p_dbg_enter;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

	gpio_cfg((uint32_t)LCD_CLK_GROUP, LCD_CLK_PIN, GPIO_Mode_Out_PP);
	gpio_cfg((uint32_t)LCD_MOSI_GROUP, LCD_MOSI_PIN, GPIO_Mode_Out_PP);
	
	gpio_cfg((uint32_t)LCD_DC_GROUP, LCD_DC_PIN, GPIO_Mode_Out_PP);
	gpio_cfg((uint32_t)LCD_RST_GROUP, LCD_RST_PIN, GPIO_Mode_Out_PP);
	gpio_cfg((uint32_t)LCD_CS_GROUP, LCD_CS_PIN, GPIO_Mode_Out_PP);
	gpio_cfg((uint32_t)LCD_LED_GROUP, LCD_LED_PIN, GPIO_Mode_Out_PP);

	LCD_CS_HIGH;
	LCD_CLK_HIGH;
	LCD_DC_HIGH;
	LCD_MOSI_HIGH;
	LCD_LED_HIGH;
	LCD_RST_LOW;
	
	Lcd_Reset(); //Reset before LCD Init.

	//LCD Init For 1.44Inch LCD Panel with ST7735R.
	Lcd_WriteIndex(0x11);//Sleep exit 
	sleep (120);
	
	//ST7735R Frame Rate
	Lcd_WriteIndex(0xB1); 
	Lcd_WriteData(0x01); 
	Lcd_WriteData(0x2C); 
	Lcd_WriteData(0x2D); 

	Lcd_WriteIndex(0xB2); 
	Lcd_WriteData(0x01); 
	Lcd_WriteData(0x2C); 
	Lcd_WriteData(0x2D); 

	Lcd_WriteIndex(0xB3); 
	Lcd_WriteData(0x01); 
	Lcd_WriteData(0x2C); 
	Lcd_WriteData(0x2D); 
	Lcd_WriteData(0x01); 
	Lcd_WriteData(0x2C); 
	Lcd_WriteData(0x2D); 
	
	Lcd_WriteIndex(0xB4); //Column inversion 
	Lcd_WriteData(0x07); 
	
	//ST7735R Power Sequence
	Lcd_WriteIndex(0xC0); 
	Lcd_WriteData(0xA2); 
	Lcd_WriteData(0x02); 
	Lcd_WriteData(0x84); 
	Lcd_WriteIndex(0xC1); 
	Lcd_WriteData(0xC5); 

	Lcd_WriteIndex(0xC2); 
	Lcd_WriteData(0x0A); 
	Lcd_WriteData(0x00); 

	Lcd_WriteIndex(0xC3); 
	Lcd_WriteData(0x8A); 
	Lcd_WriteData(0x2A); 
	Lcd_WriteIndex(0xC4); 
	Lcd_WriteData(0x8A); 
	Lcd_WriteData(0xEE); 
	
	Lcd_WriteIndex(0xC5); //VCOM 
	Lcd_WriteData(0x0E); 
	
	Lcd_WriteIndex(0x36); //MX, MY, RGB mode 
	Lcd_WriteData(0xC8); 
	
	//ST7735R Gamma Sequence
	Lcd_WriteIndex(0xe0); 
	Lcd_WriteData(0x0f); 
	Lcd_WriteData(0x1a); 
	Lcd_WriteData(0x0f); 
	Lcd_WriteData(0x18); 
	Lcd_WriteData(0x2f); 
	Lcd_WriteData(0x28); 
	Lcd_WriteData(0x20); 
	Lcd_WriteData(0x22); 
	Lcd_WriteData(0x1f); 
	Lcd_WriteData(0x1b); 
	Lcd_WriteData(0x23); 
	Lcd_WriteData(0x37); 
	Lcd_WriteData(0x00); 	
	Lcd_WriteData(0x07); 
	Lcd_WriteData(0x02); 
	Lcd_WriteData(0x10); 

	Lcd_WriteIndex(0xe1); 
	Lcd_WriteData(0x0f); 
	Lcd_WriteData(0x1b); 
	Lcd_WriteData(0x0f); 
	Lcd_WriteData(0x17); 
	Lcd_WriteData(0x33); 
	Lcd_WriteData(0x2c); 
	Lcd_WriteData(0x29); 
	Lcd_WriteData(0x2e); 
	Lcd_WriteData(0x30); 
	Lcd_WriteData(0x30); 
	Lcd_WriteData(0x39); 
	Lcd_WriteData(0x3f); 
	Lcd_WriteData(0x00); 
	Lcd_WriteData(0x07); 
	Lcd_WriteData(0x03); 
	Lcd_WriteData(0x10);  
	
	Lcd_WriteIndex(0x2a);
	Lcd_WriteData(0x00);
	Lcd_WriteData(0x00);
	Lcd_WriteData(0x00);
	Lcd_WriteData(0x7f);

	Lcd_WriteIndex(0x2b);
	Lcd_WriteData(0x00);
	Lcd_WriteData(0x00);
	Lcd_WriteData(0x00);
	Lcd_WriteData(0x9f);
	
	Lcd_WriteIndex(0xF0); //Enable test command  
	Lcd_WriteData(0x01); 
	Lcd_WriteIndex(0xF6); //Disable ram power save mode 
	Lcd_WriteData(0x00); 
	
	Lcd_WriteIndex(0x3A); //65k mode 
	Lcd_WriteData(0x05); 
	
	
	Lcd_WriteIndex(0x29);//Display on	 

	Lcd_SetRegion(0,0,X_MAX_PIXEL-1,Y_MAX_PIXEL-1);

	//Lcd_Clear(0x00ff00);
	
	return 0;
}

#endif

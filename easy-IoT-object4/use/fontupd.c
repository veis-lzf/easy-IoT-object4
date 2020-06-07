#include "fontupd.h"  
#include "drivers.h"    
#include "string.h"
#include "sys_misc.h"



//字库区域占用的总扇区数大小(3个字库+unigbk表+字库信息=3238700字节,约占791个W25QXX扇区)
#define FONTSECSIZE	 	791
//字库存放起始地址 
#define FONTINFOADDR 	1024*1024*12 					//Explorer STM32F4是从1M地址以后开始存放字库
														//前面4M被fatfs占用了.
														//4M以后紧跟3个字库+UNIGBK.BIN,总大小11.59M,被字库占用了,不能动!
														//15.59M以后,用户可以自由使用.建议用最后的100K字节比较好.
														
//用来保存字库基本信息，地址，大小等
_font_info ftinfo;


u8 font_init(void)
{		
	u8 t=0;
	m25p80_init();  
	while(t<10)//连续读取10次,都是错误,说明确实是有问题,得更新字库了
	{
		t++;
		m25p80_read(FONTINFOADDR,sizeof(ftinfo),(u8*)&ftinfo);//读出ftinfo结构体数据
		if(ftinfo.fontok==0XAA)break;
		delay_ms(20);
	}
	if(ftinfo.fontok!=0XAA)return 1;
	return 0;		    
}



































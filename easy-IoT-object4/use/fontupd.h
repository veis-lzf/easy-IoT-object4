#ifndef __FONTUPD_H__
#define __FONTUPD_H__	 
#include <stm32f4xx.h>

//字体信息保存地址,占33个字节,第1个字节用于标记字库是否存在.后续每8个字节一组,分别保存起始地址和文件大小														   
extern u32 FONTINFOADDR;	
//字库信息结构体定义
//用来保存字库基本信息，地址，大小等
__packed typedef struct 
{
	u8 fontok;				//字库存在标志，0XAA，字库正常；其他，字库不存在
	u32 ugbkaddr; 			//unigbk的地址
	u32 ugbksize;			//unigbk的大小	 
	u32 f48addr;			//gbk48地址	
	u32 gbk48size;			//gbk48的大小	 
	u32 f32addr;			//gbk40地址
	u32 gbk32size;			//gbk40的大小		 
	u32 f24addr;			//gbk32地址
	u32 gbk24size;			//gbk32的大小 
}_font_info; 


extern _font_info ftinfo;	//字库信息结构体


u8 font_init(void);										//初始化字库
#endif



















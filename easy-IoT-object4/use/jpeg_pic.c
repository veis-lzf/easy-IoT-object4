#include   "jpeg.h"
#include   "jpeg2.h"
#include   "jpeg3.h"
#include   "drivers.h"




#define PIC_6_BASE_ADDR	(2*1024*1024)   
#define PIC_6_BASE_ADDR_2	(2*1024*1024+81) 
#define PIC_6_BASE_ADDR_3	(2*1024*1024+161) 


void show_pic(u8 *pData )    
{
     
 DrawRGBImage(0,0, 320,240,(u8 *)&pData);

}

int show_pic_init(u8 pic)
{

  switch(pic)
  {
      case 1: DrawRGBImage(0,0,480,320,(u8 *)&gImage_1[0]);
           break;
      case 2:  DrawRGBImage(0,0,480,320,(u8 *)&gImage_2[0]);
           break;
  
  }      
   
  
   

     return 0;
}


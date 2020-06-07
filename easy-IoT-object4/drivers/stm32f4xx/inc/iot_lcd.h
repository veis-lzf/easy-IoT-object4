#ifndef _IOT_LCD_H
#define _IOT_LCD_H

#define X_MAX_PIXEL	        128
#define Y_MAX_PIXEL	        128

void Gui_DrawPoint(u16 x,u16 y,u16 Data);
unsigned int Lcd_ReadPoint(u16 x,u16 y);
void Lcd_Clear(u16 Color);               

int init_lcd(void);

#endif

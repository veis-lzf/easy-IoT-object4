#ifndef _MISC_H
#define _MISC_H


typedef enum
{
	BUTTON_EVENT_IDLE, BUTTON_EVENT_UP, BUTTON_EVENT_DOWN, BUTTON_EVENT_TOGGLE_ON, BUTTON_EVENT_TOGGLE_OFF
} BUTTON_EVENT;

typedef enum
{
	SWITCH_EVENT_OFF, SWITCH_EVENT_ON
} SWITCH_EVENT;

void usr_gpio_init(void);
void init_systick(void);
void misc_init(void);
void led_switch(uint8_t value);
void led_bright(uint8_t value);
void button_stat_callback(void);
void update_relay_status(int relay, int value);

void soft_reset(void);
void delay_1us(void);
void delay_us(uint32_t us);
void indicate_led_twink_start(int fre);
void indicate_led_twink_stop(void);
void ind2_status_set(int on);



#endif

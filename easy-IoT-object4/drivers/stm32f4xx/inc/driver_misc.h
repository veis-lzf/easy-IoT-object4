#ifndef _DRIVER_MISC_H
#define _DRIVER_MISC_H

//define UART PORT

#define UART1_RX_PORT_GROUP 		GPIOA
#define UART1_TX_PORT_GROUP 		GPIOA
#define UART1_TX_PIN 				GPIO_Pin_9
#define UART1_RX_PIN 				GPIO_Pin_10
#define UART1_TX_PIN_SOURSE		GPIO_PinSource9
#define UART1_RX_PIN_SOURSE		GPIO_PinSource10

#define UART3_RX_PORT_GROUP 		GPIOD
#define UART3_TX_PORT_GROUP 		GPIOD
#define UART3_TX_PIN 				GPIO_Pin_8
#define UART3_RX_PIN 				GPIO_Pin_9
#define UART3_TX_PIN_SOURSE		GPIO_PinSource8
#define UART3_RX_PIN_SOURSE		GPIO_PinSource9

//define USB PORT

#define USB_DP_PORT_GROUP 		GPIOB
#define USB_DM_PORT_GROUP 		GPIOB

#define USB_DP_PIN 				GPIO_Pin_15
#define USB_DM_PIN 				GPIO_Pin_14

#define USB_DP_PIN_SOURSE		GPIO_PinSource15
#define USB_DM_PIN_SOURSE		GPIO_PinSource14

//define sdio PORT

#define SDIO_CLK_PORT				GPIOC
#define SDIO_CLK_PIN				GPIO_Pin_12
#define SDIO_CLK_SOURCE				GPIO_PinSource12
#define SDIO_D0_PORT				GPIOC
#define SDIO_D0_PIN					GPIO_Pin_8
#define SDIO_D0_SOURCE				GPIO_PinSource8
#define SDIO_D1_PORT				GPIOC
#define SDIO_D1_PIN					GPIO_Pin_9
#define SDIO_D1_SOURCE				GPIO_PinSource9
#define SDIO_D2_PORT				GPIOC
#define SDIO_D2_PIN					GPIO_Pin_10
#define SDIO_D2_SOURCE				GPIO_PinSource10
#define SDIO_D3_PORT				GPIOC
#define SDIO_D3_PIN					GPIO_Pin_11
#define SDIO_D3_SOURCE				GPIO_PinSource11

#define SDIO_CMD_PORT				GPIOD
#define SDIO_CMD_PIN				GPIO_Pin_2
#define SDIO_CMD_SOURCE			GPIO_PinSource2

//RESET引脚在8782模块上没用到，实际由PDN控制复位
#define SDIO_RESET_PORT 			GPIOA
#define SDIO_RESET_PIN				GPIO_Pin_11

#define SDIO_PDN_PORT 				GPIOD
#define SDIO_PDN_PIN				GPIO_Pin_3

#define SDIO_RESET_HIGH				GPIO_SET(SDIO_RESET_PORT, SDIO_RESET_PIN)
#define SDIO_RESET_LOW				GPIO_CLR(SDIO_RESET_PORT, SDIO_RESET_PIN)

#define SDIO_PDN_HIGH					GPIO_SET(SDIO_PDN_PORT, SDIO_PDN_PIN)
#define SDIO_PDN_LOW					GPIO_CLR(SDIO_PDN_PORT, SDIO_PDN_PIN)


//define flash PORT
#define FLASH_CS_PORT 				GPIOA
#define FLASH_CS_PIN 				GPIO_Pin_15

#define FLASH_CLK_PORT 			GPIOB
#define FLASH_CLK_PIN 				GPIO_Pin_3
#define FLASH_CLK_PIN_SOURCE 		GPIO_PinSource3

#define FLASH_MISO_PORT 			GPIOB
#define FLASH_MISO_PIN 				GPIO_Pin_4
#define FLASH_MISO_PIN_SOURCE  	GPIO_PinSource4

#define FLASH_MOSI_PORT 			GPIOB
#define FLASH_MOSI_PIN 				GPIO_Pin_5
#define FLASH_MOSI_PIN_SOURCE  	GPIO_PinSource5

//#define FLASH_WP_PORT 				GPIOA
//#define FLASH_WP_PIN 				GPIO_Pin_0



#define FLASH_CS_ENABLE	 	 GPIO_CLR(FLASH_CS_PORT,FLASH_CS_PIN)
#define FLASH_CS_DISABLE		GPIO_SET(FLASH_CS_PORT,FLASH_CS_PIN)

//#define FLASH_WP_ENABLE		GPIO_CLR(FLASH_WP_PORT,FLASH_WP_PIN)
//#define FLASH_WP_DISABLE		GPIO_SET(FLASH_WP_PORT,FLASH_WP_PIN)


//define DCMI PORT
#define DCMI_HS_PORT_GROUP 	GPIOA
#define DCMI_HS_PIN 				GPIO_Pin_4
#define DCMI_HS_SOURCE 		GPIO_PinSource4

#define DCMI_VS_PORT_GROUP 	GPIOB
#define DCMI_VS_PIN 				GPIO_Pin_7
#define DCMI_VS_SOURCE 		GPIO_PinSource7

#define DCMI_PCLK_PORT_GROUP GPIOA
#define DCMI_PCLK_PIN 			GPIO_Pin_6
#define DCMI_PCLK_SOURCE 		GPIO_PinSource6

#define DCMI_D0_PORT_GROUP 	GPIOC
#define DCMI_D0_PIN 				GPIO_Pin_6
#define DCMI_D0_SOURCE 			GPIO_PinSource6

#define DCMI_D1_PORT_GROUP 	GPIOC
#define DCMI_D1_PIN 				GPIO_Pin_7
#define DCMI_D1_SOURCE 			GPIO_PinSource7

#define DCMI_D2_PORT_GROUP 	GPIOE
#define DCMI_D2_PIN 				GPIO_Pin_0
#define DCMI_D2_SOURCE 			GPIO_PinSource0

#define DCMI_D3_PORT_GROUP 	GPIOE
#define DCMI_D3_PIN 				GPIO_Pin_1
#define DCMI_D3_SOURCE 			GPIO_PinSource1

#define DCMI_D4_PORT_GROUP 	GPIOE
#define DCMI_D4_PIN 				GPIO_Pin_4
#define DCMI_D4_SOURCE 			GPIO_PinSource4

#define DCMI_D5_PORT_GROUP 	GPIOB
#define DCMI_D5_PIN 				GPIO_Pin_6
#define DCMI_D5_SOURCE 			GPIO_PinSource6

#define DCMI_D6_PORT_GROUP 	GPIOB
#define DCMI_D6_PIN 				GPIO_Pin_8
#define DCMI_D6_SOURCE 			GPIO_PinSource8

#define DCMI_D7_PORT_GROUP 	GPIOB
#define DCMI_D7_PIN 				GPIO_Pin_9
#define DCMI_D7_SOURCE 			GPIO_PinSource9

#define MCO_PORT_GROUP 		 GPIOA
#define MCO_PIN 					GPIO_Pin_8
#define MCO_PIN_SOURCE 			GPIO_PinSource8

#define DCMI_RST_PORT_GROUP 	GPIOD
#define DCMI_RST_PIN 			GPIO_Pin_6

#define DCMI_PWD_PORT_GROUP 	GPIOC
#define DCMI_PWD_PIN 			GPIO_Pin_5

#define SIO_C_PORT_GROUP 		GPIOE
#define SIO_C_PIN 				GPIO_Pin_3

#define SIO_D_PORT_GROUP 		GPIOE
#define SIO_D_PIN 				GPIO_Pin_2

#define SIO_D_OUT		gpio_cfg((uint32_t)SIO_D_PORT_GROUP, SIO_D_PIN, GPIO_Mode_Out_PP);
#define SIO_D_IN			gpio_cfg((uint32_t)SIO_D_PORT_GROUP, SIO_D_PIN, GPIO_Mode_IPU);

//end define DCMI PORT

//start define AUDIO PORT
#define DAC2_PORT 	GPIOA
#define DAC2_PIN 	GPIO_Pin_5

#define ADC_PORT 	GPIOA
#define ADC_PIN 		GPIO_Pin_3


//end define audio PORT

//START ETH PORT
#define  ETH_MDIO_PORT_GROUP 		GPIOA
#define  ETH_MDIO_PIN 				GPIO_Pin_2
#define  ETH_MDIO_SOURCE 			GPIO_PinSource2

#define  ETH_MDC_PORT_GROUP 		GPIOC
#define  ETH_MDC_PIN 				GPIO_Pin_1
#define  ETH_MDC_SOURCE 			GPIO_PinSource1

#define  ETH_TXD0_PORT_GROUP 		GPIOB
#define  ETH_TXD0_PIN 				GPIO_Pin_12
#define  ETH_TXD0_SOURCE 			GPIO_PinSource12

#define  ETH_TXD1_PORT_GROUP 		GPIOB
#define  ETH_TXD1_PIN 				GPIO_Pin_13
#define  ETH_TXD1_SOURCE 			GPIO_PinSource13

#define  ETH_TX_EN_PORT_GROUP 		GPIOB
#define  ETH_TX_EN_PIN 				GPIO_Pin_11
#define  ETH_TX_EN_SOURCE 			GPIO_PinSource11

#define  ETH_RXD0_PORT_GROUP 		GPIOC
#define  ETH_RXD0_PIN 				GPIO_Pin_4
#define  ETH_RXD0_SOURCE 			GPIO_PinSource4

#define  ETH_RXD1_PORT_GROUP 		GPIOC
#define  ETH_RXD1_PIN 				GPIO_Pin_5
#define  ETH_RXD1_SOURCE 			GPIO_PinSource5

#define  ETH_CRSDV_PORT_GROUP 		GPIOA
#define  ETH_CRSDV_PIN 				GPIO_Pin_7
#define  ETH_CRSDV_SOURCE 			GPIO_PinSource7

#define  ETH_RCLK_PORT_GROUP 		GPIOA
#define  ETH_RCLK_PIN 				GPIO_Pin_1
#define  ETH_RCLK_SOURCE 			GPIO_PinSource1

#define  ETH_RESET_PORT_GROUP 		GPIOC
#define  ETH_RESET_PIN 				GPIO_Pin_0
#define  ETH_RESET_SOURCE 			GPIO_PinSource0


//END ETH PORT

//START DHT11 PORT

#define DHT11_DATA_GROUP	GPIOB
#define DHT11_DATA_PIN		GPIO_Pin_2
#define DHT11_DATA_PIN_SOURCE 	GPIO_PinSource2
#define DHT11_DATA_EXTI_PORT	EXTI_PortSourceGPIOB
#define DHT11_DATA_EXTI_LINE	EXTI_Line2


#define DHT11_OUT		gpio_cfg((uint32_t)DHT11_DATA_GROUP, DHT11_DATA_PIN, GPIO_Mode_Out_OD)
#define DHT11_IN		gpio_cfg((uint32_t)DHT11_DATA_GROUP, DHT11_DATA_PIN, GPIO_Mode_IPU)
#define DHT11_LOW		do{GPIO_CLR(DHT11_DATA_GROUP, DHT11_DATA_PIN); }while(0)
#define DHT11_HIGH		do{GPIO_SET(DHT11_DATA_GROUP, DHT11_DATA_PIN); }while(0)
#define DHT11_STAT		GPIO_STAT(DHT11_DATA_GROUP, DHT11_DATA_PIN)

//END DHT11 PORT

//START RELAY PORT

#define RELAY_GROUP		GPIOE
#define RELAY_PIN		GPIO_Pin_7

#define RELAY_LOW		do{GPIO_CLR(RELAY_GROUP, RELAY_PIN); }while(0)
#define RELAY_HIGH		do{GPIO_SET(RELAY_GROUP, RELAY_PIN); }while(0)

#define RELAY_OFF		RELAY_LOW
#define RELAY_ON		RELAY_HIGH


//END RELAY PORT

//START RFID PORT
#define RFID_CLK_GROUP		GPIOE
#define RFID_CLK_PIN		GPIO_Pin_14

#define RFID_MOSI_GROUP	GPIOE
#define RFID_MOSI_PIN		GPIO_Pin_13

#define RFID_MISO_GROUP	GPIOE
#define RFID_MISO_PIN		GPIO_Pin_12
#define RFID_RST_GROUP		GPIOE
#define RFID_RST_PIN		GPIO_Pin_10
#define RFID_SDA_GROUP		GPIOE		//SPI CS
#define RFID_SDA_PIN		GPIO_Pin_15
#define RFID_IRQ_GROUP		GPIOE
#define RFID_IRQ_PIN		GPIO_Pin_11
#define RFID_IRQ_EXTI_LINE	EXTI_Line11


#define RFID_RST_HIGH 		do{GPIO_SET(RFID_RST_GROUP, RFID_RST_PIN); delay_us(10);}while(0)
#define RFID_RST_LOW 	do{GPIO_CLR(RFID_RST_GROUP, RFID_RST_PIN);delay_us(10);}while(0)
#define RFID_CS_HIGH 		do{GPIO_SET(RFID_SDA_GROUP, RFID_SDA_PIN); delay_us(10);}while(0)
#define RFID_CS_LOW 	do{GPIO_CLR(RFID_SDA_GROUP, RFID_SDA_PIN);delay_us(10);}while(0)

//END RFID PORT

//START LCD PORT	//LCD引脚和RFID是共用的，不能同时使用
#define LCD_CLK_GROUP		GPIOE
#define LCD_CLK_PIN			GPIO_Pin_14

#define LCD_MOSI_GROUP		GPIOE
#define LCD_MOSI_PIN		GPIO_Pin_13

#define LCD_DC_GROUP		GPIOE
#define LCD_DC_PIN			GPIO_Pin_12
#define LCD_RST_GROUP		GPIOE
#define LCD_RST_PIN			GPIO_Pin_10
#define LCD_CS_GROUP		GPIOE		//SPI CS
#define LCD_CS_PIN			GPIO_Pin_15
#define LCD_LED_GROUP		GPIOE
#define LCD_LED_PIN			GPIO_Pin_11


#define LCD_RST_HIGH 		do{GPIO_SET(LCD_RST_GROUP, LCD_RST_PIN);}while(0)
#define LCD_RST_LOW 		do{GPIO_CLR(LCD_RST_GROUP, LCD_RST_PIN);}while(0)
#define LCD_CS_HIGH 			do{GPIO_SET(LCD_CS_GROUP, LCD_CS_PIN); }while(0)
#define LCD_CS_LOW 			do{GPIO_CLR(LCD_CS_GROUP, LCD_CS_PIN);}while(0)

#define LCD_LED_HIGH 		do{GPIO_SET(LCD_LED_GROUP, LCD_LED_PIN);}while(0)
#define LCD_LED_LOW 		do{GPIO_CLR(LCD_LED_GROUP, LCD_LED_PIN);}while(0)
#define LCD_DC_HIGH 			do{GPIO_SET(LCD_DC_GROUP, LCD_DC_PIN); }while(0)
#define LCD_DC_LOW 			do{GPIO_CLR(LCD_DC_GROUP, LCD_DC_PIN);}while(0)

#define LCD_CLK_HIGH 		do{GPIO_SET(LCD_CLK_GROUP, LCD_CLK_PIN);}while(0)
#define LCD_CLK_LOW 		do{GPIO_CLR(LCD_CLK_GROUP, LCD_CLK_PIN);}while(0)
#define LCD_MOSI_HIGH 		do{GPIO_SET(LCD_MOSI_GROUP, LCD_MOSI_PIN);}while(0)
#define LCD_MOSI_LOW 		do{GPIO_CLR(LCD_MOSI_GROUP, LCD_MOSI_PIN);}while(0)

#define LCD_BACKLIGHT_ON	LCD_LED_HIGH
#define LCD_BACKLIGHT_OFF	LCD_LED_LOW
//END LCD  PORT

#define MUTE_PORT_GROUP 		GPIOA
#define MUTE_PIN 			GPIO_Pin_12


#define MUTE_OFF 		do{GPIO_SET(MUTE_PORT_GROUP, MUTE_PIN); }while(0)                 //音频静音模式关闭
#define MUTE_ON 	    do{GPIO_CLR(MUTE_PORT_GROUP, MUTE_PIN);}while(0)                   //音频静音模式打开    低电平为静音模式


#define IR_IN_PORT_GROUP 		GPIOD		//注意IRIN 和 IO5是同一个IO口
#define IR_IN_PIN 				GPIO_Pin_14
#define IR_IN_PIN_SOURCE		GPIO_PinSource14
#define IR_IN_SOURCE_GROUP		EXTI_PortSourceGPIOD
#define IR_IN_LINE				EXTI_Line14

#define GPIO_SET(port,pin) (port->BSRRL = pin)
#define GPIO_CLR(port,pin) (port->BSRRH = pin)
#define GPIO_STAT(port,pin)   (!!(port->IDR & pin))



//以下为P10插座的IO口配置
//包含共8个引脚
#define IO1_GROUP	GPIOD

#define IO2_GROUP	GPIOD
#define IO3_GROUP	GPIOD
#define IO4_GROUP	GPIOD
#define IO5_GROUP	GPIOD
#define IO6_GROUP	GPIOD
#define IO7_GROUP	GPIOD
#define IO8_GROUP	GPIOD

#define IO1_PIN	GPIO_Pin_10
#define IO2_PIN	GPIO_Pin_11
#define IO3_PIN	GPIO_Pin_12
#define IO4_PIN	GPIO_Pin_13
#define IO5_PIN	GPIO_Pin_14
#define IO6_PIN	GPIO_Pin_15
#define IO7_PIN	GPIO_Pin_3
#define IO8_PIN	GPIO_Pin_4

// nrf2401 用的上面一组IO口
#define NRF_2401_CLK_GROUP		GPIOD
#define NRF_2401_MISO_GROUP	GPIOD
#define NRF_2401_MOSI_GROUP	GPIOD
#define NRF_2401_CS_GROUP		GPIOD
#define NRF_2401_CE_GROUP		GPIOD
#define NRF_2401_IRQ_GROUP		GPIOD

#define NRF_2401_CLK_PIN		GPIO_Pin_12
#define NRF_2401_MISO_PIN		GPIO_Pin_10
#define NRF_2401_MOSI_PIN		GPIO_Pin_13
#define NRF_2401_CS_PIN		GPIO_Pin_15
#define NRF_2401_CE_PIN		GPIO_Pin_14
#define NRF_2401_IRQ_PIN		GPIO_Pin_11


#define NRF_2401_CS_LOW	do{GPIO_CLR(NRF_2401_CS_GROUP, NRF_2401_CS_PIN);}while(0)
#define NRF_2401_CS_HIGH	do{GPIO_SET(NRF_2401_CS_GROUP, NRF_2401_CS_PIN);}while(0)

#define NRF_2401_CE_LOW	do{GPIO_CLR(NRF_2401_CE_GROUP, NRF_2401_CE_PIN);}while(0)
#define NRF_2401_CE_HIGH	do{GPIO_SET(NRF_2401_CE_GROUP, NRF_2401_CE_PIN);}while(0)

#define NRF24L01_IRQ 	GPIO_STAT(NRF_2401_IRQ_GROUP, NRF_2401_IRQ_PIN)

#define NRF_2401_SOURCE_GROUP		EXTI_PortSourceGPIOD
#define NRF_2401_LINE				EXTI_Line11
#define NRF_2401_SOURCE_PIN		GPIO_PinSource11

//LED

#define IND1_GROUP	    GPIOD
#define IND2_GROUP	    GPIOD
#define IND3_GROUP	    GPIOA


#define IND1_PIN	    GPIO_Pin_12    //PD12
#define IND2_PIN	    GPIO_Pin_13    //PD13
#define IND3_PIN	    GPIO_Pin_1     //PA1


#define IND1_ON 		do{GPIO_SET(IND1_GROUP, IND1_PIN); }while(0)
#define IND1_OFF 	   do{GPIO_CLR(IND1_GROUP, IND1_PIN);}while(0)

#define IND2_ON 		do{GPIO_SET(IND2_GROUP, IND2_PIN); }while(0)
#define IND2_OFF 	   do{GPIO_CLR(IND2_GROUP, IND2_PIN); }while(0)

#define IND3_ON 		do{GPIO_SET(IND3_GROUP, IND3_PIN); }while(0)
#define IND3_OFF 	    do{GPIO_CLR(IND3_GROUP, IND4_PIN);}while(0)

//#define IND1_SET IND1_ON
//#define IND1_CLR IND1_OFF

//#define IND2_SET IND2_ON
//#define IND2_CLR IND2_OFF

//#define IND4_SET IND4_ON
//#define IND4_CLR IND4_OFF


//#define BUTTON_PORT		   GPIOB
//#define BUTTON_PIN			GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13
//#define BUTTON_SOURCE		GPIO_PinSource7
//#define BUTTON_EXTI_LINE 	EXTI_Line7
#define BUTTON_PORT		       GPIOC        //按键组1    GPIOB
#define BUTTON_PORT_2		   GPIOB        //按键组2    GPIOC

#define BUTTON_PIN			    GPIO_Pin_4
#define BUTTON_PIN_2			GPIO_Pin_13
#define BUTTON_PIN_3			GPIO_Pin_5
#define BUTTON_PIN_4			GPIO_Pin_2

#define BUTTON_STAT 	GPIO_STAT(BUTTON_PORT, BUTTON_PIN)     //按键1     PC4
#define BUTTON_STAT_2 	GPIO_STAT(BUTTON_PORT, BUTTON_PIN_2)     //按键2     PC13
#define BUTTON_STAT_3 	GPIO_STAT(BUTTON_PORT, BUTTON_PIN_3)     //按键3     PC15
#define BUTTON_STAT_4 	GPIO_STAT(BUTTON_PORT_2, BUTTON_PIN_4)     //按键4     PB2







void gpio_cfg(uint32_t group, uint32_t pin, uint8_t mode);
void usr_gpio_init(void);
int check_rst_stat(void);
void driver_misc_init(void);
uint32_t get_random(void);
void driver_gpio_init(void);
void _mem_init(void);
uint32_t get_us_count(void);
#endif

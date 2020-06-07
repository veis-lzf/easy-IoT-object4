/**
  ******************************************************************************
  * @file    ADC/ADC_TriggerMode/Src/stm32f4xx_it.c 
  * @author  MCD Application Team
  * @version V1.2.1
  * @date    13-March-2015
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
//#define DEBUG
#include "debug.h"
#include "drivers.h"
#include "stdio.h"
#include "api.h"
#include "stm32f4xx_it.h"
   

/** @addtogroup STM32F4xx_HAL_Examples
  * @{
  */

/** @addtogroup ADC_TriggerMode
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

void hard_fault_handler_c(unsigned int * hardfault_args)
{
	unsigned int stacked_r0;
	unsigned int stacked_r1;
	unsigned int stacked_r2;
	unsigned int stacked_r3;
	unsigned int stacked_r12;
	unsigned int stacked_lr;
	unsigned int stacked_pc;
	unsigned int stacked_psr;
	stacked_r0 = ((unsigned long) hardfault_args[0]);
	stacked_r1 = ((unsigned long) hardfault_args[1]);
	stacked_r2 = ((unsigned long) hardfault_args[2]);
	stacked_r3 = ((unsigned long) hardfault_args[3]);
	stacked_r12 = ((unsigned long) hardfault_args[4]);
	stacked_lr = ((unsigned long) hardfault_args[5]);
	stacked_pc = ((unsigned long) hardfault_args[6]);
	stacked_psr = ((unsigned long) hardfault_args[7]);
	printf ("[Hard fault handler]\n");
	printf ("R0 = %08x\n", stacked_r0);
	printf ("R1 = %08x\n", stacked_r1);
	printf ("R2 = %08x\n", stacked_r2);
	printf ("R3 = %08x\n", stacked_r3);
	printf ("R12 = %08x\n", stacked_r12);
	printf ("LR = %08x\n", stacked_lr);
	printf ("PC = %08x\n", stacked_pc);
	printf ("PSR = %08x\n", stacked_psr);
	printf ("MMAR = %08x\n", (*((volatile unsigned long *)(0xE000ED34))));
	printf ("BFAR = %08x\n", (*((volatile unsigned long *)(0xE000ED38))));
	printf ("CFSR = %08x, MFSR:%04x,BFSR:%02x,UFSR:%02x\n", (*((volatile unsigned long *)(0xE000ED28))),
		(*((volatile unsigned char *)(0xE000ED28))),
		(*((volatile unsigned char *)(0xE000ED29))),
		(*((volatile unsigned short *)(0xE000ED2a))));
	
	printf ("HFSR = %08x\n", (*((volatile unsigned long *)(0xE000ED2C))));
	printf ("DFSR = %08x\n", (*((volatile unsigned long *)(0xE000ED30))));
	printf ("AFSR = %08x\n", (*((volatile unsigned long *)(0xE000ED3C))));
	while(1);
}
/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
__asm void HardFault_Handler(void)
{
  IMPORT hard_fault_handler_c
	TST LR,#4
	ITE EQ
	MRSEQ R0,MSP
	MRSNE R0,PSP
	B hard_fault_handler_c
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
//void SVC_Handler(void)
//{
//}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
//void PendSV_Handler(void)
//{
//}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
//void SysTick_Handler(void)
//{
//}


/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (ADC), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles ADC interrupt request.
  * @param  None
  * @retval None
  */
//void ADC_IRQHandler(void)
//{
//  HAL_ADC_IRQHandler(&AdcHandle);
//}

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/


extern void __EXTI9_IRQHandler(void);
extern void __DHT11_IRQHandler(void);
void EXTI9_5_IRQHandler(void)
{
	/*
	if(EXTI_GetITStatus(RFID_IRQ_EXTI_LINE) == SET)
	{
		uint8_t n;
		EXTI_ClearITPendingBit(RFID_IRQ_EXTI_LINE);
		
		n = ReadRawRC(ComIrqReg);
		p_dbg("RFID IRQ %x", n);
	}*/
	/*
	if(EXTI_GetITStatus(EXTI_Line9) == SET)
	{
		EXTI_ClearITPendingBit(EXTI_Line9);
		__EXTI9_IRQHandler();
	}*/

	/*
#if HW_VER >= (3)
	if(EXTI_GetITStatus(EXTI_Line6) == SET)
	{
		EXTI_ClearITPendingBit(EXTI_Line6);
		__DHT11_IRQHandler();
	}
#endif
      */
}

//extern void __EXTI14_IRQHandler(void);
extern void __EXTI11_IRQHandler(void);
void EXTI15_10_IRQHandler(void)
{

//	if(EXTI_GetITStatus(EXTI_Line14) == SET)
//	{
//		EXTI_ClearITPendingBit(EXTI_Line14);
//		__EXTI14_IRQHandler();
//	}

	if(EXTI_GetITStatus(EXTI_Line11) == SET)
	{
		EXTI_ClearITPendingBit(EXTI_Line11);
		__EXTI11_IRQHandler();
	}	
}

void EXTI2_IRQHandler(void)
{

	if(EXTI_GetITStatus(EXTI_Line2) == SET)
	{
		EXTI_ClearITPendingBit(EXTI_Line2);
		__DHT11_IRQHandler();
	}

}
/**
  * @}
  */ 

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

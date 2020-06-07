/*
*********************************************************************************************************
*                                             uC/GUI V3.98
*                        Universal graphic software for embedded applications
*
*                       (c) Copyright 2002, Micrium Inc., Weston, FL
*                       (c) Copyright 2002, SEGGER Microcontroller Systeme GmbH
*
*              礐/GUI is protected by international copyright laws. Knowledge of the
*              source code may not be used to write a similar product. This file may
*              only be used in accordance with a license and should not be redistributed
*              in any way. We appreciate your understanding and fairness.
*
---Author-Explanation
* 
* 1.00.00 020519 JJL    First release of uC/GUI to uC/OS-II interface
* 
*
* Known problems or limitations with current version
*
*    None.
*
*
* Open issues
*
*    None
*********************************************************************************************************
*/

#define DEBUG

#include "drivers.h"
#include "app.h"
#include "api.h"

/*
*********************************************************************************************************
*                                         GLOBAL VARIABLES
*********************************************************************************************************
*/

static  mutex_t gui_mutex;
static  wait_event_t gui_wait;

static  wait_event_t  key_wait;
static  int        KeyPressed;
static  char       KeyIsInited = 0;


/*
*********************************************************************************************************
*                                        TIMING FUNCTIONS
*
* Notes: Some timing dependent routines of uC/GUI require a GetTime and delay funtion. 
*        Default time unit (tick), normally is 1 ms.
*********************************************************************************************************
*/

int  GUI_X_GetTime (void) 
{
    return os_time_get();
}


void  GUI_X_Delay (int period) 
{
    sleep(period);
}


/*
*********************************************************************************************************
*                                          GUI_X_ExecIdle()
*********************************************************************************************************
*/
void GUI_X_ExecIdle (void) 
{
    sleep(100);
}


/*
*********************************************************************************************************
*                                    MULTITASKING INTERFACE FUNCTIONS
*
* Note(1): 1) The following routines are required only if uC/GUI is used in a true multi task environment, 
*             which means you have more than one thread using the uC/GUI API.  In this case the #define 
*             GUI_OS 1   needs to be in GUIConf.h
*********************************************************************************************************
*/

void  GUI_X_InitOS (void)
{ 
    gui_mutex = mutex_init("gui");
    gui_wait = init_event();
}


void  GUI_X_Lock (void)
{ 
    mutex_lock(gui_mutex);
}


void  GUI_X_Unlock (void)
{ 
    mutex_unlock(gui_mutex);
}


uint32_t  GUI_X_GetTaskId (void) 
{ 
  return thread_myself();
}

/*
*********************************************************************************************************
*                                        GUI_X_WaitEvent()
*                                        GUI_X_SignalEvent()
*********************************************************************************************************
*/


void GUI_X_WaitEvent (void) 
{
    wait_event(gui_wait);
}


void GUI_X_SignalEvent (void) 
{
    wake_up(gui_wait);
}


/*被GUI_Init()调用,用来初始化一些GUI运行之前需要用的硬件,如键盘或者鼠标之类的.如果不需要的话,可以为空*/
void GUI_X_Init (void) 
{
    key_wait = init_event();
}
/*
*********************************************************************************************************
*                                      KEYBOARD INTERFACE FUNCTIONS
*
* Purpose: The keyboard routines are required only by some widgets.
*          If widgets are not used, they may be eliminated.
*
* Note(s): If uC/OS-II is used, characters typed into the log window will be placed	in the keyboard buffer. 
*          This is a neat feature which allows you to operate your target system without having to use or 
*          even to have a keyboard connected to it. (useful for demos !)
*********************************************************************************************************
*/

static  void  CheckInit (void) 
{
    if (KeyIsInited == 0) {
        KeyIsInited = 1;
        GUI_X_Init();
    }
}

int  GUI_X_GetKey (void) 
{
    int r;
    r = KeyPressed;
    CheckInit();
    KeyPressed = 0;
    return (r);
}


int  GUI_X_WaitKey (void) 
{
    int    r;
    CheckInit();
    
    if (KeyPressed == 0) {
        wait_event(key_wait);
    }
    r          = KeyPressed;
    KeyPressed = 0;
    return (r);
}


void  GUI_X_StoreKey (int k) 
{
    KeyPressed = k;
    wake_up(key_wait);
}
	 	 			 		    	 				 	  			   	 	 	 	 	 	  	  	      	   		 	 	 		  		  	 		 	  	  			     			       	   	 			  		    	 	     	 				  	 					 	 			   	  	  			 				 		 	 	 			     			 
void GUI_X_Log(const char *s) 
{ 
	p_dbg("gui dbg:%s", s); 
}

void GUI_X_Warn(const char *s) 
{
	p_dbg("gui warn:%s", s); 
}

void GUI_X_ErrorOut(const char *s)
{ 
	p_err("gui err:%s", s); 
}

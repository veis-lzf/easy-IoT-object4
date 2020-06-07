/*
    this file aims to port EDP to  LwIP1.4.1
    what is EDP:  From OneNet, M2M package solution

    author: Forrest
    create date: 2015-01
*/

#ifndef __COMMON_H__
#define __COMMON_H__

//data type from LwIP
#include "lwip/arch.h"
/*------------------------------------------------------*/
/* Type Definition Macros, port types from LwIP*/
/*------------------------------------------------------*/
    typedef u8_t  uint8;
    typedef s8_t  int8;
    typedef u16_t uint16;
    typedef s16_t int16;
    typedef u32_t uint32;
    typedef s32_t int32;

#endif /* __COMMON_H__ */

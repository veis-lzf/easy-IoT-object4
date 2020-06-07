#ifndef	 __W25P80_H
#define	__W25P80_H
#include "type.h"
#include "api.h"


#define FLASH_ROOM_SIZE				(16*1024*1024)
#define FIRMWARE_BASE_ADDR			(512*1024)

//#define ssize_t int

int  m25p80_init(void);
int m25p80_write(u32 to, size_t len,const u_char *buf);
int m25p80_read(u32 from, size_t len, u_char *buf);
int m25p80_erase(u32 addr, int len);
uint32_t m25p80_read_id(void);
int read_firmware(u32 offset, u32 len, u8 * pbuf);


#endif


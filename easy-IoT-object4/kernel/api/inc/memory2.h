#ifndef __MEMORY2_H__
#define __MEMORY2_H__

void *mem_malloc2( size_t xWantedSize );
void mem_free2( void *pv );
void sys_meminit2(void *pool, unsigned int size);
uint32_t mem_get_size2(void);
uint32_t mem_get_free2(void);
void *mem_calloc2(unsigned int count, unsigned int size);
void *mem_realloc2(void *ptr, size_t size);

#endif

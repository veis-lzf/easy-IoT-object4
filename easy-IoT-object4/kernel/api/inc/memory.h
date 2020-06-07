#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "lwip/opt.h"

#define mem_init()

#if MEM_MAN_MODE == 0 //#if MEM_USE_POOLS

#define USE_MEM_DEBUG			0
typedef unsigned int mem_size_t;
typedef unsigned int mem_ptr_t;

void *mem_malloc(mem_size_t size);
void *mem_calloc(mem_size_t count, mem_size_t size);

void mem_slide_check(int show_list);
void mem_free(void *rmem);
uint32_t get_mem_size(void *rmem);
#define mem_trim(mem, size) (mem)

#elif MEM_MAN_MODE == 1

#define USE_MEM_DEBUG			1
#define LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT	0	
#define MEM_ALIGNMENT     		4
#define USE_MEM_ASSERT			0

/* MEM_SIZE would have to be aligned, but using 64000 here instead of
 * 65535 leaves some room for alignment...
 */

typedef unsigned int mem_size_t;
typedef unsigned int mem_ptr_t;

void sys_meminit(void *pool, unsigned int size);

#if USE_MEM_DEBUG

void *mem_malloc_ex(const char *name, mem_size_t size);
void *mem_calloc_ex(const char *name, mem_size_t count, mem_size_t size);

#define mem_malloc(SIZE) mem_malloc_ex(__FUNCTION__, SIZE)
#define mem_calloc(SIZE, COUNT) mem_calloc_ex(__FUNCTION__, (SIZE), (COUNT))

#else

void *mem_malloc(mem_size_t size);
void *mem_calloc(mem_size_t count, mem_size_t size);
#endif

void mem_slide_check(int show_list);

void mem_free(void *rmem);
uint32_t get_mem_size(void *rmem);
void *mem_trim(void *mem, mem_size_t size);
#elif MEM_MAN_MODE == 2

#define USE_MEM_DEBUG			0
#define mem_trim(mem, size) (mem)

typedef unsigned int mem_size_t;
typedef unsigned int mem_ptr_t;

void *mem_malloc(mem_size_t size);
void mem_free(void *rmem);

__inline void *mem_calloc(mem_size_t count, mem_size_t size)
{
	void *p;

	/* allocate 'count' objects of size 'size' */
	p = mem_malloc(count *size);
	if (p)
	{
		/* zero the memory */
		memset(p, 0, count *size);
	}
	return p;
}

void mem_slide_check(int show_list);
void sys_meminit(void *pool, unsigned int size);

#endif

#define malloc 		mem_malloc
#define free 			mem_free

void *mem_realloc(void *ptr, size_t size);
	
uint32_t mem_get_free(void);
uint32_t mem_get_size(void);
#endif /*__LWIP_MEM_H__*/

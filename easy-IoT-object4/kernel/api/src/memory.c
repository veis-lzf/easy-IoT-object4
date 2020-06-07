#define DEBUG

#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/sys.h"
#include "lwip/stats.h"
#include "lwip/err.h"
#include "lwip/debug.h"
#include "debug.h"
#include "api.h"
#include "drivers.h"

#if USE_MEM_DEBUG
#define MEM_CRITICAL_FLAG 		0x12345678
#define MEM_CRITICAL_AREA_SIZE	4
#else
#define MEM_CRITICAL_AREA_SIZE	0
#endif




#include <string.h>

#if MEM_MAN_MODE == 0

typedef unsigned int mem_size_t;
typedef unsigned int mem_ptr_t;
/* lwIP head implemented with different sized pools */

void sys_meminit(void *pool, unsigned int size)
{
	p_err("left %d memory not used", size);
	memp_init();
}


/**
 * Allocate memory: determine the smallest pool that is big enough
 * to contain an element of 'size' and get an element from that pool.
 *
 * @param size the size in bytes of the memory needed
 * @return a pointer to the allocated memory or NULL if the pool is empty
 */
void *mem_malloc(mem_size_t size)
{
  void *ret;
  struct memp_malloc_helper *element;
  memp_t poolnr;
  mem_size_t required_size = size + LWIP_MEM_ALIGN_SIZE(sizeof(struct memp_malloc_helper));

  for (poolnr = MEMP_POOL_FIRST; poolnr <= MEMP_POOL_LAST; poolnr = (memp_t)(poolnr + 1)) {
#if MEM_USE_POOLS_TRY_BIGGER_POOL
again:
#endif /* MEM_USE_POOLS_TRY_BIGGER_POOL */
    /* is this pool big enough to hold an element of the required size
       plus a struct memp_malloc_helper that saves the pool this element came from? */
    if (required_size <= memp_sizes[poolnr]) {
      break;
    }
  }
  if (poolnr > MEMP_POOL_LAST) {
    LWIP_ASSERT("mem_malloc(): no pool is that big!", 0);
    return NULL;
  }
  element = (struct memp_malloc_helper*)memp_malloc(poolnr);
  if (element == NULL) {
    /* No need to DEBUGF or ASSERT: This error is already
       taken care of in memp.c */
#if MEM_USE_POOLS_TRY_BIGGER_POOL
    /** Try a bigger pool if this one is empty! */
    if (poolnr < MEMP_POOL_LAST) {
      poolnr++;
      goto again;
    }
#endif /* MEM_USE_POOLS_TRY_BIGGER_POOL */
    return NULL;
  }

  /* save the pool number this element came from */
  element->poolnr = poolnr;
  /* and return a pointer to the memory directly after the struct memp_malloc_helper */
  ret = (u8_t*)element + LWIP_MEM_ALIGN_SIZE(sizeof(struct memp_malloc_helper));

  return ret;
}

/**
 * Free memory previously allocated by mem_malloc. Loads the pool number
 * and calls memp_free with that pool number to put the element back into
 * its pool
 *
 * @param rmem the memory element to free
 */
void
mem_free(void *rmem)
{
  struct memp_malloc_helper *hmem;

  LWIP_ASSERT("rmem != NULL", (rmem != NULL));
  LWIP_ASSERT("rmem == MEM_ALIGN(rmem)", (rmem == LWIP_MEM_ALIGN(rmem)));

  /* get the original struct memp_malloc_helper */
  hmem = (struct memp_malloc_helper*)(void*)((u8_t*)rmem - LWIP_MEM_ALIGN_SIZE(sizeof(struct memp_malloc_helper)));

  LWIP_ASSERT("hmem != NULL", (hmem != NULL));
  LWIP_ASSERT("hmem == MEM_ALIGN(hmem)", (hmem == LWIP_MEM_ALIGN(hmem)));
  LWIP_ASSERT("hmem->poolnr < MEMP_MAX", (hmem->poolnr < MEMP_MAX));

  /* and put it in the pool we saved earlier */
  memp_free(hmem->poolnr, hmem);
}

void *mem_calloc(mem_size_t count, mem_size_t size)
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

void mem_slide_check(int show_list)
{
	
}

uint32_t mem_get_size()
{
	return 0;
}

uint32_t mem_get_free()
{
	return 0;
}

#elif MEM_MAN_MODE == 1

#if LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT
#define USE_MUTEX_LOCK	1
#else
#define USE_MUTEX_LOCK	0
#endif

DECLARE_MONITOR_ITEM("mem used", mem_used);
DECLARE_MONITOR_ITEM("mem max", mem_max);
DECLARE_MONITOR_ITEM("mem err", mem_err);
DECLARE_MONITOR_ITEM("mem block", mem_block);

const char not_used_name[] = "not used";

//#define LWIP_MEM_ALIGN_SIZE(size) (((size) + MEM_ALIGNMENT - 1) & ~(MEM_ALIGNMENT - 1))
//#define LWIP_MEM_ALIGN(addr) ((void *)(((mem_ptr_t)(addr) + MEM_ALIGNMENT - 1)&~(mem_ptr_t)(MEM_ALIGNMENT - 1)))

/**
 * The heap is made up as a list of structs of this type.
 * This does not have to be aligned since for getting its size,
 * we only use the macro SIZEOF_STRUCT_MEM, which automatically alignes.
 */
struct mem
{
	/** index (-> ram[next]) of the next struct */
	mem_size_t next;
	/** index (-> ram[prev]) of the previous struct */
	mem_size_t prev;
	/** 1: this area is used; 0: this area is unused */
	char used;
	#ifdef USE_MEM_DEBUG
	const char *name;
	#endif
};

/** All allocated blocks will be MIN_SIZE bytes big, at least!
 * MIN_SIZE can be overridden to suit your needs. Smaller values save space,
 * larger values could prevent too small blocks to fragment the RAM too much. */
#ifndef MIN_SIZE
#define MIN_SIZE             12
#endif /* MIN_SIZE */
/* some alignment macros: we define them here for better source code layout */
#define MIN_SIZE_ALIGNED     LWIP_MEM_ALIGN_SIZE(MIN_SIZE)
#define SIZEOF_STRUCT_MEM    LWIP_MEM_ALIGN_SIZE(sizeof(struct mem))
#define MEM_SIZE_ALIGNED     LWIP_MEM_ALIGN_SIZE(ram_size)


/** the heap. we need one struct mem at the end and some room for alignment */
//unsigned char ram_heap[MEM_SIZE_ALIGNED + (2*SIZEOF_STRUCT_MEM) + MEM_ALIGNMENT + MEM_CRITICAL_AREA_SIZE];
//#define LWIP_RAM_HEAP_POINTER ram_heap

/** pointer to the heap (ram_heap): for alignment, ram is now a pointer instead of an array */
static unsigned char *ram;
static unsigned int ram_size;
static unsigned int ram_totol, ram_used;
static unsigned int malloc_cnt = 0;

/** the last entry, always unused! */
static struct mem *ram_end;
/** pointer to the lowest free block, this is used for faster search */
static struct mem *lfree;

/** concurrent access protection */

#if LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT

static volatile unsigned char mem_free_count;
#if USE_MUTEX_LOCK
static mutex_t mem_mutex;
#endif
/* Allow mem_free from other (e.g. interrupt) context */
#if USE_MUTEX_LOCK
#define LWIP_MEM_FREE_DECL_PROTECT()
#define LWIP_MEM_FREE_PROTECT()       mutex_lock(mem_mutex);
#define LWIP_MEM_FREE_UNPROTECT()     mutex_unlock(mem_mutex);
#define LWIP_MEM_ALLOC_DECL_PROTECT()
#define LWIP_MEM_ALLOC_PROTECT()     mutex_lock(mem_mutex);
#define LWIP_MEM_ALLOC_UNPROTECT()    mutex_unlock(mem_mutex);
#else

#define LWIP_MEM_FREE_DECL_PROTECT()
#define LWIP_MEM_FREE_PROTECT()       enter_critical();
#define LWIP_MEM_FREE_UNPROTECT()     exit_critical();
#define LWIP_MEM_ALLOC_DECL_PROTECT()
#define LWIP_MEM_ALLOC_PROTECT()     enter_critical();
#define LWIP_MEM_ALLOC_UNPROTECT()    exit_critical();
#endif

#else /* LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT */

#define LWIP_MEM_FREE_DECL_PROTECT()  uint32_t irq_flag;
#define LWIP_MEM_FREE_PROTECT()       irq_flag = local_irq_save();
#define LWIP_MEM_FREE_UNPROTECT()     local_irq_restore(irq_flag);
#define LWIP_MEM_ALLOC_DECL_PROTECT() uint32_t irq_flag;
#define LWIP_MEM_ALLOC_PROTECT()      irq_flag = local_irq_save();
#define LWIP_MEM_ALLOC_UNPROTECT()    local_irq_restore(irq_flag);

#endif /* LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT */


/**
 * "Plug holes" by combining adjacent empty struct mems.
 * After this function is through, there should not exist
 * one empty struct mem pointing to another empty struct mem.
 *
 * @param mem this points to a struct mem which just has been freed
 * @internal this function is only called by mem_free() and mem_trim()
 *
 * This assumes access to the heap is protected by the calling function
 * already.
 */
static void plug_holes(struct mem *mem)
{
	struct mem *nmem;
	struct mem *pmem;

	#if USE_MEM_ASSERT
	LWIP_ASSERT("plug_holes: mem >= ram", (unsigned char*)mem >= ram);
	LWIP_ASSERT("plug_holes: mem < ram_end", (unsigned char*)mem < (unsigned char*)ram_end);
	LWIP_ASSERT("plug_holes: mem->used == 0", mem->used == 0);

	/* plug hole forward */
	LWIP_ASSERT("plug_holes: mem->next <= MEM_SIZE_ALIGNED", mem->next <= MEM_SIZE_ALIGNED);
	#else
	if (!(((unsigned char*)mem >= ram) && ((unsigned char*)mem < (unsigned char*)ram_end) && (mem->used == 0) && (mem->next <= MEM_SIZE_ALIGNED)))
	{
#if USE_MEM_DEBUG	
		mem_slide_check(FALSE);
#endif
		p_err_fun;
		while (1)
			;
	}

	#endif
	nmem = (struct mem*)(void*) &ram[mem->next];
	if (mem != nmem && nmem->used == 0 && (unsigned char*)nmem != (unsigned char*)ram_end)
	{
		/* if mem->next is unused and not end of ram, combine mem and mem->next */
		if (lfree == nmem)
		{
			lfree = mem;
		} mem->next = nmem->next;
		((struct mem*)(void*) &ram[nmem->next])->prev = (mem_size_t)((unsigned char*)mem - ram);
	}

	/* plug hole backward */
	pmem = (struct mem*)(void*) &ram[mem->prev];
	if (pmem != mem && pmem->used == 0)
	{
		/* if mem->prev is unused, combine mem and mem->prev */
		if (lfree == mem)
		{
			lfree = pmem;
		} pmem->next = mem->next;
		((struct mem*)(void*) &ram[mem->next])->prev = (mem_size_t)((unsigned char*)pmem - ram);
	}
}

/**
 * Zero the heap and initialize start, end and lowest-free
 */
void sys_meminit(void *pool, unsigned int size)
{
	struct mem *mem;
    #if USE_MEM_ASSERT
	LWIP_ASSERT("Sanity check alignment", (SIZEOF_STRUCT_MEM &(MEM_ALIGNMENT - 1)) == 0);
    #endif
	/* align the heap */
	//ram = (unsigned char *)LWIP_MEM_ALIGN(LWIP_RAM_HEAP_POINTER);
	ram = (unsigned char*)LWIP_MEM_ALIGN(pool);
	ram_size = (size &~(MEM_ALIGNMENT - 1)) - ((2 *SIZEOF_STRUCT_MEM) + MEM_ALIGNMENT + MEM_CRITICAL_AREA_SIZE);
	ram_totol = ram_size;
	ADD_MONITOR_ITEM_VALUE(mem_max, ram_size);
	/* initialize the start of the heap */
	mem = (struct mem*)ram;
	mem->next = MEM_SIZE_ALIGNED;

	mem->prev = 0;
	mem->used = 0;
	#if USE_MEM_DEBUG
	mem->name = not_used_name;
	#endif

	/* initialize the end of the heap */
	ram_end = (struct mem*) &ram[MEM_SIZE_ALIGNED];
	ram_end->used = 1;
	ram_end->next = MEM_SIZE_ALIGNED;
	ram_end->prev = MEM_SIZE_ALIGNED;
	#if USE_MEM_DEBUG
	ram_end->name = "mem_end";
	#endif
	/* initialize the lowest-free pointer to the start of the heap */
	lfree = (struct mem*)(void*)ram;
	#if USE_MUTEX_LOCK
	mem_mutex = mutex_init(__FUNCTION__);
	#endif
}

/**
 * Put a struct mem back on the heap
 *
 * @param rmem is the data portion of a struct mem as returned by a previous
 *             call to mem_malloc()
 */
void mem_free(void *rmem)
{
	uint32_t size = 0;
	struct mem *mem;
	LWIP_MEM_FREE_DECL_PROTECT();

	if (rmem == NULL)
	{
		p_dbg("mem_free(p == NULL) was called.\n");

		return/* size*/;
	}
	#if USE_MEM_ASSERT
	LWIP_ASSERT("mem_free: sanity check alignment", (((mem_ptr_t)rmem) &(MEM_ALIGNMENT - 1)) == 0);

	LWIP_ASSERT("mem_free: legal memory", (unsigned char*)rmem >= (unsigned char*)ram && (unsigned char*)rmem < (unsigned char*)ram_end);
	#else
	if (!(((((mem_ptr_t)rmem) &(MEM_ALIGNMENT - 1)) == 0) && ((unsigned char*)rmem >= (unsigned char*)ram) && ((unsigned char*)rmem < (unsigned char*)ram_end)))
	{
#if USE_MEM_DEBUG	
		mem_slide_check(FALSE);
#endif
		p_err_fun;
		while (1)
			;
	}

	#endif
	if ((unsigned char*)rmem < (unsigned char*)ram || (unsigned char*)rmem >= (unsigned char*)ram_end)
	{

		p_dbg("mem_free: illegal memory\n");
		/* protect mem stats from concurrent access */
		//MEM_STATS_INC(illegal);

		return/* size*/;
	}
	/* protect the heap from concurrent access */
	LWIP_MEM_FREE_PROTECT();
	/* Get the corresponding struct mem ... */
	mem = (struct mem*)((unsigned char*)rmem - SIZEOF_STRUCT_MEM);
	/* ... which has to be in a used state ... */
	#if USE_MEM_ASSERT
	LWIP_ASSERT("mem_free: mem->used", mem->used);
	#else
	if (mem->used != 1)
	{
		LWIP_MEM_FREE_UNPROTECT();
#if USE_MEM_DEBUG	
		mem_slide_check(FALSE);
#endif
		p_err("mem_free err:%x", mem->used);
		return/* size*/;
		//while(1);
	}
	#endif
	/* ... and is now unused. */
	mem->used = 0;
	#if USE_MEM_DEBUG
	mem->name = not_used_name;
	#endif

	if (mem < lfree)
	{
		/* the newly freed struct is now the lowest */
		lfree = mem;
	}

	//MEM_STATS_DEC_USED(used, mem->next - (mem_size_t)(((unsigned char *)mem - ram)));
	size = mem->next - (mem_size_t)(((unsigned char*)mem - ram));
	SUB_MONITOR_ITEM_VALUE(mem_used, size);
	ram_used -= size;
	/* finally, see if prev or next are free also */
	plug_holes(mem);
	#if LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT
	mem_free_count = 1;
	#endif /* LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT */
	LWIP_MEM_FREE_UNPROTECT();

//	return size - SIZEOF_STRUCT_MEM - MEM_CRITICAL_AREA_SIZE;
}


uint32_t get_mem_size(void *rmem)
{
	uint32_t size = 0;
	struct mem *mem;

	if (rmem == NULL)
	{
		return size;
	}
	
	mem = (struct mem*)((unsigned char*)rmem - SIZEOF_STRUCT_MEM);

	if (mem->used != 1)
	{
		return size;
	}
	size = mem->next - (mem_size_t)(((unsigned char*)mem - ram));

	return size - SIZEOF_STRUCT_MEM - MEM_CRITICAL_AREA_SIZE;
}

#if USE_MEM_DEBUG
void mem_check(void *rmem)
{
	struct mem *mem;
	LWIP_MEM_FREE_DECL_PROTECT();

	if (!(((((mem_ptr_t)rmem) &(MEM_ALIGNMENT - 1)) == 0) && ((unsigned char*)rmem >= (unsigned char*)ram) && ((unsigned char*)rmem < (unsigned char*)ram_end)))
	{
		mem_slide_check(FALSE);
		p_err_fun;
		while (1)
			;
	}

	/* protect the heap from concurrent access */
	LWIP_MEM_FREE_PROTECT();
	/* Get the corresponding struct mem ... */
	mem = (struct mem*)(void*)((unsigned char*)rmem - SIZEOF_STRUCT_MEM);
	/* ... which has to be in a used state ... */

	if (mem->used != 1 && mem->used != 0)
	{
		mem_slide_check(FALSE);
		//p_err_fun;
		//return;
		while (1)
			p_err("mem_check stop");
	}

	LWIP_MEM_FREE_UNPROTECT();
}

#endif
/**
 * Shrink memory returned by mem_malloc().
 *
 * @param rmem pointer to memory allocated by mem_malloc the is to be shrinked
 * @param newsize required size after shrinking (needs to be smaller than or
 *                equal to the previous size)
 * @return for compatibility reasons: is always == rmem, at the moment
 *         or NULL if newsize is > old size, in which case rmem is NOT touched
 *         or freed!
 */
void *mem_trim(void *rmem, mem_size_t newsize)
{
	mem_size_t size;
	mem_size_t ptr, ptr2;
	struct mem *mem,  *mem2;
	/* use the FREE_PROTECT here: it protects with sem OR SYS_ARCH_PROTECT */
	LWIP_MEM_FREE_DECL_PROTECT();

	/* Expand the size of the allocated memory region so that we can
	adjust for alignment. */
	newsize = LWIP_MEM_ALIGN_SIZE(newsize);

	if (newsize < MIN_SIZE_ALIGNED)
	{
		/* every data block must be at least MIN_SIZE_ALIGNED long */
		newsize = MIN_SIZE_ALIGNED;
	}

	if (newsize > MEM_SIZE_ALIGNED)
	{
		return NULL;
	}
	#if USE_MEM_ASSERT
	LWIP_ASSERT("mem_trim: legal memory", (unsigned char*)rmem >= (unsigned char*)ram && (unsigned char*)rmem < (unsigned char*)ram_end);
	#else
	if (!(((unsigned char*)rmem >= (unsigned char*)ram) && ((unsigned char*)rmem < (unsigned char*)ram_end)))
	{
#if USE_MEM_DEBUG	
		mem_slide_check(FALSE);
#endif
		p_err_fun;
		while (1)
			;
	}
	#endif
	if ((unsigned char*)rmem < (unsigned char*)ram || (unsigned char*)rmem >= (unsigned char*)ram_end)
	{

		p_dbg("mem_trim: illegal memory\n");
		/* protect mem stats from concurrent access */

		//    MEM_STATS_INC(illegal);
		INC_MONITOR_ITEM_VALUE(mem_err);
		return rmem;
	}
	/* Get the corresponding struct mem ... */
	mem = (struct mem*)((unsigned char*)rmem - SIZEOF_STRUCT_MEM);
	/* ... and its offset pointer */
	ptr = (mem_size_t)((unsigned char*)mem - ram);

	size = mem->next - ptr - SIZEOF_STRUCT_MEM;
	#if USE_MEM_ASSERT
	LWIP_ASSERT("mem_trim can only shrink memory", newsize <= size);
	#else
	if (!(newsize <= size))
	{
#if USE_MEM_DEBUG	
		mem_slide_check(FALSE);
#endif
		p_err_fun;
		while (1)
			;
	}
	#endif
	if (newsize > size)
	{
		/* not supported */
		return NULL;
	}
	if (newsize == size)
	{
		/* No change in size, simply return */
		return rmem;
	}

	/* protect the heap from concurrent access */
	LWIP_MEM_FREE_PROTECT();

	mem2 = (struct mem*) &ram[mem->next];
	if (mem2->used == 0)
	{
		/* The next struct is unused, we can simply move it at little */
		mem_size_t next;
		/* remember the old next pointer */
		next = mem2->next;
		/* create new struct mem which is moved directly after the shrinked mem */
		ptr2 = ptr + SIZEOF_STRUCT_MEM + newsize;
		if (lfree == mem2)
		{
			lfree = (struct mem*) &ram[ptr2];
		} mem2 = (struct mem*) &ram[ptr2];
		mem2->used = 0;
		#if USE_MEM_DEBUG
		mem2->name = not_used_name;
		#endif
		/* restore the next pointer */
		mem2->next = next;
		/* link it back to mem */
		mem2->prev = ptr;
		/* link mem to it */
		mem->next = ptr2;
		/* last thing to restore linked list: as we have moved mem2,
		 * let 'mem2->next->prev' point to mem2 again. but only if mem2->next is not
		 * the end of the heap */
		if (mem2->next != MEM_SIZE_ALIGNED)
		{
			((struct mem*) &ram[mem2->next])->prev = ptr2;
		}
		//MEM_STATS_DEC_USED(used, (size - newsize));
		SUB_MONITOR_ITEM_VALUE(mem_used, (size - newsize));
		ram_used -= size - newsize;
		/* no need to plug holes, we've already done that */
	}
	else if (newsize + SIZEOF_STRUCT_MEM + MIN_SIZE_ALIGNED <= size)
	{
		/* Next struct is used but there's room for another struct mem with
		 * at least MIN_SIZE_ALIGNED of data.
		 * Old size ('size') must be big enough to contain at least 'newsize' plus a struct mem
		 * ('SIZEOF_STRUCT_MEM') with some data ('MIN_SIZE_ALIGNED').
		 * @todo we could leave out MIN_SIZE_ALIGNED. We would create an empty
		 *       region that couldn't hold data, but when mem->next gets freed,
		 *       the 2 regions would be combined, resulting in more free memory */
		ptr2 = ptr + SIZEOF_STRUCT_MEM + newsize;
		mem2 = (struct mem*) &ram[ptr2];
		if (mem2 < lfree)
		{
			lfree = mem2;
		} mem2->used = 0;
		#if USE_MEM_DEBUG
		mem2->name = not_used_name;
		#endif
		mem2->next = mem->next;
		mem2->prev = ptr;
		mem->next = ptr2;
		if (mem2->next != MEM_SIZE_ALIGNED)
		{
			((struct mem*) &ram[mem2->next])->prev = ptr2;
		}
		//MEM_STATS_DEC_USED(used, (size - newsize));
		SUB_MONITOR_ITEM_VALUE(mem_used, (size - newsize));
		ram_used -= size - newsize;
		/* the original mem->next is used, so no need to plug holes! */
	}
	/* else {
	next struct mem is used but size between mem and mem2 is not big enough
	to create another struct mem
	-> don't do anyhting.
	-> the remaining space stays unused since it is too small
	} */
	#if LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT
	mem_free_count = 1;
	#endif /* LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT */
	LWIP_MEM_FREE_UNPROTECT();
	return rmem;
}

/**
 * Adam's mem_malloc() plus solution for bug #17922
 * Allocate a block of memory with a minimum of 'size' bytes.
 *
 * @param size is the minimum size of the requested block in bytes.
 * @return pointer to allocated memory or NULL if no free memory was found.
 *
 * Note that the returned value will always be aligned (as defined by MEM_ALIGNMENT).
 */
#if USE_MEM_DEBUG
void *mem_malloc_ex(const char *name, mem_size_t size)
{
	static int once = 0;
	int local_once = 0;
	mem_size_t ptr, ptr2;
	struct mem *mem,  *mem2;
	#if LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT
	unsigned char local_mem_free_count = 0;
	#endif /* LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT */
	LWIP_MEM_ALLOC_DECL_PROTECT();

	if (size == 0)
	{
		return NULL;
	}

	/* Expand the size of the allocated memory region so that we can
	adjust for alignment. */
	size = LWIP_MEM_ALIGN_SIZE(size + MEM_CRITICAL_AREA_SIZE);

	if (size < MIN_SIZE_ALIGNED)
	{
		/* every data block must be at least MIN_SIZE_ALIGNED long */
		size = MIN_SIZE_ALIGNED;
	}

	if (size > MEM_SIZE_ALIGNED)
	{
		return NULL;
	}
	again:
	/* protect the heap from concurrent access */
	LWIP_MEM_ALLOC_PROTECT();
	#if LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT
	/* run as long as a mem_free disturbed mem_malloc or mem_trim */
	do
	{
		local_mem_free_count = 0;
		#endif /* LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT */

		/* Scan through the heap searching for a free block that is big enough,
		 * beginning with the lowest free block.
		 */
		for (ptr = (mem_size_t)((unsigned char*)lfree - ram); ptr < (MEM_SIZE_ALIGNED /* - SIZEOF_STRUCT_MEM*/ /*add by forrest*/ - size); ptr = ((struct mem*)(void*) &ram[ptr])->next)
		{
			mem = (struct mem*)(void*) &ram[ptr];
			#if LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT
			mem_free_count = 0;
			//LWIP_MEM_ALLOC_UNPROTECT();
			/* allow mem_free or mem_trim to run */
			//LWIP_MEM_ALLOC_PROTECT();
			if (mem_free_count != 0)
			{
				/* If mem_free or mem_trim have run, we have to restart since they
				could have altered our current struct mem. */
				//   local_mem_free_count = 1;
				//   break;
				local_mem_free_count = mem_free_count;
			} mem_free_count = 0;
			#endif /* LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT */

			if ((!mem->used) && (mem->next - (ptr + SIZEOF_STRUCT_MEM)) >= size)
			{
				/* mem is not used and at least perfect fit is possible:
				 * mem->next - (ptr + SIZEOF_STRUCT_MEM) gives us the 'user data size' of mem */

				if (mem->next - (ptr + SIZEOF_STRUCT_MEM) >= (size + SIZEOF_STRUCT_MEM + MIN_SIZE_ALIGNED))
				{
					/* (in addition to the above, we test if another struct mem (SIZEOF_STRUCT_MEM) containing
					 * at least MIN_SIZE_ALIGNED of data also fits in the 'user data space' of 'mem')
					 * -> split large block, create empty remainder,
					 * remainder must be large enough to contain MIN_SIZE_ALIGNED data: if
					 * mem->next - (ptr + (2*SIZEOF_STRUCT_MEM)) == size,
					 * struct mem would fit in but no data between mem2 and mem2->next
					 * @todo we could leave out MIN_SIZE_ALIGNED. We would create an empty
					 *       region that couldn't hold data, but when mem->next gets freed,
					 *       the 2 regions would be combined, resulting in more free memory
					 */
					ptr2 = ptr + SIZEOF_STRUCT_MEM + size;
					/* create mem2 struct */
					mem2 = (struct mem*) &ram[ptr2];
					mem2->used = 0;
					#if USE_MEM_DEBUG
					mem2->name = not_used_name;
					#endif
					mem2->next = mem->next;
					mem2->prev = ptr;
					/* and insert it between mem and mem->next */
					mem->next = ptr2;
					{
						uint32_t *p_used = (uint32_t*) &mem->used;
						*p_used = 1 | (malloc_cnt << 8);
					} 
					malloc_cnt++;
					#if USE_MEM_DEBUG
					mem->name = name;
					*((uint32_t*)(&ram[((uint32_t)mem->next) - MEM_CRITICAL_AREA_SIZE])) = MEM_CRITICAL_FLAG;
					#endif

					if (mem2->next != MEM_SIZE_ALIGNED)
					{
						((struct mem*) &ram[mem2->next])->prev = ptr2;
					}
					//MEM_STATS_INC_USED(used, (size + SIZEOF_STRUCT_MEM));
					ADD_MONITOR_ITEM_VALUE(mem_used, (size + SIZEOF_STRUCT_MEM));
					ram_used += size + SIZEOF_STRUCT_MEM;
				}
				else
				{
					/* (a mem2 struct does no fit into the user data space of mem and mem->next will always
					 * be used at this point: if not we have 2 unused structs in a row, plug_holes should have
					 * take care of this).
					 * -> near fit or excact fit: do not split, no mem2 creation
					 * also can't move mem->next directly behind mem, since mem->next
					 * will always be used at this point!
					 */
					{
						uint32_t *p_used = (uint32_t*) &mem->used;
						*p_used = 1 | (malloc_cnt << 8);
					} malloc_cnt++;
					#if USE_MEM_DEBUG
					mem->name = name;
					*((uint32_t*)(&ram[((uint32_t)mem->next) - MEM_CRITICAL_AREA_SIZE])) = MEM_CRITICAL_FLAG;
					#endif
					//MEM_STATS_INC_USED(used, mem->next - (mem_size_t)((unsigned char *)mem - ram));
					ram_used += mem->next - (mem_size_t)((unsigned char*)mem - ram);
					ADD_MONITOR_ITEM_VALUE(mem_used, mem->next - (mem_size_t)((unsigned char*)mem - ram));
				}

				if (mem == lfree)
				{
					while (lfree->used && lfree != ram_end)
					{
						//LWIP_MEM_ALLOC_UNPROTECT(); /* prevent high interrupt latency...* /
						
						//LWIP_MEM_ALLOC_PROTECT();
						lfree = (struct mem*) &ram[lfree->next];
					}
					#if USE_MEM_ASSERT
					LWIP_ASSERT("mem_malloc: !lfree->used", ((lfree == ram_end) || (!lfree->used)));
					#else
					if (!(((lfree == ram_end) || (!lfree->used))))
					{
					#if USE_MEM_DEBUG	
						mem_slide_check(FALSE);
					#endif
						p_err_fun;
						while (1)
							;
					}
					#endif
				}

				LWIP_MEM_ALLOC_UNPROTECT();

				#if USE_MEM_ASSERT
				LWIP_ASSERT("mem_malloc: allocated memory not above ram_end.", (mem_ptr_t)mem + SIZEOF_STRUCT_MEM + size <= (mem_ptr_t)ram_end);
				LWIP_ASSERT("mem_malloc: allocated memory properly aligned.", ((mem_ptr_t)mem + SIZEOF_STRUCT_MEM) % MEM_ALIGNMENT == 0);
				LWIP_ASSERT("mem_malloc: sanity check alignment", (((mem_ptr_t)mem) &(MEM_ALIGNMENT - 1)) == 0);
				#else
				if (!(((mem_ptr_t)mem + SIZEOF_STRUCT_MEM + size <= (mem_ptr_t)ram_end) && ((((mem_ptr_t)mem) &(MEM_ALIGNMENT - 1)) == 0) && (((mem_ptr_t)mem + SIZEOF_STRUCT_MEM) % MEM_ALIGNMENT == 0)))
				{
					mem_slide_check(FALSE);
					p_err_fun;
					while (1)
						;
				}

				#endif
				return (unsigned char*)mem + SIZEOF_STRUCT_MEM;
			}
		}
		#if LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT
		/* if we got interrupted by a mem_free, try again */
	}
	while (local_mem_free_count != 0)
		;
	#endif /* LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT */
	//p_err("mem_malloc: could not allocate %d bytes, mem_used:%d\n", size, ram_used);
	//MEM_STATS_INC(err);
	INC_MONITOR_ITEM_VALUE(mem_err);
	LWIP_MEM_ALLOC_UNPROTECT();
	mem_slide_check(FALSE);
	if (once == 0)
	{
		once = 1;
		//mem_slide_check(TRUE);
	}

	if (local_once == 0)
	{
		local_once = 1;
		goto again;
	}

	return NULL;
}

#else

void *mem_malloc(mem_size_t size)
{
	mem_size_t ptr, ptr2;
	struct mem *mem,  *mem2;
	#if LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT
	unsigned char local_mem_free_count = 0;
	#endif /* LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT */
	LWIP_MEM_ALLOC_DECL_PROTECT();

	if (size == 0)
	{
		return NULL;
	}

	/* Expand the size of the allocated memory region so that we can
	adjust for alignment. */
	size = LWIP_MEM_ALIGN_SIZE(size);

	if (size < MIN_SIZE_ALIGNED)
	{
		/* every data block must be at least MIN_SIZE_ALIGNED long */
		size = MIN_SIZE_ALIGNED;
	}

	if (size > MEM_SIZE_ALIGNED)
	{
		return NULL;
	}

	/* protect the heap from concurrent access */
	LWIP_MEM_ALLOC_PROTECT();
	#if LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT
	/* run as long as a mem_free disturbed mem_malloc or mem_trim */
	do
	{
		local_mem_free_count = 0;
		#endif /* LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT */

		/* Scan through the heap searching for a free block that is big enough,
		 * beginning with the lowest free block.
		 */
		for (ptr = (mem_size_t)((unsigned char*)lfree - ram); ptr < (MEM_SIZE_ALIGNED /* - SIZEOF_STRUCT_MEM*/ /*add by forrest*/ - size); ptr = ((struct mem*)(void*) &ram[ptr])->next)
		{
			mem = (struct mem*)(void*) &ram[ptr];
			#if LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT
			mem_free_count = 0;
			LWIP_MEM_ALLOC_UNPROTECT();
			/* allow mem_free or mem_trim to run */
			LWIP_MEM_ALLOC_PROTECT();
			if (mem_free_count != 0)
			{
				/* If mem_free or mem_trim have run, we have to restart since they
				could have altered our current struct mem. */
				//   local_mem_free_count = 1;
				//   break;
				local_mem_free_count = mem_free_count;
			} mem_free_count = 0;
			#endif /* LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT */

			if ((!mem->used) && (mem->next - (ptr + SIZEOF_STRUCT_MEM)) >= size)
			{
				/* mem is not used and at least perfect fit is possible:
				 * mem->next - (ptr + SIZEOF_STRUCT_MEM) gives us the 'user data size' of mem */

				if (mem->next - (ptr + SIZEOF_STRUCT_MEM) >= (size + SIZEOF_STRUCT_MEM + MIN_SIZE_ALIGNED))
				{
					/* (in addition to the above, we test if another struct mem (SIZEOF_STRUCT_MEM) containing
					 * at least MIN_SIZE_ALIGNED of data also fits in the 'user data space' of 'mem')
					 * -> split large block, create empty remainder,
					 * remainder must be large enough to contain MIN_SIZE_ALIGNED data: if
					 * mem->next - (ptr + (2*SIZEOF_STRUCT_MEM)) == size,
					 * struct mem would fit in but no data between mem2 and mem2->next
					 * @todo we could leave out MIN_SIZE_ALIGNED. We would create an empty
					 *       region that couldn't hold data, but when mem->next gets freed,
					 *       the 2 regions would be combined, resulting in more free memory
					 */
					ptr2 = ptr + SIZEOF_STRUCT_MEM + size;
					/* create mem2 struct */
					mem2 = (struct mem*) &ram[ptr2];
					mem2->used = 0;
					#if USE_MEM_DEBUG
					mem2->name = not_used_name;
					#endif
					mem2->next = mem->next;
					mem2->prev = ptr;
					/* and insert it between mem and mem->next */
					mem->next = ptr2;
					mem->used = 1;

					if (mem2->next != MEM_SIZE_ALIGNED)
					{
						((struct mem*) &ram[mem2->next])->prev = ptr2;
					}
					//MEM_STATS_INC_USED(used, (size + SIZEOF_STRUCT_MEM));
					ADD_MONITOR_ITEM_VALUE(mem_used, (size + SIZEOF_STRUCT_MEM));
					ram_used += size + SIZEOF_STRUCT_MEM;
				}
				else
				{
					/* (a mem2 struct does no fit into the user data space of mem and mem->next will always
					 * be used at this point: if not we have 2 unused structs in a row, plug_holes should have
					 * take care of this).
					 * -> near fit or excact fit: do not split, no mem2 creation
					 * also can't move mem->next directly behind mem, since mem->next
					 * will always be used at this point!
					 */
					mem->used = 1;
					//MEM_STATS_INC_USED(used, mem->next - (mem_size_t)((unsigned char *)mem - ram));
					ram_used += mem->next - (mem_size_t)((unsigned char*)mem - ram);
					ADD_MONITOR_ITEM_VALUE(mem_used, mem->next - (mem_size_t)((unsigned char*)mem - ram));
				}

				if (mem == lfree)
				{
					while (lfree->used && lfree != ram_end)
					{
						LWIP_MEM_ALLOC_UNPROTECT(); /* prevent high
						interrupt latency...
						 */
						LWIP_MEM_ALLOC_PROTECT();
						lfree = (struct mem*) &ram[lfree->next];
					}
                    #if USE_MEM_ASSERT
					LWIP_ASSERT("mem_malloc: !lfree->used", ((lfree == ram_end) || (!lfree->used)));
                    #endif
				}
				LWIP_MEM_ALLOC_UNPROTECT();
                #if USE_MEM_ASSERT
				LWIP_ASSERT("mem_malloc: allocated memory not above ram_end.", (mem_ptr_t)mem + SIZEOF_STRUCT_MEM + size <= (mem_ptr_t)ram_end);
				LWIP_ASSERT("mem_malloc: allocated memory properly aligned.", ((mem_ptr_t)mem + SIZEOF_STRUCT_MEM) % MEM_ALIGNMENT == 0);
				LWIP_ASSERT("mem_malloc: sanity check alignment", (((mem_ptr_t)mem) &(MEM_ALIGNMENT - 1)) == 0);
                #endif
				return (unsigned char*)mem + SIZEOF_STRUCT_MEM;
			}
		}
		#if LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT
		/* if we got interrupted by a mem_free, try again */
	}
	while (local_mem_free_count != 0)
		;
	#endif /* LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT */
	//p_dbg("mem_malloc: could not allocate %" S16_F " bytes\n", (s16_t)size);
	//MEM_STATS_INC(err);
	INC_MONITOR_ITEM_VALUE(mem_err);
	LWIP_MEM_ALLOC_UNPROTECT();
	return NULL;
}

#endif


/**
 * Contiguously allocates enough space for count objects that are size bytes
 * of memory each and returns a pointer to the allocated memory.
 *
 * The allocated memory is filled with bytes of value zero.
 *
 * @param count number of objects to allocate
 * @param size size of the objects to allocate
 * @return pointer to allocated memory / NULL pointer if there is an error
 */
#if USE_MEM_DEBUG
void *mem_calloc_ex(const char *name, mem_size_t count, mem_size_t size)
{
	void *p;

	/* allocate 'count' objects of size 'size' */
	p = (void*)mem_malloc_ex(name, count *size);
	if (p)
	{
		/* zero the memory */
		memset(p, 0, count *size);
	} return p;
}

#else

void *mem_calloc(mem_size_t count, mem_size_t size)
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

#endif

extern char Image$$RW_IRAM1$$ZI$$Limit[];

#define const_data_range_ok(X)	(((uint32_t)(X )< 0x8100000) && ((uint32_t)(X) >= 0x8004000))

#define alloc_mem_range_ok(X)  ((uint32_t)(X) < (0x20000000 + 0x30000) && ((uint32_t)(X) >= (uint32_t) &Image$$RW_IRAM1$$ZI$$Limit))

#define is_mem_head_ok(X) ((alloc_mem_range_ok(X))\
&& ((X->next == 0) || alloc_mem_range_ok(X->next))\
&& ((X->prev == 0) || alloc_mem_range_ok(X->prev))\
&& ((X->used == 0) || (X->used == 1) )\
&& alloc_mem_range_ok(X->name))


#if USE_MEM_DEBUG
extern UART_SEND u1_send;

void mem_slide_check(int show_list)
{
	uint32_t i = 0;
	uint32_t totol = 0;
	
	struct mem *mem;
	char *p_char;
	mem_size_t ptr;
	LWIP_MEM_ALLOC_DECL_PROTECT();
	
#if USE_14_CHANNELS
	static int once = 0;

#endif
	LWIP_MEM_ALLOC_PROTECT();
again:
	for (ptr = 0; ptr < MEM_SIZE_ALIGNED; ptr = ((struct mem*)(void*) &ram[ptr])->next)
	{

		assert(ptr < MEM_SIZE_ALIGNED);

		mem = (struct mem*) &ram[ptr];

		//assert(is_mem_head_ok(mem));
		/*
		assert(alloc_mem_range_ok(mem));
		assert((mem->next) <= MEM_SIZE_ALIGNED);
		assert((mem->prev) < MEM_SIZE_ALIGNED);
		assert((mem->used == 0) || (mem->used == 1));
		assert(const_data_range_ok(mem->name));*/
		if (!((alloc_mem_range_ok(mem)) && ((mem->next) <= MEM_SIZE_ALIGNED) && ((mem->prev) < MEM_SIZE_ALIGNED) && ((mem->used == 0) || (mem->used == 1)) && (const_data_range_ok(mem->name))))
		//如果出现异常，把异常之前的内存信息打印出来
		{
			if (show_list == 0)
			{
				ptr = 0;
				show_list = 1;
				goto again;
			}
			else
				u1_send.sending.val = 0;

			assert(alloc_mem_range_ok(mem));
			p_err("err mem, addr:%x, next:%x, prev:%x", mem, &ram[mem->next], &ram[mem->prev]);
			assert((mem->next) <= MEM_SIZE_ALIGNED);
			assert((mem->prev) < MEM_SIZE_ALIGNED);
			assert((mem->used == 0) || (mem->used == 1));
			assert(const_data_range_ok(mem->name));
		}

		if (mem->next == 0)
			break;
		if (mem->next == mem->prev)
			break;
		p_char = (char*)mem;

		if (show_list)
		{
			char name[32];
			uint32_t *p_used = (uint32_t*) &mem->used;
			snprintf(name, 32, "%s", mem->name);
			name[31] = 0;
			u1_send.sending.val = 0;
			p_err("m%d:%s, addr:%08x,size:%d,%d, %d\n", i, name, p_char + sizeof(struct mem), mem->next - ((uint32_t)p_char - (uint32_t)ram) - sizeof(struct mem), (*p_used) >> 8, mem->used);
			totol += mem->next - ((uint32_t)p_char - (uint32_t)ram) - sizeof(struct mem);
			 
		}
		if (mem->used)
		{
			i++;
			p_char = ((char*)mem->next) - MEM_CRITICAL_AREA_SIZE;

			if (*((uint32_t*) &ram[(uint32_t)p_char]) != MEM_CRITICAL_FLAG)
			{
				u1_send.sending.val = 0;
				p_err("detect memory overflow, mem_size:%d, flag:%x", (uint32_t)((uint32_t*) &ram[(uint32_t)p_char]) - (uint32_t)mem - SIZEOF_STRUCT_MEM, *(uint32_t*) &ram[(uint32_t)p_char]);
				p_err("mem_head:%x, data_addr:%x, last addr:%x, next_addr:%x", mem, (uint32_t)mem + SIZEOF_STRUCT_MEM, &ram[mem->prev], &ram[mem->next]);
				if (USE_MEM_DEBUG)
				{
					char name[32];
					struct mem *tmp_mem;

					p_err("critical addr:%x", ((uint32_t*) &ram[(uint32_t)p_char]));

					snprintf(name, 32, "%s", mem->name);
					p_err("name:%s", name);

					tmp_mem = (struct mem*) &ram[mem->prev];
					snprintf(name, 32, "%s", tmp_mem->name);
					p_err("last name:%s", name);

					tmp_mem = (struct mem*) &ram[mem->next];
					snprintf(name, 32, "%s", tmp_mem->name);
					p_err("next name:%s", name);
				}
				while (1)
					;
			}
		}
	}
	LWIP_MEM_ALLOC_UNPROTECT();
	SET_MONITOR_ITEM_VALUE(mem_block, i);
}
#endif

uint32_t mem_get_size()
{
	return ram_totol;
}

uint32_t mem_get_free()
{
	return ram_totol - ram_used;
}

#elif MEM_MAN_MODE == 2

#define portBYTE_ALIGNMENT			8
#define portBYTE_ALIGNMENT_MASK ( 0x0007U )
/* Block sizes must not get too small. */
#define heapMINIMUM_BLOCK_SIZE	( ( size_t ) ( xHeapStructSize * 2 ) )

/* Assumes 8bit bytes! */
#define heapBITS_PER_BYTE		( ( size_t ) 8 )

#define mtCOVERAGE_TEST_MARKER()
/* Allocate the memory for the heap. */
//#if( configAPPLICATION_ALLOCATED_HEAP == 1 )
//	/* The application writer has already defined the array used for the RTOS
//	heap - probably so it can be placed in a special segment or address. */
//	extern uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
//#else
//	static uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
//#endif /* configAPPLICATION_ALLOCATED_HEAP */

static uint8_t *ucHeap;
uint32_t ucHeap_size = 0;

/* Define the linked list structure.  This is used to link free blocks in order
of their memory address. */
typedef struct A_BLOCK_LINK
{
	struct A_BLOCK_LINK *pxNextFreeBlock;	/*<< The next free block in the list. */
	size_t xBlockSize;						/*<< The size of the free block. */
} BlockLink_t;

/*-----------------------------------------------------------*/

/*
 * Inserts a block of memory that is being freed into the correct position in
 * the list of free memory blocks.  The block being freed will be merged with
 * the block in front it and/or the block behind it if the memory blocks are
 * adjacent to each other.
 */
static void prvInsertBlockIntoFreeList( BlockLink_t *pxBlockToInsert );

/*
 * Called automatically to setup the required heap structures the first time
 * pvPortMalloc() is called.
 */
static void prvHeapInit( void );

/*-----------------------------------------------------------*/

/* The size of the structure placed at the beginning of each allocated memory
block must by correctly byte aligned. */
static const size_t xHeapStructSize	= ( ( sizeof( BlockLink_t ) + ( portBYTE_ALIGNMENT - 1 ) ) & ~portBYTE_ALIGNMENT_MASK );

/* Create a couple of list links to mark the start and end of the list. */
static BlockLink_t xStart, *pxEnd = NULL;

/* Keeps track of the number of free bytes remaining, but says nothing about
fragmentation. */
static size_t xFreeBytesRemaining = 0U;
static size_t xMinimumEverFreeBytesRemaining = 0U;

/* Gets set to the top bit of an size_t type.  When this bit in the xBlockSize
member of an BlockLink_t structure is set then the block belongs to the
application.  When the bit is free the block is still part of the free heap
space. */
static size_t xBlockAllocatedBit = 0;

/*-----------------------------------------------------------*/

void *mem_malloc( size_t xWantedSize )
{
	BlockLink_t *pxBlock, *pxPreviousBlock, *pxNewBlockLink;
	void *pvReturn = NULL;
	uint32_t irq_flag;
	//vTaskSuspendAll();
	irq_flag = local_irq_save();
	{
		/* If this is the first call to malloc then the heap will require
		initialisation to setup the list of free blocks. */
		if( pxEnd == NULL )
		{
			prvHeapInit();
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}

		/* Check the requested block size is not so large that the top bit is
		set.  The top bit of the block size member of the BlockLink_t structure
		is used to determine who owns the block - the application or the
		kernel, so it must be free. */
		if( ( xWantedSize & xBlockAllocatedBit ) == 0 )
		{
			/* The wanted size is increased so it can contain a BlockLink_t
			structure in addition to the requested amount of bytes. */
			if( xWantedSize > 0 )
			{
				xWantedSize += xHeapStructSize;

				/* Ensure that blocks are always aligned to the required number
				of bytes. */
				if( ( xWantedSize & portBYTE_ALIGNMENT_MASK ) != 0x00 )
				{
					/* Byte alignment required. */
					xWantedSize += ( portBYTE_ALIGNMENT - ( xWantedSize & portBYTE_ALIGNMENT_MASK ) );
					assert( ( xWantedSize & portBYTE_ALIGNMENT_MASK ) == 0 );
				}
				else
				{
					mtCOVERAGE_TEST_MARKER();
				}
			}
			else
			{
				mtCOVERAGE_TEST_MARKER();
			}

			if( ( xWantedSize > 0 ) && ( xWantedSize <= xFreeBytesRemaining ) )
			{
				/* Traverse the list from the start	(lowest address) block until
				one	of adequate size is found. */
				pxPreviousBlock = &xStart;
				pxBlock = xStart.pxNextFreeBlock;
				while( ( pxBlock->xBlockSize < xWantedSize ) && ( pxBlock->pxNextFreeBlock != NULL ) )
				{
					pxPreviousBlock = pxBlock;
					pxBlock = pxBlock->pxNextFreeBlock;
				}

				/* If the end marker was reached then a block of adequate size
				was	not found. */
				if( pxBlock != pxEnd )
				{
					/* Return the memory space pointed to - jumping over the
					BlockLink_t structure at its start. */
					pvReturn = ( void * ) ( ( ( uint8_t * ) pxPreviousBlock->pxNextFreeBlock ) + xHeapStructSize );

					/* This block is being returned for use so must be taken out
					of the list of free blocks. */
					pxPreviousBlock->pxNextFreeBlock = pxBlock->pxNextFreeBlock;

					/* If the block is larger than required it can be split into
					two. */
					if( ( pxBlock->xBlockSize - xWantedSize ) > heapMINIMUM_BLOCK_SIZE )
					{
						/* This block is to be split into two.  Create a new
						block following the number of bytes requested. The void
						cast is used to prevent byte alignment warnings from the
						compiler. */
						pxNewBlockLink = ( void * ) ( ( ( uint8_t * ) pxBlock ) + xWantedSize );
						assert( ( ( ( uint32_t ) pxNewBlockLink ) & portBYTE_ALIGNMENT_MASK ) == 0 );

						/* Calculate the sizes of two blocks split from the
						single block. */
						pxNewBlockLink->xBlockSize = pxBlock->xBlockSize - xWantedSize;
						pxBlock->xBlockSize = xWantedSize;

						/* Insert the new block into the list of free blocks. */
						prvInsertBlockIntoFreeList( ( pxNewBlockLink ) );
					}
					else
					{
						mtCOVERAGE_TEST_MARKER();
					}

					xFreeBytesRemaining -= pxBlock->xBlockSize;

					if( xFreeBytesRemaining < xMinimumEverFreeBytesRemaining )
					{
						xMinimumEverFreeBytesRemaining = xFreeBytesRemaining;
					}
					else
					{
						mtCOVERAGE_TEST_MARKER();
					}

					/* The block is being returned - it is allocated and owned
					by the application and has no "next" block. */
					pxBlock->xBlockSize |= xBlockAllocatedBit;
					pxBlock->pxNextFreeBlock = NULL;
				}
				else
				{
					mtCOVERAGE_TEST_MARKER();
				}
			}
			else
			{
				mtCOVERAGE_TEST_MARKER();
			}
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}

		//traceMALLOC( pvReturn, xWantedSize );
	}
	local_irq_restore(irq_flag);
	//( void ) xTaskResumeAll();

	#if( configUSE_MALLOC_FAILED_HOOK == 1 )
	{
		if( pvReturn == NULL )
		{
			extern void vApplicationMallocFailedHook( void );
			vApplicationMallocFailedHook();
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}
	}
	#endif

	assert( ( ( ( uint32_t ) pvReturn ) & portBYTE_ALIGNMENT_MASK ) == 0 );
	return pvReturn;
}
/*-----------------------------------------------------------*/

void mem_free( void *pv )
{
uint8_t *puc = ( uint8_t * ) pv;
BlockLink_t *pxLink;

	if( pv != NULL )
	{
		/* The memory being freed will have an BlockLink_t structure immediately
		before it. */
		puc -= xHeapStructSize;

		/* This casting is to keep the compiler from issuing warnings. */
		pxLink = ( void * ) puc;

		/* Check the block is actually allocated. */
		assert( ( pxLink->xBlockSize & xBlockAllocatedBit ) != 0 );
		assert( pxLink->pxNextFreeBlock == NULL );

		if( ( pxLink->xBlockSize & xBlockAllocatedBit ) != 0 )
		{
			if( pxLink->pxNextFreeBlock == NULL )
			{
				/* The block is being returned to the heap - it is no longer
				allocated. */
				uint32_t irq_flag;
				pxLink->xBlockSize &= ~xBlockAllocatedBit;
				
				irq_flag = local_irq_save();
				//vTaskSuspendAll();
				{
					/* Add this block to the list of free blocks. */
					xFreeBytesRemaining += pxLink->xBlockSize;
					//traceFREE( pv, pxLink->xBlockSize );
					prvInsertBlockIntoFreeList( ( ( BlockLink_t * ) pxLink ) );
				}
				local_irq_restore(irq_flag);
				//( void ) xTaskResumeAll();
			}
			else
			{
				mtCOVERAGE_TEST_MARKER();
			}
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}
	}
}
/*-----------------------------------------------------------*/

static size_t xPortGetFreeHeapSize( void )
{
	return xFreeBytesRemaining;
}
/*-----------------------------------------------------------*/
/*
static size_t xPortGetMinimumEverFreeHeapSize( void )
{
	return xMinimumEverFreeBytesRemaining;
}*/
/*-----------------------------------------------------------*/


static void prvHeapInit( void )
{
BlockLink_t *pxFirstFreeBlock;
uint8_t *pucAlignedHeap;
uint32_t ulAddress;
size_t xTotalHeapSize = ucHeap_size;

	/* Ensure the heap starts on a correctly aligned boundary. */
	
	ulAddress = ( uint32_t ) ucHeap;

	if( ( ulAddress & portBYTE_ALIGNMENT_MASK ) != 0 )
	{
		ulAddress += ( portBYTE_ALIGNMENT - 1 );
		ulAddress &= ~portBYTE_ALIGNMENT_MASK;
		xTotalHeapSize -= ulAddress - ( uint32_t ) ucHeap;
	}

	pucAlignedHeap = ( uint8_t * ) ulAddress;

	/* xStart is used to hold a pointer to the first item in the list of free
	blocks.  The void cast is used to prevent compiler warnings. */
	xStart.pxNextFreeBlock = ( void * ) pucAlignedHeap;
	xStart.xBlockSize = ( size_t ) 0;

	/* pxEnd is used to mark the end of the list of free blocks and is inserted
	at the end of the heap space. */
	ulAddress = ( ( uint32_t ) pucAlignedHeap ) + xTotalHeapSize;
	ulAddress -= xHeapStructSize;
	ulAddress &= ~portBYTE_ALIGNMENT_MASK;
	pxEnd = ( void * ) ulAddress;
	pxEnd->xBlockSize = 0;
	pxEnd->pxNextFreeBlock = NULL;

	/* To start with there is a single free block that is sized to take up the
	entire heap space, minus the space taken by pxEnd. */
	pxFirstFreeBlock = ( void * ) pucAlignedHeap;
	pxFirstFreeBlock->xBlockSize = ulAddress - ( uint32_t ) pxFirstFreeBlock;
	pxFirstFreeBlock->pxNextFreeBlock = pxEnd;

	/* Only one block exists - and it covers the entire usable heap space. */
	xMinimumEverFreeBytesRemaining = pxFirstFreeBlock->xBlockSize;
	xFreeBytesRemaining = pxFirstFreeBlock->xBlockSize;

	/* Work out the position of the top bit in a size_t variable. */
	xBlockAllocatedBit = ( ( size_t ) 1 ) << ( ( sizeof( size_t ) * heapBITS_PER_BYTE ) - 1 );
}
/*-----------------------------------------------------------*/

static void prvInsertBlockIntoFreeList( BlockLink_t *pxBlockToInsert )
{
BlockLink_t *pxIterator;
uint8_t *puc;

	/* Iterate through the list until a block is found that has a higher address
	than the block being inserted. */
	for( pxIterator = &xStart; pxIterator->pxNextFreeBlock < pxBlockToInsert; pxIterator = pxIterator->pxNextFreeBlock )
	{
		/* Nothing to do here, just iterate to the right position. */
	}

	/* Do the block being inserted, and the block it is being inserted after
	make a contiguous block of memory? */
	puc = ( uint8_t * ) pxIterator;
	if( ( puc + pxIterator->xBlockSize ) == ( uint8_t * ) pxBlockToInsert )
	{
		pxIterator->xBlockSize += pxBlockToInsert->xBlockSize;
		pxBlockToInsert = pxIterator;
	}
	else
	{
		mtCOVERAGE_TEST_MARKER();
	}

	/* Do the block being inserted, and the block it is being inserted before
	make a contiguous block of memory? */
	puc = ( uint8_t * ) pxBlockToInsert;
	if( ( puc + pxBlockToInsert->xBlockSize ) == ( uint8_t * ) pxIterator->pxNextFreeBlock )
	{
		if( pxIterator->pxNextFreeBlock != pxEnd )
		{
			/* Form one big block from the two blocks. */
			pxBlockToInsert->xBlockSize += pxIterator->pxNextFreeBlock->xBlockSize;
			pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock->pxNextFreeBlock;
		}
		else
		{
			pxBlockToInsert->pxNextFreeBlock = pxEnd;
		}
	}
	else
	{
		pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock;
	}

	/* If the block being inserted plugged a gab, so was merged with the block
	before and the block after, then it's pxNextFreeBlock pointer will have
	already been set, and should not be set here as that would make it point
	to itself. */
	if( pxIterator != pxBlockToInsert )
	{
		pxIterator->pxNextFreeBlock = pxBlockToInsert;
	}
	else
	{
		mtCOVERAGE_TEST_MARKER();
	}
}

void sys_meminit(void *pool, unsigned int size)
{
	ucHeap_size = size;
	ucHeap = pool;
	prvHeapInit();
}

void mem_slide_check(int show_list)
{
	
}

uint32_t mem_get_size()
{
	return ucHeap_size;
}

uint32_t mem_get_free()
{
	return xPortGetFreeHeapSize();
}

#endif

void *mem_realloc(void *ptr, size_t size)
{
	void *mem;
	mem = mem_calloc(size, 1);
	if(!mem)
		return 0;
	
	if(ptr){
		memcpy(mem, ptr, size);
		mem_free(ptr);
	}
	return mem;
}


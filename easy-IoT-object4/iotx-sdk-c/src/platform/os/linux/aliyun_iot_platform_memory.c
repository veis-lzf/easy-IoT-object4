#include "aliyun_iot_platform_memory.h"
//#include "lwip/mem.h"
#include "memory2.h"
void *aliyun_iot_memory_malloc(uint32_t size)
{
    void *data = mem_malloc2(size);
    if(!data)    printf("[ZZZZZZZZZZZZZZ] aliyun_iot_memory_malloc fail error\n");
    return data;
}

void aliyun_iot_memory_free(void *ptr)
{
    if(ptr) mem_free2(ptr);
    return;
}

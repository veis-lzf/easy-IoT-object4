/**
 * @file
 * Network buffer management
 *
 */
 
/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#include "lwip/opt.h"

#if LWIP_NETCONN /* don't build if not configured for use in lwipopts.h */

#include "lwip/netbuf.h"
#include "lwip/memp.h"

#include <string.h>

/**
 * Create (allocate) and initialize a new netbuf.
 * The netbuf doesn't yet contain a packet buffer!
 *
 * @return a pointer to a new netbuf
 *         NULL on lack of memory
 */
struct
netbuf *netbuf_new(void)     //申请一个新的NETBUF空间，但不会分配任何数据空间，函数会返回一个netbuf结构指针，指向申请成功的NETBUF结构
{
  struct netbuf *buf;

  buf = (struct netbuf *)memp_malloc(MEMP_NETBUF);
  if (buf != NULL) {
    buf->p = NULL;
    buf->ptr = NULL;
    ip_addr_set_any(&buf->addr);
    buf->port = 0;
#if LWIP_NETBUF_RECVINFO || LWIP_CHECKSUM_ON_COPY
#if LWIP_CHECKSUM_ON_COPY
    buf->flags = 0;
#endif /* LWIP_CHECKSUM_ON_COPY */
    buf->toport_chksum = 0;
#if LWIP_NETBUF_RECVINFO
    ip_addr_set_any(&buf->toaddr);
#endif /* LWIP_NETBUF_RECVINFO */
#endif /* LWIP_NETBUF_RECVINFO || LWIP_CHECKSUM_ON_COPY */
    return buf;
  } else {
    return NULL;
  }
}

/**
 * Deallocate a netbuf allocated by netbuf_new().
 *
 * @param buf pointer to a netbuf allocated by netbuf_new()
 */
void
netbuf_delete(struct netbuf *buf)     //释放一个nutbuf空间
{
  if (buf != NULL) {
    if (buf->p != NULL) {
      pbuf_free(buf->p);
      buf->p = buf->ptr = NULL;
    }
    memp_free(MEMP_NETBUF, buf);
  }
}

/**
 * Allocate memory for a packet buffer for a given netbuf.
 *
 * @param buf the netbuf for which to allocate a packet buffer
 * @param size the size of the packet buffer to allocate
 * @return pointer to the allocated memory
 *         NULL if no memory could be allocated
 */
void *
netbuf_alloc(struct netbuf *buf, u16_t size)      //功能是为?netbuf?结构分配?size?大小的数据空间，当然这个数据空间是通过pbuf?的形式来表现的
{
  LWIP_ERROR("netbuf_alloc: invalid buf", (buf != NULL), return NULL;);

  /* Deallocate any previously allocated memory. */
  if (buf->p != NULL) {
    pbuf_free(buf->p);
  }
  buf->p = pbuf_alloc(PBUF_TRANSPORT, size, PBUF_RAM);
  if (buf->p == NULL) {
     return NULL;
  }
  LWIP_ASSERT("check that first pbuf can hold size",
             (buf->p->len >= size));
  buf->ptr = buf->p;
  return buf->p->payload;
}

/**
 * Free the packet buffer included in a netbuf
 *
 * @param buf pointer to the netbuf which contains the packet buffer to free
 */
void
netbuf_free(struct netbuf *buf)       //功能是释放 netbuf 结构指向的数据?pbuf
{
  LWIP_ERROR("netbuf_free: invalid buf", (buf != NULL), return;);
  if (buf->p != NULL) {
    pbuf_free(buf->p);
  }
  buf->p = buf->ptr = NULL;
}

/**
 * Let a netbuf reference existing (non-volatile) data.
 *
 * @param buf netbuf which should reference the data
 * @param dataptr pointer to the data to reference
 * @param size size of the data
 * @return ERR_OK if data is referenced
 *         ERR_MEM if data couldn't be referenced due to lack of memory
 */
err_t
netbuf_ref(struct netbuf *buf, const void *dataptr, u16_t size) //其功能与 netbuf_alloc 相似，区别在于它不会分配具体的数据区域，而只是为数据分配一个?pbuf?首部结构（包含各协议首部空间），并将?pbuf?的?payload?指针指向数据地址dataptr
{
  LWIP_ERROR("netbuf_ref: invalid buf", (buf != NULL), return ERR_ARG;);
  if (buf->p != NULL) {
    pbuf_free(buf->p);
  }
  buf->p = pbuf_alloc(PBUF_TRANSPORT, 0, PBUF_REF);
  if (buf->p == NULL) {
    buf->ptr = NULL;
    return ERR_MEM;
  }
  buf->p->payload = (void*)dataptr;
  buf->p->len = buf->p->tot_len = size;
  buf->ptr = buf->p;
  return ERR_OK;
}

/**
 * Chain one netbuf to another (@see pbuf_chain)
 *
 * @param head the first netbuf
 * @param tail netbuf to chain after head, freed by this function, may not be reference after returning
 */
void
netbuf_chain(struct netbuf *head, struct netbuf *tail)       //功能是将?tail?中的?pbuf?连接到?head?中的?pbuf?之后，调用此函数后，tail结构会被删除，用户不能再引用这个 netbuf
{
  LWIP_ERROR("netbuf_ref: invalid head", (head != NULL), return;);
  LWIP_ERROR("netbuf_chain: invalid tail", (tail != NULL), return;);
  pbuf_cat(head->p, tail->p);
  head->ptr = head->p;
  memp_free(MEMP_NETBUF, tail);
}

/**
 * Get the data pointer and length of the data inside a netbuf.
 *
 * @param buf netbuf to get the data from
 * @param dataptr pointer to a void pointer where to store the data pointer
 * @param len pointer to an u16_t where the length of the data is stored
 * @return ERR_OK if the information was retreived,
 *         ERR_BUF on error.
 */
err_t
netbuf_data(struct netbuf *buf, void **dataptr, u16_t *len)    //功能是将 netbuf 结构中 ptr?指针记录的 pbuf 数据起始地址填入 dataptr 中,同时将该?pbuf?中的数据长度填入到?len?中
{
  LWIP_ERROR("netbuf_data: invalid buf", (buf != NULL), return ERR_ARG;);
  LWIP_ERROR("netbuf_data: invalid dataptr", (dataptr != NULL), return ERR_ARG;);
  LWIP_ERROR("netbuf_data: invalid len", (len != NULL), return ERR_ARG;);

  if (buf->ptr == NULL) {
    return ERR_BUF;
  }
  *dataptr = buf->ptr->payload;
  *len = buf->ptr->len;
  return ERR_OK;
}

/**
 * Move the current data pointer of a packet buffer contained in a netbuf
 * to the next part.
 * The packet buffer itself is not modified.
 *
 * @param buf the netbuf to modify
 * @return -1 if there is no next part
 *         1  if moved to the next part but now there is no next part
 *         0  if moved to the next part and there are still more parts
 */
s8_t
netbuf_next(struct netbuf *buf)   // 其功能是将 netbuf 结构的 ptr?指针指向 pbuf 链表中的下一个 pbuf
{
  LWIP_ERROR("netbuf_free: invalid buf", (buf != NULL), return -1;);
  if (buf->ptr->next == NULL) {
    return -1;
  }
  buf->ptr = buf->ptr->next;
  if (buf->ptr->next == NULL) {
    return 1;
  }
  return 0;
}

/**
 * Move the current data pointer of a packet buffer contained in a netbuf
 * to the beginning of the packet.
 * The packet buffer itself is not modified.
 *
 * @param buf the netbuf to modify
 */
void
netbuf_first(struct netbuf *buf)     // 功能是将 netbuf 结构的 ptr 指针指向第一个 pbuf，
{
  LWIP_ERROR("netbuf_free: invalid buf", (buf != NULL), return;);
  buf->ptr = buf->p;
}

#endif /* LWIP_NETCONN */

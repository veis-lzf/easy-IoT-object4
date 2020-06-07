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
 * Author: Jani Monoses <jani@iv.ro>
 *
 */

#ifndef __LWIP_IP_FRAG_H__
#define __LWIP_IP_FRAG_H__

#include "lwip/opt.h"
#include "lwip/err.h"
#include "lwip/pbuf.h"
#include "lwip/netif.h"
#include "lwip/ip_addr.h"
#include "lwip/ip.h"

#ifdef __cplusplus
extern "C" {
#endif

#if IP_REASSEMBLY
/* The IP reassembly timer interval in milliseconds. */
#define IP_TMR_INTERVAL 1000

/* IP reassembly helper struct.
 * This is exported because memp needs to know the size.
 */
	//主要用于描述一份正在被重装的数据报
struct ip_reassdata {       		//为某个将被重装的数据报建立的节点信息数据结构，其内部包括目标?IP?地址、数据报编号等信息
  struct ip_reassdata *next;    //用于构建单向链表的指针
  struct pbuf *p;                //该数据报的数据链表
  struct ip_hdr iphdr;           //该数据报的 IP 首部
  u16_t datagram_len;            //已经收到的数据报长度
  u8_t flags;                    //是否收到最后一个分片
  u8_t timer;                     //设置超时间隔
};

void ip_reass_init(void);
void ip_reass_tmr(void);
struct pbuf * ip_reass(struct pbuf *p);
#endif /* IP_REASSEMBLY */

#if IP_FRAG
#if !IP_FRAG_USES_STATIC_BUF && !LWIP_NETIF_TX_SINGLE_PBUF
/** A custom pbuf that holds a reference to another pbuf, which is freed
 * when this custom pbuf is freed. This is used to create a custom PBUF_REF
 * that points into the original pbuf. */
struct pbuf_custom_ref {
  /** 'base class' */
  struct pbuf_custom pc;
  /** pointer to the original pbuf that is referenced */
  struct pbuf *original;
};
#endif /* !IP_FRAG_USES_STATIC_BUF && !LWIP_NETIF_TX_SINGLE_PBUF */

err_t ip_frag(struct pbuf *p, struct netif *netif, ip_addr_t *dest);
#endif /* IP_FRAG */

#ifdef __cplusplus
}
#endif

#endif /* __LWIP_IP_FRAG_H__ */

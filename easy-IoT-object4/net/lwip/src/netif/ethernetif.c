/**
 * @file
 * Ethernet Interface Skeleton
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

/*
 * This file is a skeleton for developing Ethernet network interface
 * drivers for lwIP. Add code to the low_level functions and do a
 * search-and-replace for the word "ethernetif" to replace it with
 * something that better describes your network interface.
 */
#include "lwip/opt.h"

#if 1 /* don't build, this is only a skeleton, see previous comment */

#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include <lwip/stats.h>
#include <lwip/snmp.h>
#include "netif/etharp.h"
#include "netif/ppp_oe.h"

#include "app.h"
#include "wifi.h"
#include "wpa.h"

#include "moal_cfg80211.h"


struct sk_buff *tx_skb = 0;


/* Define those to better describe your network interface. */
#define IFNAME0 'e'
#define IFNAME1 'n'

extern struct net_device *get_wifi_dev(void);
/**
 * Helper struct to hold private data used to operate your ethernet interface.
 * Keeping the ethernet address of the MAC in this struct is not necessary
 * as it is already kept in the struct netif.
 * But this is only an example, anyway...
 */
struct ethernetif {
  struct eth_addr *ethaddr;
  /* Add whatever per-interface state that is needed here. */
};     

/* Forward declarations. */
void  ethernetif_input(struct netif *netif, void *p_buf,int size);

/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */

//extern OS_EVENT  *pool_alloc_sem;

static void
low_level_init(struct netif *netif)                            //网卡初始化函数，用来完成网卡复位级参数初始化
{
   struct net_device *p_priv = get_wifi_dev();
   
  /* set MAC hardware address length */
  netif->hwaddr_len = ETHARP_HWADDR_LEN;      //设置网络接口结构?netif?中的网卡?MAC 地址字段     该宏在?etharp.h 中定义，值为 6

  if(!p_priv)
  {
  	p_err("priv not init");
	return;
  }

  /* set MAC hardware address */
  netif->hwaddr[0] = p_priv->dev_addr[0];
  netif->hwaddr[1] = p_priv->dev_addr[1];
  netif->hwaddr[2] = p_priv->dev_addr[2];
  netif->hwaddr[3] = p_priv->dev_addr[3];
  netif->hwaddr[4] = p_priv->dev_addr[4];
  netif->hwaddr[5] = p_priv->dev_addr[5];
  
  /* maximum transfer unit */
  netif->mtu = 1500;     //设置最大传输包大小
  
  /* device capabilities */
  /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
  netif->flags = NETIF_FLAG_BROADCAST |    //设置网络接口的属性字段，支持广播、多播、ARP、网络接口链路使能
  				NETIF_FLAG_ETHARP | 
  				NETIF_FLAG_LINK_UP | 
  				NETIF_FLAG_IGMP;

  /* Do whatever else is needed to initialize interface. */  
}

/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become availale since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */

DECLARE_MONITOR_ITEM("send packets cnt", send_packets_cnt) ;

void mac_data_xmit(struct pbuf *skb)
{
#ifdef DEBUG
	static int first_pkg = 1;
#endif
	struct pbuf *q;
	char *p802x_hdr;
	
	if(!is_hw_ok())
		return;

	tx_skb = alloc_skb(skb->tot_len + MLAN_MIN_DATA_HEADER_LEN + sizeof(mlan_buffer), 0);
	if(tx_skb == 0)
		return;

	skb_reserve(tx_skb, MLAN_MIN_DATA_HEADER_LEN + sizeof(mlan_buffer));
	
	p802x_hdr = (char*)tx_skb->data;
	for(q = skb; q != NULL; q = q->next) {
		
		memcpy(p802x_hdr, q->payload, q->len);
		p802x_hdr += q->len;
		
	}
	tx_skb->len = skb->tot_len;
	p802x_hdr = (char*)tx_skb->data;

	//dump_hex("s", tx_skb->data, tx_skb->len);

	INC_MONITOR_ITEM_VALUE(send_packets_cnt);

	if(wifi_dev && wifi_dev->netdev_ops)
		wifi_dev->netdev_ops->ndo_start_xmit((void*)tx_skb, wifi_dev);
	
	return;
}
	


static err_t
low_level_output(struct netif *netif, struct pbuf *p)       //网卡数据包发送函数    p：所有puf组成的一个链表
{

#if ETH_PAD_SIZE
  pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif

  mac_data_xmit(p);

#if ETH_PAD_SIZE
  pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif
  
  LINK_STATS_INC(link.xmit);   //协议栈内部的数据统计

  return ERR_OK;
}

/**
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return a pbuf filled with the received packet (including MAC header)
 *         NULL on memory error
 */
static struct pbuf *
low_level_input(struct netif *netif,void *p_buf,int size)          //网卡数据包接收函数
{
//  struct ethernetif *ethernetif = netif->state;
  struct pbuf *p, *q;
  u16_t len;
  int rem_len;

  /* Obtain the size of the packet and put it into the "len"
     variable. */
  len = size;

#if ETH_PAD_SIZE
  len += ETH_PAD_SIZE; /* allow room for Ethernet padding */
#endif

  /* We allocate a pbuf chain of pbufs from the pool. */
  p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);       //请求分配pbuf空间
  
  if (p != NULL && (p->tot_len >= len)) {

#if ETH_PAD_SIZE
    pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */         // 调整payload指针和长度字段以便为pbuf中的数据预置包头，常用于实现对pbuf预留空间的操作
#endif

    /* We iterate over the pbuf chain until we have read the entire
     * packet into the pbuf. */
     rem_len = p->tot_len;
    for(q = p; q != NULL; q = q->next) {
      /* Read enough bytes to fill this pbuf in the chain. The
       * available data in the pbuf is given by the q->len
       * variable.
       * This does not necessarily have to be a memcpy, you can also preallocate
       * pbufs for a DMA-enabled MAC and after receiving truncate it to the
       * actually received size. In this case, ensure the tot_len member of the
       * pbuf is the sum of the chained pbuf len members.
       */
    if(rem_len > 0)
	{
		memcpy(q->payload, (char*)p_buf + (p->tot_len - rem_len), q->len);
		rem_len -= q->len;
	}
	else
		p_err("low_level_input memcpy err\n");
 //     read data into(q->payload, q->len);
    }
 //   acknowledge that packet has been read();

#if ETH_PAD_SIZE
    pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

    LINK_STATS_INC(link.recv);
  } else {

    LINK_STATS_INC(link.memerr);
    LINK_STATS_INC(link.drop);
  }

  return p;     //返回接收到数据包的链表（指针）
}

/**
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
#define ETH_FRAME_LEN	1514

DECLARE_MONITOR_ITEM("recv packets cnt", recv_packets_cnt) ;


//ethernetif_input的主要作用是调用网卡数据包接收函数low_level_input 从网卡处读取一个数据包，
//然后解析该数据包的类型（ARP包或IP包），最后将该数据包递交给相应的上层

void ethernetif_input(struct netif *netif,void *p_buf,int size)               
{
//  struct ethernetif *ethernetif;
  struct eth_hdr *ethhdr;
  struct pbuf *p;

  if(size > ETH_FRAME_LEN)
  {
  	p_err("ethernetif_input to long:%d\n",size);
	return;
  }
  p = low_level_input(netif, p_buf, size);
  if (p == NULL) {
	return;
  }

  /* points to packet payload, which starts with an Ethernet header */
  ethhdr = p->payload;

  switch (htons(ethhdr->type)) {
  /* IP or ARP packet? */
  case ETHTYPE_IP:
  case ETHTYPE_ARP:
  case ETH_P_EAPOL:
#if PPPOE_SUPPORT
  /* PPPoE packet? */
  case ETHTYPE_PPPOEDISC:
  case ETHTYPE_PPPOE:
#endif /* PPPOE_SUPPORT */
    /* full packet send to tcpip_thread to process */
    INC_MONITOR_ITEM_VALUE(recv_packets_cnt);
    if (netif->input(p, netif)!=ERR_OK)       //已经将?netif->input?函数注册为?ethernet_input?函数，所以这个函数将被调用    ethernet_input这函数在etharp.c里面
     { LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
       pbuf_free(p);
       p = NULL;
     }
    break;

  default:
    pbuf_free(p);
    p = NULL;
    break;
  }
}

/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */

//ethernetif_init 是上层在管理网络接口结构 netif 时会调用的函数。
//该函数主要完成 netif 结构中某些字段的初始化，并最终调用low_level_init?完成网卡的初始化

err_t
ethernetif_init(struct netif *netif)
{
  struct ethernetif *ethernetif;

  LWIP_ASSERT("netif != NULL", (netif != NULL));
    
  ethernetif = mem_malloc(sizeof(struct ethernetif));
  if (ethernetif == NULL) {
    LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_init: out of memory\n"));
    return ERR_MEM;
  }

#if LWIP_NETIF_HOSTNAME
  /* Initialize interface hostname */
  netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

  /*
   * Initialize the snmp variables and counters inside the struct netif.
   * The last argument should be replaced with your link speed, in units
   * of bits per second.
   */
  NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, LINK_SPEED_OF_YOUR_NETIF_IN_BPS);

  netif->state = ethernetif;
  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;
  /* We directly use etharp_output() here to save a function call.
   * You can instead declare your own function an call etharp_output()
   * from it if you have to do some checks before sending (e.g. if link
   * is available...) */
  netif->output = etharp_output;               //   output实际指向etharp_output，由IP层调用，参数是：netif，IP包，目的IP地址
  netif->linkoutput = low_level_output;        //    inkoutput实际指向low_level_output，由ARP层调用，参数是：netif，ETH包    linkoutput是在已知目的MAC地址时调用，而output则相反。
  
  ethernetif->ethaddr = (struct eth_addr *)&(netif->hwaddr[0]);
  
  /* initialize the hardware */
  low_level_init(netif);

  return ERR_OK;
}

#endif /* 0 */

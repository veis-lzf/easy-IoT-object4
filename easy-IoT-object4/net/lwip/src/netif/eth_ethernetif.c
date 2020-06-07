#define DEBUG
#include "lwip/opt.h"

#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include <lwip/stats.h>
#include <lwip/snmp.h>
#include "netif/etharp.h"
#include "netif/ppp_oe.h"

#include "drivers.h"
#include "app.h"
#include "api.h"
#include "wifi.h"
#include "wpa.h"

#include "lan8720.h"
#include "web_cfg.h"

struct ethernetif {
  struct eth_addr *ethaddr;
  uint8_t link_state;
  /* Add whatever per-interface state that is needed here. */
};
/* Ethernet Rx & Tx DMA Descriptors */
//extern ETH_DMADESCTypeDef  DMARxDscrTab[ETH_RXBUFNB], DMATxDscrTab[ETH_TXBUFNB];

/* Ethernet Driver Receive buffers  */
//extern uint8_t Rx_Buff[ETH_RXBUFNB][ETH_RX_BUF_SIZE]; 

/* Ethernet Driver Transmit buffers */
//extern uint8_t Tx_Buff[ETH_TXBUFNB][ETH_TX_BUF_SIZE]; 

extern ETH_DMADESCTypeDef *DMARxDscrTab;	//以太网DMA接收描述符数据结构体指针
extern ETH_DMADESCTypeDef *DMATxDscrTab;	//以太网DMA发送描述符数据结构体指针 
extern uint8_t *Rx_Buff; 					//以太网底层驱动接收buffers指针 
extern uint8_t *Tx_Buff; 					//以太网底层驱动发送buffers指针

extern int auto_get_ip(struct netif *p_netif);

/* Global pointers to track current transmit and receive descriptors */
extern ETH_DMADESCTypeDef  *DMATxDescToSet;
extern ETH_DMADESCTypeDef  *DMARxDescToGet;

/* Global pointer for last received frame infos */
extern ETH_DMA_Rx_Frame_infos *DMA_RX_FRAME_infos;
/**
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return a pbuf filled with the received packet (including MAC header)
 *         NULL on memory error
 */
static struct pbuf *
eth_low_level_input(struct netif *netif,void *p_buf,int size)
{
  struct pbuf *p, *q;
  u16_t len;
  int l =0;
  FrameTypeDef frame;
  u8 *buffer;
  uint32_t i=0;
  __IO ETH_DMADESCTypeDef *DMARxNextDesc;
  
  
  p = NULL;
  
  /* get received frame */
  frame = ETH_Get_Received_Frame();
  
  /* Obtain the size of the packet and put it into the "len" variable. */
  len = frame.length;
  buffer = (u8 *)frame.buffer;
	
//	dump_hex("", buffer, len);
  
  /* We allocate a pbuf chain of pbufs from the Lwip buffer pool */
  p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
  
  /* copy received frame to pbuf chain */
  if (p != NULL)
  {
    for (q = p; q != NULL; q = q->next)
    {
      memcpy((u8_t*)q->payload, (u8_t*)&buffer[l], q->len);
      l = l + q->len;
    }    
  }
  
  /* Release descriptors to DMA */
  /* Check if frame with multiple DMA buffer segments */
  if (DMA_RX_FRAME_infos->Seg_Count > 1)
  {
    DMARxNextDesc = DMA_RX_FRAME_infos->FS_Rx_Desc;
  }
  else
  {
    DMARxNextDesc = frame.descriptor;
  }
  
  /* Set Own bit in Rx descriptors: gives the buffers back to DMA */
  for (i=0; i<DMA_RX_FRAME_infos->Seg_Count; i++)
  {  
    DMARxNextDesc->Status = ETH_DMARxDesc_OWN;
    DMARxNextDesc = (ETH_DMADESCTypeDef *)(DMARxNextDesc->Buffer2NextDescAddr);
  }
  
  /* Clear Segment_Count */
  DMA_RX_FRAME_infos->Seg_Count =0;
  
  /* When Rx Buffer unavailable flag is set: clear it and resume reception */
  if ((ETH->DMASR & ETH_DMASR_RBUS) != (u32)RESET)  
  {
    /* Clear RBUS ETHERNET DMA flag */
    ETH->DMASR = ETH_DMASR_RBUS;
    /* Resume DMA reception */
    ETH->DMARPDR = 0;
  }
  return p;
}


void eth_ethernetif_input(struct netif *netif,void *p_buf,int size)
{
//  struct ethernetif *ethernetif;
  struct eth_hdr *ethhdr;
  struct pbuf *p;

  if(size > ETH_FRAME_LEN)
  {
  	p_err("ethernetif_input to long:%d\n",size);
	return;
  }
  p = eth_low_level_input(netif, p_buf, size);
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
    if (netif->input(p, netif)!=ERR_OK)
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
static void
eth_low_level_init(struct netif *netif)
{
//   int i;
  /* set MAC hardware address length */
  netif->hwaddr_len = ETHARP_HWADDR_LEN;
  
  ETH_MACAddressConfig(ETH_MAC_Address0, netif->hwaddr); 
  /* maximum transfer unit */
  netif->mtu = 1500;
  
  /* device capabilities */
  /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
  netif->flags = NETIF_FLAG_BROADCAST | 
  				NETIF_FLAG_ETHARP | 
  				NETIF_FLAG_LINK_UP | 
  				NETIF_FLAG_IGMP;
  /* Initialize Tx Descriptors list: Chain Mode */
  //ETH_DMATxDescChainInit(DMATxDscrTab, &Tx_Buff[0][0], ETH_TXBUFNB);
  /* Initialize Rx Descriptors list: Chain Mode  */
 // ETH_DMARxDescChainInit(DMARxDscrTab, &Rx_Buff[0][0], ETH_RXBUFNB);
  ETH_DMATxDescChainInit(DMATxDscrTab, Tx_Buff, ETH_TXBUFNB);
  ETH_DMARxDescChainInit(DMARxDscrTab, Rx_Buff, ETH_RXBUFNB);

#ifdef CHECKSUM_BY_HARDWARE
  /* Enable the TCP, UDP and ICMP checksum insertion for the Tx frames */
  for(i=0; i<ETH_TXBUFNB; i++)
    {
      ETH_DMATxDescChecksumInsertionConfig(&DMATxDscrTab[i], ETH_DMATxDesc_ChecksumTCPUDPICMPFull);
    }
#endif
  ETH_Start();
  /* Do whatever else is needed to initialize interface. */  
}


static err_t
eth_low_level_output(struct netif *netif, struct pbuf *p)
{

  struct pbuf *q;
  int framelength = 0;
  u8 *buffer =  (u8 *)(DMATxDescToSet->Buffer1Addr);
  
  /* copy frame from pbufs to driver buffers */
  for(q = p; q != NULL; q = q->next) 
  {
    memcpy((u8_t*)&buffer[framelength], q->payload, q->len);
	framelength = framelength + q->len;
  }
  
  /* Note: padding and CRC for transmitted frame 
     are automatically inserted by DMA */

  /* Prepare transmit descriptors to give to DMA*/ 
  ETH_Prepare_Transmit_Descriptors(framelength);

  return ERR_OK;
}

wait_event_t eth_int_event = 0;
char eth_int_pending = 0;

void eth_int_thread(void *pdata)    //以太网的任务
{
	int rval, last_link_status = 0;
	eth_int_event = init_event();

	rval=ETH_MACDMA_Config();

	if(rval != ETH_SUCCESS)
		goto end;

	rval = lwip_eth_netif_init();
	if(rval != 0)
		goto end;

	if(web_cfg.eth_ip.auto_get){
	     auto_get_ip(p_eth_netif);
	}

	while(1)
	{
		rval = wait_event_timeout(eth_int_event, 1000);
		if(rval == WAIT_EVENT_TIMEOUT)
		{
			struct ethernetif *eth_state = (struct ethernetif *)p_eth_netif->state;
			uint16_t status = ETH_ReadPHYRegister(LAN8720_PHY_ADDRESS, PHY_BSR);
			if(!(status & PHY_Linked_Status) && (last_link_status == 1))
			{
				last_link_status = 0;
				if(eth_state)
					eth_state->link_state = 0;
				p_dbg("eth link lost");
//				send_work_event(ETH_LINK_LOST_EVENT);
				
			}else if((status & PHY_Linked_Status) && (last_link_status == 0)){
				last_link_status = 1;
				if(eth_state)
					eth_state->link_state = 1;
//				p_dbg("eth link ok");
//				send_work_event(ETH_LINK_OK_EVENT);
				
			}
		}

		while(ETH_CheckFrameReceived())
   		{
			if(p_eth_netif)
			{
				eth_ethernetif_input(p_eth_netif, NULL, 0);
			}
    	}
		/* Clear the Eth DMA Rx IT pending bits */
		ETH_DMAClearITPendingBit(ETH_DMA_IT_R);
		ETH_DMAClearITPendingBit(ETH_DMA_IT_NIS);
		eth_int_pending = 0;
		NVIC_EnableIRQ(ETH_IRQn);
	}
end:
	p_err_fun;
	thread_exit(thread_myself());
}


err_t
eth_ethernetif_init(struct netif *netif)
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
  netif->name[0] = 'e';
  netif->name[1] = 't';
  /* We directly use etharp_output() here to save a function call.
   * You can instead declare your own function an call etharp_output()
   * from it if you have to do some checks before sending (e.g. if link
   * is available...) */
  netif->output = etharp_output;
  netif->linkoutput = eth_low_level_output;
  
  ethernetif->ethaddr = (struct eth_addr *)&(netif->hwaddr[0]);
  
  /* initialize the hardware */
  eth_low_level_init(netif);

  return ERR_OK;
}

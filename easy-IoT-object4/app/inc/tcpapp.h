#ifndef _TCP_H
#define _TCP_H

struct ret_connect_stat
{
	uint32_t local_addr;
	uint32_t remote_addr;
	uint16_t local_port;
	uint16_t remote_port;
	uint8_t socket_num;
	uint8_t type; //tcp udp
};
extern struct netif *p_netif, *p_eth_netif;
int my_lwip_init(void);
int netif_addr_init(void);
int init_lwip(void);
int socket_link(char *ip, uint16_t port);
//int auto_get_ip(struct netif *p_netif);
int get_host_by_name(char *hostname, uint32_t *addr);
int tcp_client_create(char *ip, uint16_t port);
int udp_client_create(char *ip, uint16_t port);
int udp_add_membership(char *ip, uint16_t port);
int udp_create_server(uint16_t port);
int udp_data_send(int socket, void *data, int len, uint16_t remote_port, uint32_t remote_addr);
extern struct netif *p_netif;
int close_socket(uint32_t socket_num);
int lwip_netif_init(void);
int send_data(int socket, uint8_t *buff, int size);
int lwip_eth_netif_init(void);
uint32_t get_gw_addr(struct netif *p_netif);
#endif


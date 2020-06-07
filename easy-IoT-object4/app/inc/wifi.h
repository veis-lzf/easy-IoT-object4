#ifndef _WIFI_H_
#define _WIFI_H_

typedef enum
{
	KEY_NONE = 0, KEY_WEP, KEY_WPA, KEY_WPA2, KEY_MAX_VALUE
} SECURITY_TYPE;

extern struct net_device *wifi_dev;

int wifi_set_mode(int type);
int wifi_client_add_key(int index, char *key);
	
int is_wifi_connected(void);
int wifi_disconnect(void);

int wifi_connect( char *essid, char *key);

int wifi_get_mac(unsigned char *mac_addr);
int init_wifi(void);
int wifi_set_channel(int channel);
//int wifi_get_stats(struct station_info *sinfo);
int wifi_ap_cfg(char *name, char *key, int key_type, int channel, int max_client);

//int wifi_scan(void (*scan_result_callback)(struct scan_result_data *res_data), 
//		char *essid); 
int is_hw_ok(void);
int create_mac(unsigned char *mac);
int wifi_stop_ap(void);
int wifi_join_adhoc(char *ssid, char *key, int ch);
int wifi_leave_adhoc(void);
int wifi_power_cfg(uint8_t power_save_level);
int wifi_set_multicast(uint32_t flags);
int wifi_set_promisc(void);
int wifi_get_mode(void);
#endif

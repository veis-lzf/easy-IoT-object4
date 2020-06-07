#define DEBUG
#include "debug.h"
#include "drivers.h"
#include "app.h"
#include "api.h"


#include "cfg80211.h"
#include "defs.h"
#include "type.h"
#include "types.h"

#include "wifi.h"

#include "moal_main.h"
#include "moal_uap.h"
#include "moal_uap_priv.h"
#include "mlan_ioctl.h"

#include "lwip/netif.h"

#ifdef UAP_SUPPORT
extern int wifi_get_sta_list(mlan_ds_sta_list *p_list);
#endif
extern int wifi_get_stats(uint8_t *mac, struct station_info *sinfo);
extern int wifi_scan(void (*scan_result_callback)(struct scan_result_data *res_data), 
		char *essid); 


const char *wpa_proto[4] = {
	"NONE",
	"WPA",
	"WPA2/RSN",
	"OPEN/SHARE"
};

void print_scan_result(struct scan_result_data *dat)
{
	char *cipher = "";
	char *mode;
	
	char essid_name[33];

	if(dat->cipher & WPA_CIPHER_NONE)
		cipher = "NONE";
	else
	{
		if(dat->cipher & WPA_CIPHER_WEP40)
			cipher = "WEP40";
		if(dat->cipher & WPA_CIPHER_WEP104)
			cipher = "WEP104";
		if(dat->cipher & WPA_CIPHER_TKIP)
			cipher = "TKIP";
		if(dat->cipher & WPA_CIPHER_CCMP)
			cipher = "CCMP";
	}	

	if(dat->mode == MODE_ADHOC)
		mode = "AD-HOC";
	else if(dat->mode == MODE_AP)
		mode = "MASTER";
	else
		mode = "unkown";
	
	memcpy(essid_name,dat->essid,32);
	essid_name[32] = 0;
	if(dat->essid_len == 0)
	{
		essid_name[0] =0;
	}
	if(dat->essid_len < 32)
	{
		essid_name[dat->essid_len] =0;
	}
	
	p_dbg("\r\nAP:%d",dat->num);	
	p_dbg("	信号强度:%ddbm",dat->rssi);
	p_dbg("	认证类型:%s",	(dat->proto <= 3)?wpa_proto[dat->proto]:"unkown");	
	p_dbg("	加密模式:%s", cipher);
	p_dbg("	频道:%d",dat->channel);	
	p_dbg("	频率:%dMHz",dat->freq);	
	p_dbg("	模式:%s", mode);	
	p_dbg("	ssid:%s",essid_name);
	dump_hex("	bssid",dat->bssid,6);
}


void scan_result_fun(struct scan_result_data *res_data)
{
	if(res_data)
		print_scan_result(res_data);
}

char essid[] = {0xe9,0xad,0x85,0xe8,0x93,0x9d,0x20,0x6e,0x6f,0x74,0x65};

/*
*测试扫描,命令'1'
*/
void test_scan()
{
	p_dbg_enter;
	wifi_scan(scan_result_fun, "");
	p_dbg_exit;
}

/**
 *测试WIFI连接,命令'2'
 *连接到名字为"TP-LINK_2AA6”的路由器，加密模式和频道自动适应
 *密码长度在WPA或WPA2模式下8 <= len <= 64;在WEP模式下必须为5或13
 */
void test_wifi_connect()
{
	char essid[] = "TP-LINK_2AA6";
	char *password = "zsl540535";
	p_dbg_enter;

	p_dbg("Connect to STA:%s, PWD:%s", essid, password);

	wifi_set_mode(MODE_STATION);

	wifi_connect(essid, password);

	if (is_wifi_connected())
	{
		p_dbg("wifi connect ok");
		netif_set_down(p_netif);
		netif_set_up(p_netif);
	}

	p_dbg_exit;

}


/*
 *建立AP,命令'4'
 *
 */
void test_create_ap()
{
	char *essid = "lwip_ap";
	char *password = "12345678";
	p_dbg_enter;
	p_dbg("建立ap:%s, 密码:%s", essid, password);

	wifi_set_mode(MODE_AP);

	wifi_ap_cfg(essid, password, KEY_WPA2, 6, 4);

	p_dbg_exit;
}

/*
 *测试断开WIFI连接,命令'3'
 *
 */
void test_wifi_disconnect()
{
	p_dbg_enter;
	wifi_disconnect();
	p_dbg_exit;
}


/*
 *测试加入adhoc网络,命令'q'
 *
 */
void test_wifi_join_adhoc()
{
	p_dbg_enter;
	wifi_set_mode(MODE_STATION);  //ADHOC工作在sta模式下
	wifi_set_mode(MODE_ADHOC);
	wifi_join_adhoc("xrf_adhoc", "1234567890abc", 6);
	p_dbg_exit;
}

/*
 *测试推出adhoc网络,命令'r'
 *
 */
void test_wifi_leave_adhoc()
{
	p_dbg_enter;
	wifi_leave_adhoc();
	p_dbg_exit;
}



/*
 *测试获取WIFI连接信息,因可能有多个客户端连接,所以需要指定其地址
 *这里从test_get_station_list里面取最后一个mac地址作为测试
 *所以执行此测试之前请先执行test_get_station_list
 *,命令'f'
 */
uint8_t sta_mac[8] = {0,0,0,0,0,0};
void test_wifi_get_stats()
{
	int ret;
	struct station_info sinfo;
	
	ret = wifi_get_stats(sta_mac, &sinfo);

	if(ret == 0)
		p_dbg("wifi stats, rssi:%d", sinfo.signal);
	else
		p_err("test_wifi_get_stats err");
}

/*
 *关闭AP,命令'n'
 *
 */

void test_stop_ap()
{
	p_dbg_enter;
	wifi_stop_ap();
	p_dbg_exit;
}

/*
 *测试获取ap模式下已连接的客户端信息,命令'o'
 *
 */
void test_get_station_list()
{
#ifdef UAP_SUPPORT
	int i;
	int rssi = 0xffffff00;
	mlan_ds_sta_list *p_list;
	p_dbg_enter;
	p_list = (mlan_ds_sta_list *)mem_calloc(sizeof(mlan_ds_sta_list), 1);
	if(p_list){
		wifi_get_sta_list(p_list);

		for(i = 0; i < p_list->sta_count; i++)
		{
			p_dbg("sta %d", i);
			rssi = rssi | p_list->info[i].rssi;
			p_dbg("	rssi %d", rssi);
			dump_hex("	mac", p_list->info[i].mac_address, 6);

			memcpy(sta_mac, p_list->info[i].mac_address, 6);
		}
	}
#endif
	p_dbg_exit;
}


void test_power_save_enable()
{
	p_dbg("enter power save");
	wifi_power_cfg(2);
}

void test_power_save_disable()
{
	p_dbg("exit power save");
	wifi_power_cfg(0);
}



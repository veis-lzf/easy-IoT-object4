#define DEBUG

#include "drivers.h"
#include "app.h"
#include "api.h"

#include "lwip\sockets.h"
#include "lwip\netif.h"
#include "lwip\dns.h"
#include "lwip\api.h"
#include "lwip\tcp.h"

#include "gui.h"
#include "TEXT.h"
#include "BUTTON.h"
#include "EDIT.h"

#define STR_TMP_SIZE 64
char str_tmp[STR_TMP_SIZE];

static char *sys_info = "";
EDIT_Handle h_time, h_temp, h_hum, h_wifi_ip, h_eth_ip, h_cpu_used, h_mem_used;
TEXT_Handle h_txt;

extern int cpu_used;

void gui_show_info(char *str)
{
	sys_info = str;
}

void show_system_stat()
{
	snprintf(str_tmp, STR_TMP_SIZE, "%d-%d-%d %d:%d:%d", sensers_pkg.sensers_stat.year + 2000, sensers_pkg.sensers_stat.mon, sensers_pkg.sensers_stat.date,
		sensers_pkg.sensers_stat.hour, sensers_pkg.sensers_stat.min, sensers_pkg.sensers_stat.sec);		
	EDIT_SetText(h_time,  str_tmp);

	snprintf(str_tmp, STR_TMP_SIZE, "%d", sensers_pkg.sensers_stat.temperature);
	EDIT_SetText(h_temp,  str_tmp);

	snprintf(str_tmp, STR_TMP_SIZE, "%d", sensers_pkg.sensers_stat.humidity);
	EDIT_SetText(h_hum,  str_tmp);

	snprintf(str_tmp, STR_TMP_SIZE, "%d/100", cpu_used);
	EDIT_SetText(h_cpu_used,  str_tmp);

	snprintf(str_tmp, STR_TMP_SIZE, "%d/%dk", mem_get_free()/1024, mem_get_size()/1024);
	EDIT_SetText(h_mem_used,  str_tmp);

	if(p_netif)
		snprintf(str_tmp, STR_TMP_SIZE, "%d.%d.%d.%d", ip4_addr1(&p_netif->ip_addr.addr), ip4_addr2(&p_netif->ip_addr.addr), ip4_addr3(&p_netif->ip_addr.addr), ip4_addr4(&p_netif->ip_addr.addr));
	else
		snprintf(str_tmp, STR_TMP_SIZE, "invalid");
	
	EDIT_SetText(h_wifi_ip,  str_tmp);

	if(p_eth_netif)
		snprintf(str_tmp, STR_TMP_SIZE, "%d.%d.%d.%d", ip4_addr1(&p_eth_netif->ip_addr.addr), ip4_addr2(&p_eth_netif->ip_addr.addr), ip4_addr3(&p_eth_netif->ip_addr.addr), ip4_addr4(&p_eth_netif->ip_addr.addr));
	else
		snprintf(str_tmp, STR_TMP_SIZE, "invalid");
	EDIT_SetText(h_eth_ip,  str_tmp);

	TEXT_SetText(h_txt, sys_info);
}



#define LINE_HIGH	14
#define LINE_PAD		2
void gui_task(void *arg)
{
	//TEXT_Handle h_txt;
	//BUTTON_Handle h_bt;
	
	GUI_Init();
	
	GUI_SetBkColor(GUI_BLUE);
	GUI_SetColor(GUI_WHITE);
	GUI_Clear();
	
	GUI_DispStringAt("easynetworking.cn",0,0);

	h_time = EDIT_Create(0, LINE_HIGH, 128, LINE_HIGH, 0, 32, WM_CF_SHOW);
	
	GUI_DispStringAt("temp",0, (LINE_HIGH + LINE_PAD)*2 + LINE_PAD);
	h_temp = EDIT_Create(24, (LINE_HIGH + LINE_PAD)*2, 64 - 24, LINE_HIGH, 0, 32, WM_CF_SHOW);

	GUI_DispStringAt("humi",64, (LINE_HIGH + LINE_PAD)*2 + LINE_PAD);
	h_hum = EDIT_Create(64 + 24, (LINE_HIGH + LINE_PAD)*2, 64 - 24, LINE_HIGH, 0, 32, WM_CF_SHOW);

	GUI_DispStringAt("cpu",0, (LINE_HIGH + LINE_PAD)*3 + LINE_PAD);
	h_cpu_used= EDIT_Create(24, (LINE_HIGH + LINE_PAD)*3, 64 - 24, LINE_HIGH, 0, 32, WM_CF_SHOW);

	GUI_DispStringAt("mem",64, (LINE_HIGH + LINE_PAD)*3 + LINE_PAD);
	h_mem_used= EDIT_Create(64 + 16, (LINE_HIGH + LINE_PAD)*3, 64 - 16, LINE_HIGH, 0, 32, WM_CF_SHOW);

	GUI_DispStringAt("WIFI",0,  (LINE_HIGH + LINE_PAD)*4+ LINE_PAD);
	h_wifi_ip = EDIT_Create(24, (LINE_HIGH + LINE_PAD)*4, 128 - 24, LINE_HIGH, 1, 32, WM_CF_SHOW);

	GUI_DispStringAt("ETH",0,  (LINE_HIGH + LINE_PAD)*5 + LINE_PAD);
	h_eth_ip = EDIT_Create(24, (LINE_HIGH + LINE_PAD)*5, 128 - 24, LINE_HIGH, 2, 32, WM_CF_SHOW);

	h_txt = TEXT_Create(0, (LINE_HIGH + LINE_PAD)*6, 128, 32, 0, WM_CF_SHOW, "", GUI_TA_LEFT|GUI_TA_TOP);
	TEXT_SetBkColor(h_txt, 0xffffff);

	while(1)
	{
		sleep(1000);
		show_system_stat();
		GUI_Exec();
	}
}

void test_gui()
{
	thread_create(gui_task, 0, TASK_GUI_APP_PRIO, 0, TASK_GUI_APP_STACK_SIZE, "gui app thread");
}

#ifndef _WEB_CFG_H__
#define _WEB_CFG_H__

#include "app_cfg.h"
#include "work.h"

#define WEB_CFG_ADDR				0
#define REMOTE_SERVER_NUM 		1

#define WIFI_NAME_LEN 32
#define MQTT_NAME_LEN 64
#define MUSIC_NAME_LEN 96


struct WEB_CFG_DEVINFO
{
	char	devname[WIFI_NAME_LEN];
	char	version[WIFI_NAME_LEN];
}__packed;
struct WEB_CFG_LINK
{
	char	auth;
	char	mode;
	char	channel;
    char  enable;
	char	essid[WIFI_NAME_LEN];
	char	key[WIFI_NAME_LEN];
}__packed;

struct WEB_CFG_IP{
	unsigned int	ip;
	unsigned int	gw;
	unsigned int	msk;
	unsigned int	dns;
	unsigned int	auto_get;
}__packed;

struct WEB_CFG_SERVER{
	unsigned int	tcpudp;
	unsigned int	port;
	unsigned int	open;
}__packed;

struct WEB_CFG_MQTT{
	char			server_name[MQTT_NAME_LEN];
	unsigned int	tcpudp;
	unsigned int	server_ip;
	unsigned int	port;
	unsigned int	qos_sub;
	unsigned int	qos_pub;
	char			usr_name[MQTT_NAME_LEN];
	char			password[MQTT_NAME_LEN];
}__packed;

struct WEB_CFG_AUDIO{
	unsigned int	audio_fre;
	unsigned int	audio_qua;
	unsigned int	audio_alarm;
}__packed;
struct WEB_CFG_MUSIC{
	char	start;
	char	pause;
	char	play[MUSIC_NAME_LEN];
}__packed;

struct WEB_CFG_IOCTL{
        char    relay;
        char    led1;
        char    led2;
        char    lamp;
}__packed;

struct WEB_CFG_SENSERS{
        unsigned int    temperature;
        unsigned int    humidity;
        unsigned int    illuminant;
        unsigned int    smog;
}__packed;

typedef struct  _WEB_CFG
{
	struct WEB_CFG_DEVINFO     devinfo;
	struct WEB_CFG_LINK		ap;
	struct WEB_CFG_LINK		sta;
	struct WEB_CFG_IP		wifi_ip;
	struct WEB_CFG_IP		eth_ip;
	struct WEB_CFG_SERVER	server;
	struct WEB_CFG_MQTT		remote[REMOTE_SERVER_NUM];
	struct WEB_CFG_AUDIO	audio;
	struct iot_work_config  work_cfg;
	struct WEB_CFG_IOCTL io_ctl;
	struct WEB_CFG_SENSERS sensers;
	struct WEB_CFG_MUSIC music;
	char	password[31];
	uint8_t check_sum;
}WEB_CFG;

#define SAVE_WEB_CFG_SIZE(x)	(sizeof(WEB_CFG))
#define SAVE_WEB_CFG_DATA_SIZE(x)	(sizeof(WEB_CFG)-1) 

WEB_CFG *get_web_cfg(void);
void default_web_cfg(WEB_CFG *cfg);

#if SUPPORT_WEBSERVER
void web_server_init(void);
#endif

int load_web_cfg(WEB_CFG *cfg);
int save_web_cfg(WEB_CFG *cfg);
int read_web_cfg(WEB_CFG *cfg);

extern WEB_CFG web_cfg;

#endif


#define DEBUG


#include <stdio.h>
#include <string.h>
#include "web_cfg.h"
#include "lwip/inet.h"

#include "drivers.h"
#include "app.h"
#include "api.h"


#include "cfg80211.h"
#include "defs.h"
#include "type.h"
#include "types.h"

#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "wpa.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"

#include "usr_cfg.h"


WEB_CFG web_cfg;





#if SUPPORT_WEBSERVER
void web_server_init(void)
{
}

#endif

uint8_t cal_buff_check_sum(uint8_t *buff, int size)
{
    int i;
    uint8_t sum = 0;
    
    for(i = 0; i < size; i++)
        sum ^= buff[i];

    return sum;
}

WEB_CFG *get_web_cfg(void)
{
    return &web_cfg;
}

int verify_web_cfg(WEB_CFG *cfg)
{   
    if(cfg->check_sum == 0xff)
    {
        printf("verify_web_cfg: checksume not valid\n");
        return 0;
    }
    return (cfg->check_sum == cal_buff_check_sum((uint8_t*)cfg, SAVE_WEB_CFG_DATA_SIZE(cfg)));
}

void reset_ctl_cfg(WEB_CFG *cfg)
{
    cfg->io_ctl.lamp = 50;
    cfg->io_ctl.led1 = 0;
    cfg->io_ctl.led2 = 0;
    cfg->io_ctl.relay = 0;

    cfg->music.pause = 0;
    cfg->music.start = 0;
    strcpy(cfg->music.play, "not started.mp3");

}

#define str_ap_n  "智能识别器"  
#define str_ap_p  "123456789"  
#define str_sta_n  "veis"  
#define str_sta_p  "123456789"  
 

extern char * str_ap_name;
extern char *str_ap_pwd;
extern char *str_sta_name;
extern char *str_sta_pwd;


void first_web_cfg(WEB_CFG *cfg)   //第一次登陆设置
{
    str_ap_name=str_ap_n;
    str_ap_pwd=str_ap_p;
    str_sta_name=str_sta_n;
    str_sta_pwd=str_sta_p;
}


void default_web_cfg(WEB_CFG *cfg)
{     p_dbg_enter;

     memset(cfg, 0, sizeof(WEB_CFG));
 
  
    strcpy(cfg->devinfo.devname, "IoT Unified Platform");
    strcpy(cfg->devinfo.version, "V2 with LwIP2.0.3");
    
    cfg->ap.channel = 6;
    cfg->ap.enable = 1;
    cfg->ap.mode= MODE_AP;
//    strcpy(cfg->ap.essid, str_ap_name);
//    strcpy(cfg->ap.key, str_ap_pwd);
     strcpy(cfg->ap.essid, "智能识别器");
    strcpy(cfg->ap.key, "123456789");
    printf("\r\n 账号=%s\r\n",cfg->ap.essid);
    printf("\r\n 密码=%s\r\n",cfg->ap.key);

    cfg->sta.channel = 6;
    cfg->sta.enable = 0;
    cfg->sta.mode= MODE_STATION;
    strcpy(cfg->sta.essid, str_sta_name );
    strcpy(cfg->sta.key,  str_sta_pwd);
    printf("\r\n 账号=%s\r\n",cfg->sta.essid);
    printf("\r\n 密码=%s\r\n",cfg->sta.key);

    cfg->wifi_ip.ip = inet_addr("192.168.0.37");
    cfg->wifi_ip.gw = inet_addr("192.168.0.1");
    cfg->wifi_ip.msk= inet_addr("255.255.255.0");
    cfg->wifi_ip.dns = inet_addr("8.8.8.8");
    cfg->wifi_ip.auto_get = 1;  //wifi链接后自动获取一次IP

    cfg->eth_ip.ip = inet_addr("192.168.0.11");
    cfg->eth_ip.gw = inet_addr("192.168.0.1");
    cfg->eth_ip.msk= inet_addr("255.255.255.0");
    cfg->eth_ip.dns = inet_addr("8.8.8.8");
    cfg->eth_ip.auto_get = 1;   //以太网，网线插入之后自动获取一次IP

    cfg->audio.audio_fre= AUDIO_SAMPLERATE16000;          //波特率
    cfg->audio.audio_qua = SPEEX_DEFAULT_QUALITY;        //音色
    cfg->audio.audio_alarm = 1000;                       //声音的幅值
    
    strcpy(cfg->remote[0].server_name, "www.easynetworking.cn");
    cfg->remote[0].server_ip = inet_addr("121.42.188.97");
    cfg->remote[0].port = 1883;
    
    cfg->remote[0].qos_sub = 0;
    cfg->remote[0].qos_pub = 0;

    strcpy(cfg->remote[0].usr_name, "test");
    strcpy(cfg->remote[0].password, "test");

    //默认的订阅topic就是"system"用于接收系统消息
    //memset(cfg->work_cfg.bond[0].mac, '0', 12);
    //strcpy(cfg->work_cfg.bond[0].sn, "system");

    //default port for camera server
    cfg->server.port = 4800;

    reset_ctl_cfg(cfg);
}

int save_web_cfg(WEB_CFG *cfg)
{
    int ret;
//  uint32_t *p_buf;
//  uint32_t Address;
    p_dbg_enter;

    cfg->check_sum = cal_buff_check_sum((uint8_t*)cfg, SAVE_WEB_CFG_DATA_SIZE(cfg));
    
    ret =m25p80_erase(WEB_CFG_ADDR, SAVE_WEB_CFG_SIZE(cfg));
    if(ret)
    {
        printf("save_web_cfg: erase error\n");
        return -1;
    }
    ret = m25p80_write(WEB_CFG_ADDR, SAVE_WEB_CFG_SIZE(cfg), (void*)cfg);
    if(ret)
    {
        printf("save_web_cfg: write error\n");
        return -1;
    }
    return 0;
}


int read_web_cfg(WEB_CFG *cfg)
{
    int ret = 0;
    p_dbg_enter;
    memset(cfg, 0, sizeof(WEB_CFG));
    ret = m25p80_read(WEB_CFG_ADDR, SAVE_WEB_CFG_SIZE(cfg), (void*)cfg);    
    
    p_err("load_web_cfg ret=[%d]", ret);


}

int load_web_cfg(WEB_CFG *cfg)
{
    int ret = 0;
    p_dbg_enter;
    memset(cfg, 0, sizeof(WEB_CFG));
    ret = m25p80_read(WEB_CFG_ADDR, SAVE_WEB_CFG_SIZE(cfg), (void*)cfg);    
    
    p_err("load_web_cfg ret=[%d]", ret);
    if(1)//this allow user to set their own info for the first time
    //if(ret != 0 || !verify_web_cfg(cfg))
    {
        printf("load_web_cfg: reset web\n");
        
        first_web_cfg(cfg);
        default_web_cfg(cfg); 
        save_web_cfg(cfg);
        return 0;
    }

    reset_ctl_cfg(cfg);
    return 0;
}



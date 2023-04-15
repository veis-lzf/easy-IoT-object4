#define DEBUG
#include "drivers.h"
#include "app.h"
#include "api.h"

#include "usr_cfg.h"
extern wait_event_t client_camera_event;
extern wait_event_t camera_mode_event ;

struct usr_ctl_key ctl_key;

timer_t key_detect_timer;

u8 interface_ret = 0;

u8 FACE_MODE;

extern u8 wifi_ret;

void handle_key(int key_type, int key_level, int long_press);

#define LONG_PRESS_HOLD_TIME	20	// 2S



int  BUTTON_STAT_rev(int arg)
{
    switch( arg )
    {

    case KEY_USR :
        return  BUTTON_STAT;
        break;
    case KEY2 :
        return  BUTTON_STAT_2;
        break;
    }

}

void key_detect_timer_callback(void *arg)
{
    int i;
    for(i = 0; i < KEY_MAX; i++)
    {
        if(ctl_key.key_stat[i] != ctl_key.rev(i))
        {
            if(!ctl_key.rev(i)) {
                ctl_key.press_time_cnt[i] = 0;
			}
            else {
                if(ctl_key.press_time_cnt[i] < LONG_PRESS_HOLD_TIME) {
                    handle_key(i, ctl_key.rev(i), 0);
				}
            }
            ctl_key.key_stat[i] = ctl_key.rev(i);
        } 
		else {
            if(!ctl_key.rev(i)) {
                if(ctl_key.press_time_cnt[i]++ == LONG_PRESS_HOLD_TIME) {
                    handle_key(i, ctl_key.rev(i), 1);
				}
            }
        }
    }

}


void init_key_dev()
{
    memset(&ctl_key, 0, sizeof(struct usr_ctl_key));

    gpio_cfg((uint32_t)BUTTON_PORT, BUTTON_PIN, GPIO_Mode_IPU);
    gpio_cfg((uint32_t)BUTTON_PORT, BUTTON_PIN_2, GPIO_Mode_IPU);
    ctl_key.rev = BUTTON_STAT_rev;

    ctl_key.key_stat[KEY_USR] = 1;
    ctl_key.key_stat[KEY2] = 1;

    key_detect_timer = timer_setup(150, 1, key_detect_timer_callback, NULL);
    add_timer(key_detect_timer);
}

extern int send_voice_recording_event(void);

void handle_key(int key_type, int key_level, int long_press)
{

    uint32_t now_time, time_period;
    if(key_type >= KEY_MAX)
        return;

    now_time = TIM_GetCounter(TIM5);
    time_period = now_time - ctl_key.last_detect_time[key_type];
    if(time_period < 1000) {
        //p_err("time_period too short:%d", time_period);
        return;
    }
    ctl_key.last_detect_time[key_type] = now_time;

    p_dbg("key:%d, type:%d", key_type, long_press);
    ctl_key.key_stat[key_type] = key_level;

    switch(key_type)
    {
    case KEY2:
        if(!long_press)
        {
            printf("\r\n****** KEY2  SHORT *******\r\n");
            if(interface_ret == 0)
            {   
				printf("\r\n******按键按下，准备进入WIFI模式设置 *******\r\n");
                wifi_ret = 0;
                interface_ret = 1;
                send_work_event(MODE_DETECT_CHANGE_AP);
            }
#if SUPPORT_AUDIO
            else {
                if(audio_cfg.recording_to_flash) {
                    p_dbg("alreading in record");
                } 
				else {
                    audio_record_to_flash();
                    send_voice_recording_event();             //按键按下后开始录音
                    p_dbg("start record");
                }
            }
#endif
        } 
		else {
#if SUPPORT_WIFI
            printf("\r\n****** KEY2  LONG *******\r\n");
            printf("\r\n******  *******\r\n");
#endif
        }
        break;
    case KEY_USR:
        if(!long_press) {   
			printf("\r\n****** KEY1  SHORT *******\r\n");
            if(interface_ret == 0)
            {
                printf("\r\n******按键按下，准备进入识别模式设置 *******\r\n");
                interface_ret = 1;
                send_work_event(INTERFACE_MODE_SHIBIE);
            }
            else {
                wake_up(client_camera_event);             //释放信号量  ，唤醒事件  client_camera_event
            }
        } 
		else {
            printf("\r\n****** KEY1  LONG *******\r\n");
            send_work_event(INTERFACE_MODE_MOST);
        }
        break;
    default:
        break;
    }
}
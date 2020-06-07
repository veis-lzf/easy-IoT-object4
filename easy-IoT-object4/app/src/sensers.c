#define DEBUG

#include "drivers.h"
#include "app.h"
#include "api.h"

timer_t senser_poll_timer;
mutex_t senser_poll_mutex;
//struct _sensers_stat sensers_stat;
struct _sensers_pkg sensers_pkg;

//four global recording:temperature, humidity,MQ2, BH1750
uint8_t onenet_temp = 30;
uint8_t onenet_humi = 60;
uint16_t g_mq2 = 58;
uint32_t g_bh1750 = 95;

void senser_poll_callback(void *arg)
{
    int ret;
    uint8_t temp[5];
//  uint32_t card_id;
//  uint16_t card_type;
    RTC_TimeTypeDef RTC_TimeStructure;
    RTC_DateTypeDef RTC_DateStructure;

    mutex_lock(senser_poll_mutex);                  //等待一个互斥信号

    RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
    RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);

    
    sensers_pkg.sensers_stat.year = RTC_DateStructure.RTC_Year;
    sensers_pkg.sensers_stat.mon = RTC_DateStructure.RTC_Month;
    sensers_pkg.sensers_stat.date = RTC_DateStructure.RTC_Date;
    sensers_pkg.sensers_stat.week= RTC_DateStructure.RTC_WeekDay;

    sensers_pkg.sensers_stat.hour = RTC_TimeStructure.RTC_Hours;
    sensers_pkg.sensers_stat.min = RTC_TimeStructure.RTC_Minutes;
    sensers_pkg.sensers_stat.sec = RTC_TimeStructure.RTC_Seconds;

    ret = dht11_read_data(temp);
    if(ret == 0){
        sensers_pkg.sensers_stat.temperature = temp[2];
        sensers_pkg.sensers_stat.humidity= temp[0]; 
        p_dbg("Poll temperature:[%d]", (int)temp[2]);
        p_dbg("Poll humidity:[%d]", (int)temp[0]);
        onenet_temp = temp[2];
        onenet_humi = temp[0];
    }
    
//    g_mq2 = MQ2_GetValue(1);
    g_bh1750 = get_BH1750_value();
    p_dbg("Poll MQ2:[%d]\n", (int)g_mq2);
    p_dbg("Poll BH1750:[%d]\n", (int)g_bh1750);
    
    sensers_pkg.pkg_head.msg_id = MQTT_MSG_SENSERS;
    sensers_pkg.pkg_head.dir = MSG_DIR_DEV2APP;
    sensers_pkg.pkg_head.reserved = 0;

    //#if MQTT_ENABLE
    if(arg) send_work_event(SEND_SNESERS_EVENT);
       //#endif
    mutex_unlock(senser_poll_mutex);                  //释放一个互斥量

    dht11_start_read();
}

void init_sensers(void)
{
//  int ret;
#if ENABLE_RFID == 1
    init_rc522();
#endif
    BH1750_I2C_Init();    //init port  to control BH1750
    init_dht11();
//     LCD_Init();		      //初始化lcd屏幕
//    POINT_COLOR=RED;      //设置字体为红色 
    senser_poll_mutex = mutex_init("");               //新建一个互斥量

}
void start_senser_poll(void)
{
    senser_poll_timer = timer_setup(5000, 1, senser_poll_callback, &senser_poll_timer);
    mod_timer(senser_poll_timer, 5000);
}

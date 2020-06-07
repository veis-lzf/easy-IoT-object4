#ifndef _MQTT_APP_H
#define _MQTT_APP_H


void init_mqtt(WEB_CFG *web_cfg, struct iot_work_struct *work);
void mqtt_pub(void *buff, int len);
int get_mqtt_state(void);

#endif

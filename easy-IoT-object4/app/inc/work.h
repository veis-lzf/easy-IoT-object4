#ifndef _APP_WORK_H
#define _APP_WORK_H
#include "api.h"

//#define MODE_DETECT_CHANGE	(1L << 0)	    // 工作模式切换事件
//#define MODE_DETECT_CHANGE_STA	(1L << 1)	// 切换到sta模式
//#define MODE_DETECT_CHANGE_AP	(1L << 2)	// 切换到ap模式
//#define TEST_EVENT	(1L << 3)	            // 测试
//#define SEND_SNESERS_EVENT	(1L << 4)       // 传感器
//#define INTERFACE_MODE_SHIBIE	(1L << 5)	// 网口断开
//#define INTERFACE_MODE_WIFI	(1L << 6)	    // 网口连接
//#define INTERFACE_MODE_MOST	(1L << 7)       // 测试，保存图片  INTERFACE_MODE


enum PEND_EVENT_TYPE{
	MODE_DETECT_CHANGE = (1L << 0), // 工作模式切换事件标志
	MODE_DETECT_CHANGE_STA = (1L << 1), // 切换到STA模式事件标志
	MODE_DETECT_CHANGE_AP = (1L << 2), // 切换到AP模式事件标志
	TEST_EVENT = (1L << 3), // 串口输出接收结果事件标志
	SEND_SNESERS_EVENT = (1L << 4), // 发送传感器数据
	INTERFACE_MODE_SHIBIE = (1L << 5), // 进入是识别模式事件标志
	INTERFACE_MODE_MOST	= (1L << 7),  // 返回主界面显示时间标志
};

enum MSG_DIR{
	MSG_DIR_DEV2APP = 0,
	MSG_DIR_APP2DEV,
	MSG_DIR_MAX,
};

enum MQTT_MSG_ID{

	MQTT_MSG_SENSERS = 0,
	MQTT_MSG_SWITCH,
	MQTT_MSG_AUDIO,
	MQTT_MSG_IMAGE,
	MQTT_MSG_TIME,
	MQTT_MSG_LISTEN,
	MQTT_MSG_AIR,	//红外
	MQTT_MSG_RALAY,//继电器
	MQTT_MSG_JSON, //JSON格式的消息
	MQTT_MSG_MAX,
};

//本地通信消息 
//LOCAL_MSG_DISC值的大小可随意，这里用MQTT_MSG_MAX
enum LOCAL_MSG_ID{
	LOCAL_MSG_DISC_REQ = 10,
	LOCAL_MSG_DISC_RES,
	LOCAL_MSG_BOND_REQ,
	LOCAL_MSG_BOND_RES,
};

//discovery response struct
struct disc_res{
	char mac[12]; 
	char eof;
	char sn[32];
	char eof1;
}__packed;

//bond request struct
struct bond_req{
	char mac[12];
	char eof;
	char sn[32];
	char eof1;
}__packed;

struct local_pkg_head
{
	uint8_t dir;
	uint8_t msg_id;
	uint16_t data_len;
}__packed;


struct mqtt_pkg_head
{
	uint8_t dir;
	uint8_t msg_id;
	uint16_t reserved;
}__packed;

enum CTL_MSG_TYPE{

	CTL_MSG_TIME = 0,
};

#define MAX_BOND_NUM	8

struct iot_work_config{
	struct bond_req bond[MAX_BOND_NUM];
}__packed;

struct iot_work_struct{
	uint32_t event_flag;	// 事件标志
	wait_event_t event;		// 等待事件
	struct disc_res self_sn;
};

struct ir_pkg{
	struct mqtt_pkg_head head;
	uint8_t data_len;
}__packed;

extern struct iot_work_struct  iot_work;
extern int local_udp_server, local_udp_client;
void send_work_event(uint32_t event);
void main_process(void);
int udp_broadcast(uint16_t port, uint8_t *p_data, int len);
void handle_local_udp_recv(uint8_t *data, int len);
int bond_host(struct bond_req *req);
int test_connect_to_machtalk_server(void);

#endif


#ifndef _APP_WORK_H
#define _APP_WORK_H
#include "api.h"

//#define MODE_DETECT_CHANGE	(1L << 0)	    // ����ģʽ�л��¼�
//#define MODE_DETECT_CHANGE_STA	(1L << 1)	// �л���staģʽ
//#define MODE_DETECT_CHANGE_AP	(1L << 2)	// �л���apģʽ
//#define TEST_EVENT	(1L << 3)	            // ����
//#define SEND_SNESERS_EVENT	(1L << 4)       // ������
//#define INTERFACE_MODE_SHIBIE	(1L << 5)	// ���ڶϿ�
//#define INTERFACE_MODE_WIFI	(1L << 6)	    // ��������
//#define INTERFACE_MODE_MOST	(1L << 7)       // ���ԣ�����ͼƬ  INTERFACE_MODE


enum PEND_EVENT_TYPE{
	MODE_DETECT_CHANGE = (1L << 0), // ����ģʽ�л��¼���־
	MODE_DETECT_CHANGE_STA = (1L << 1), // �л���STAģʽ�¼���־
	MODE_DETECT_CHANGE_AP = (1L << 2), // �л���APģʽ�¼���־
	TEST_EVENT = (1L << 3), // ����������ս���¼���־
	SEND_SNESERS_EVENT = (1L << 4), // ���ʹ���������
	INTERFACE_MODE_SHIBIE = (1L << 5), // ������ʶ��ģʽ�¼���־
	INTERFACE_MODE_MOST	= (1L << 7),  // ������������ʾʱ���־
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
	MQTT_MSG_AIR,	//����
	MQTT_MSG_RALAY,//�̵���
	MQTT_MSG_JSON, //JSON��ʽ����Ϣ
	MQTT_MSG_MAX,
};

//����ͨ����Ϣ 
//LOCAL_MSG_DISCֵ�Ĵ�С�����⣬������MQTT_MSG_MAX
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
	uint32_t event_flag;	// �¼���־
	wait_event_t event;		// �ȴ��¼�
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


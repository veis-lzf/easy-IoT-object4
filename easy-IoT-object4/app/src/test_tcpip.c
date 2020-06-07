#define DEBUG

#include "drivers.h"
#include "app.h"
#include "api.h"

#include "lwip\sockets.h"
#include "lwip\netif.h"
#include "lwip\dns.h"
#include "lwip\api.h"
#include "lwip\tcp.h"

#include "cfg80211.h"
#include "defs.h"
#include "type.h"
#include "types.h"

#include "tcpapp.h"
#include "wifi.h"
#include "debug.h"
#include "lwip/dhcp.h"
#include "sys_misc.h"

#include "web_cfg.h"

//全局变量声明
extern int errno; //lwip错误号

/*
 * socket互斥锁，对socket的所有线程外的操作需要上锁
 * 如果不加锁，lwip会很不稳定，原因是lwip不支持多线程同时操作一个socket
 * 需要注意的是像send这样可能会阻塞的函数，如果加了锁将会降低数据
 * 收发的效率，所以收发数据时采用非阻塞方式要好些(MSG_DONTWAIT)
 */
extern mutex_t socket_mutex;

//函数声明
void test_tcp_recv(void *_fd);

//摄像头服务器socket
int server_socket_fd =  - 1;
//记录客户端的连接，用于向客户端发送摄像头数据
int remote_socket_fd =	- 1;

struct sockaddr udp_remote_client __attribute__((aligned(4))); //用于保存远程udp客户端信息

//thread 句柄
int server_accept_thread_fd =  - 1;

/*
 * @brief  连接到IP地址为192.168.0.100，端口号4700的TCP服务器
 *  连接成功后发送数据并关闭连接
 */
void test_tcp_client()
{
	char *ip = "192.168.1.108";
	char *data = "hello, Im a TCP CLIENT";
	uint16_t port = 4700;
	int fd = -1;
	p_dbg_enter;
	p_dbg("TCP连接到:%s, 端口号:%d", ip, port);

	fd = tcp_client_create(ip, port);
	if(fd > 0)
	{
            send(fd, data, strlen(data), 0);
            close(fd);
            fd = -1;
	}
	p_dbg_exit;
}

/*
 * @brief  连接到IP地址为192.168.0.100，端口号4701的UDP服务器
 * 连接成功后，发送数据并关闭连接
 */
void test_udp_client()
{
	char *ip = "192.168.1.100";
	char *data = "hello, Im an UDP CLIENT";
	uint16_t port = 4701;
	int fd = -1;
	p_dbg_enter;
	p_dbg("UDP连接到:%s, 端口号:%d", ip, port);

	fd = udp_client_create(ip, port);
	if(fd > 0)
	{
            send(fd, data, strlen(data), 0);
            close(fd);
            fd = -1;
	}
	
	p_dbg_exit;
}


/*
 *这里测试发送udp数据到远程端
 *这里新建一个socket并发送数据到192.168.1.101:4703
 *未绑定远端端口，使用sendto发送数据
 */
void test_udp_client2(char *pstr)
{
	int fd;
	struct sockaddr_in addr;
	int len = strlen(pstr);
	
	p_dbg_enter;

	p_dbg("enter %s\n", __FUNCTION__);
	addr.sin_family = AF_INET;
	addr.sin_len = sizeof(struct sockaddr_in);
	addr.sin_port = htons(4703);
	addr.sin_addr.s_addr = inet_addr("192.168.1.101");

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0)
	{
		p_err("get socket err:%d", errno);
		return;
	} 

	mutex_lock(socket_mutex);
	len = sendto(fd, (u8*)pstr, len, 0, (struct sockaddr*)&addr, sizeof(struct sockaddr));
	mutex_unlock(socket_mutex);
	p_dbg("ret:%d", len);
	close(fd);
	
	p_dbg_exit;
}

/*
 * @brief  添加到多播组
 * 组播地址为224.0.0.2，端口号4702
 */
void test_multicast_send_data(int udp_fd);
void test_multicast_join()
{
    //加入到这个多播组，可以接收这个多播组的所有数据
	char *ip = "239.118.0.0";
	uint16_t port = 10000;
	int udp_fd = -1;
	p_dbg_enter;
	p_dbg("连接到:%s, 端口号:%d", ip, port);
#if SUPPORT_WIFI
	wifi_set_multicast(IFF_ALLMULTI);
#endif
	udp_fd = udp_add_membership(ip, port);

	if(udp_fd != -1)
		test_multicast_send_data(udp_fd);
    //TODO:这里也可以修改程序，尝试接收UDP多播包

	
	close(udp_fd);//测试完成，关闭连接
	p_dbg_exit;
}

/*
 * @brief 像多播组发送数据
 * 组播地址为224.0.0.2，端口号4702
 */
void test_multicast_send_data(int udp_fd)
{
    //发送数据到一个多播组，UDP客户端可以发送任何数据到任何多播组
	char *ip = "224.0.0.2";
	uint16_t port = 4702;
	struct sockaddr_in multisnd_addr;
		
	p_dbg_enter;
	p_dbg("发送数据到:%s, 端口号:%d", ip, port);

	if(udp_fd == -1)
	{
		p_err("udp socket not create");
		return ;
	}
	memset(&multisnd_addr, 0, sizeof(struct sockaddr_in));
	multisnd_addr.sin_len = sizeof(struct sockaddr);
	multisnd_addr.sin_family = AF_INET;
	multisnd_addr.sin_port=htons(port);
	multisnd_addr.sin_addr.s_addr=inet_addr(ip);
	mutex_lock(socket_mutex);
	sendto(udp_fd, "this is test data", 20, 0,(struct sockaddr*)&multisnd_addr,sizeof(struct sockaddr));
	mutex_unlock(socket_mutex);
	p_dbg_exit;
}


DECLARE_MONITOR_ITEM("tcp totol send", tcp_totol_send);

/*
 * @brief TCP数据发送线程
 * tcp_send_stop用于用户控制结束测试
 *
 */
int tcp_send_stop = 0;
#define TEST_PACKET_SIZE 	1024
void tcp_send_thread(void *arg)
{
	int i;
	char *send_buff;
	char *ip = "192.168.1.108";
	uint16_t port = 4700;
    int fd = -1;
	send_buff = (char*)malloc(TEST_PACKET_SIZE);
	if(!send_buff)
		goto end;
    fd = tcp_client_create(ip, port);
	
	if(fd == -1){
		p_err("请先与服务器建立TCP连接");
		goto end;
	}

	for(i = 0; i < TEST_PACKET_SIZE; i++)
		send_buff[i] = i;
	p_dbg("full tcp send start");
	while(1)
	{

		mutex_lock(socket_mutex);
		//采用阻塞方式发送,这种方式发送会阻塞相当长时间
		i = send(fd, send_buff, TEST_PACKET_SIZE, /*MSG_DONTWAIT*/0);
		mutex_unlock(socket_mutex);

		ADD_MONITOR_VALUE(tcp_totol_send, TEST_PACKET_SIZE);
		if(tcp_send_stop)
		{
			p_dbg("stop tcp send test");
			goto end;
		}
		//sleep(10); //如果想减慢发送速度，可以在这里延时一下
	}

end:
	p_dbg("tcp send end");
	if(send_buff)
		mem_free(send_buff);
	thread_exit(thread_myself());
}
/*
 * @brief 满负荷发送，请先建立TCP连接
 * 建立单独的线程
 *
 */
void test_full_speed_send()
{
	p_dbg_enter;
	tcp_send_stop = 0;
	thread_create(tcp_send_thread, 0, TASK_TCP_SEND_PRIO, 0, TASK_TCP_SEND_STACK_SIZE, "tcp_send_thread");
	p_dbg_exit;
}

void test_full_speed_send_stop()
{
	p_dbg_enter;
	tcp_send_stop = 1;
	p_dbg_exit;
}

/*
 * @brief  关闭本地服务器
 *
 */
void test_close_camera_server()
{
	p_dbg_enter;
	mutex_lock(socket_mutex);
	if (remote_socket_fd !=  - 1)
	{
		close_socket(remote_socket_fd);
		remote_socket_fd =  - 1;
	}

	if (server_socket_fd !=  - 1)
	{
		close_socket(server_socket_fd);
		server_socket_fd =  - 1;
	}

	if (server_accept_thread_fd !=  - 1)
	{
		thread_exit(server_accept_thread_fd);
		server_accept_thread_fd =  - 1;
	}
    mutex_unlock(socket_mutex);
	p_dbg_exit;
}

/*
 * @brief 本地服务器监听线程
 *
 */
void tcp_camera_task(void *server_fd)
{
	int sockaddr_len, new_socket, opt;
	struct sockaddr_in addr;

	sockaddr_len = sizeof(struct sockaddr);

	while (1)
	{
		p_dbg("waiting for remote connect");
		new_socket = accept((int)server_fd, (struct sockaddr*) &addr, (socklen_t*) &sockaddr_len);
		if (new_socket ==  - 1)
		{
			p_err("accept err");
			break;
		} 
		p_dbg("accept a new client");
        if(remote_socket_fd == -1)
        {
		    remote_socket_fd = new_socket;
        }
		else
		{
            p_err("a camera client already connected");
			close_socket(new_socket);
			continue;
		}
		opt = 1;
		if (setsockopt(new_socket, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(int)) ==  - 1)
			p_err("SO_KEEPALIVE err\n");

//		start_capture_img(); //开始摄像头捕获，发给本地客户端

	}
	thread_exit(server_accept_thread_fd);
}

/*
 * @brief  在本地建立一个服务器并等待连接
 * 对于连接成功的客户端返回摄像头数据
 *
 */
void camera_tcp_server(uint16_t port)
{

	int socket_s =  - 1, err = 0;
	struct sockaddr_in serv;
	memset(&serv, 0, sizeof(struct sockaddr_in));
	p_dbg_enter;

	p_dbg("Camera server, port:%d", port);
	test_close_camera_server();

	serv.sin_family = AF_INET;
	serv.sin_port = htons(port);
	serv.sin_addr.s_addr = htons(INADDR_ANY);

	socket_s = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_s ==  - 1)
	{
		goto err;
	}
	err = bind(socket_s, (struct sockaddr*) &serv, sizeof(struct sockaddr_in));
	if (err ==  - 1)
	{
		goto err;
	} err = listen(socket_s, 4);
	if (err ==  - 1)
	{
		goto err;
	}

	server_socket_fd = socket_s;

	server_accept_thread_fd = thread_create(tcp_camera_task, (void*)server_socket_fd, TASK_TCP_ACCEPT_PRIO, 0, TASK_ACCEPT_STACK_SIZE, "tcp_accept_task");

	return ;
	err: if (err < 0)
		p_err("err:%d", err);
	if (socket_s !=  - 1)
		close_socket(socket_s);
	p_dbg_exit;
}


void show_tcpip_info(struct netif *p_netif)
{
	ip_addr_t dns_server;

	dns_server = dns_getserver(0);

	p_dbg("ipaddr: %d.%d.%d.%d", ip4_addr1(&p_netif->ip_addr.addr), ip4_addr2(&p_netif->ip_addr.addr), ip4_addr3(&p_netif->ip_addr.addr), ip4_addr4(&p_netif->ip_addr.addr));

	p_dbg("netmask: %d.%d.%d.%d", ip4_addr1(&p_netif->netmask.addr), ip4_addr2(&p_netif->netmask.addr), ip4_addr3(&p_netif->netmask.addr), ip4_addr4(&p_netif->netmask.addr));

	p_dbg("gw: %d.%d.%d.%d", ip4_addr1(&p_netif->gw.addr), ip4_addr2(&p_netif->gw.addr), ip4_addr3(&p_netif->gw.addr), ip4_addr4(&p_netif->gw.addr));


	p_dbg("dns_server: %d.%d.%d.%d", ip4_addr1(&dns_server.addr), ip4_addr2(&dns_server.addr), ip4_addr3(&dns_server.addr), ip4_addr4(&dns_server.addr));
}


/*
 * @brief  自动获取IP测试
 *
 *
 */
int auto_get_ip(struct netif *p_netif);
void test_auto_get_ip(struct netif *p_netif)
{
	int i, wait_time = 10;

	auto_get_ip(p_netif);

	for (i = 0; i < wait_time; i++)
	{
		p_dbg("%d", i);
		if (p_netif->ip_addr.addr)
			break;
		sleep(1000);
	}

	if (p_netif->ip_addr.addr)
	{
		show_tcpip_info(p_netif);
	}

}

char loopback_enable = 1;
void switch_loopback_test()
{
	loopback_enable = !loopback_enable;
	if(loopback_enable)
		p_dbg("使能回发测试");
	else
		p_dbg("关闭回发测试");
}

/**
 * @brief  处理接收到的socket通信数据，目前只作为测试命令使用
 * @比如，tcp客户端连接到了开发板，客户端发过来的数据将在这里处理
 * @
 */
void handle_test_recv(int socket, uint8_t *data, int len)
{
	if(loopback_enable)
	{
		mutex_lock(socket_mutex);
		send(socket, data, len, 0);
		mutex_unlock(socket_mutex);
	}else{
		//p_dbg("socket:%d, recv:%d byte", socket, len);
		//dump_hex("data", data, len);
		//send_cmd_to_self(data[0]);
	}
}


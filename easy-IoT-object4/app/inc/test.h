#ifndef _TEST_H
#define _TEST_H

void test_scan(void);
void test_wifi_connect(void);
void test_start_ap(void);
void test_wifi_disconnect(void);
void test_tcpudp_recv(void);
void test_tcp_client(void);

void test_close_camera_server(void);
void camera_tcp_server(uint16_t port);
void test_auto_get_ip(struct netif *p_netif);
void test_open_audio(void);
void test_close_audio(void);
void test_wifi_get_stats(void);
void show_tcpip_info(struct netif *p_netif);
void show_sys_info(struct netif *p_netif);
void test_udp_client(void);
void test_multicast_join(void);
void test_create_ap(void);
void test_stop_ap(void);
void test_get_station_list(void);
void test_power_save_enable(void);
void test_power_save_disable(void);
void test_wifi_join_adhoc(void);
void test_wifi_leave_adhoc(void);
void monitor_switch(void);
void test_full_speed_send(void);
void test_full_speed_send_stop(void);
void switch_loopback_test(void);
void print_help(void);
void handle_test_recv(int socket, uint8_t *data, int len);
void handle_cmd(void);
void test_gui(void);
#ifdef SURPPORT_USB
void test_fs_speed(void);
void test_fs_open_create_file(void);
void test_fs_read_file(void);
void test_fs_write_file(void);
void test_mount_fs(void);
u8 mf_scan_files(u8 * path);
void test_save_jpeg(void);
void test_usb_device_read(void);
void test_play_mp3(void);
void test_stop_mp3(void);
#endif
#endif

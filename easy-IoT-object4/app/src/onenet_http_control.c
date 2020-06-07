#define DEBUG

#include "lwip\sockets.h"
#include "lwip\netif.h"
#include "lwip\dns.h"
#include "lwip\api.h"
#include "lwip\tcp.h"
#include "lwip\netdb.h"
#include "debug.h"

#if ONENET_ENABLE
/*remember system time*/
static unsigned int sys_timer = 0;
#define YBUFSIZE 1024
static char request[YBUFSIZE];

#define HTTP_ADDR "api.heclouds.com"
#define HTTP_PORT 80                   //server port


//#define DEV_ID "7558895"                              //change to your device-ID
//#define API_KEY "2cFWuEAf6lg0tX4mvw=gCYVfMO0="        //change to your API-Key
#define DEV_ID "9444919"                         //change to your device-ID
#define API_KEY "RJYQPAEr0snGneaAu3TNh0ia=CU=" //change to your API-Key

#define LEDCTL_ID "ledctl"     //IO4 for PWM
#define RELAYA_ID "relaya"     //IO5
#define RELAYB_ID "relayb"     //IO6
#define RELAYC_ID "relayc"     //IO7
#define RELAYD_ID "relayd"     //IO8

int http_open(const char *addr, int portno)
{
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    int opt;
	
    /* create socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
		p_dbg("yeelink new socket error");
        return -1; 
    }
	
    server = gethostbyname(addr);
    if (server == NULL) {
        close(sockfd);
        p_dbg("yeelink dns error");
        return -1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_len = sizeof(serv_addr);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = PP_HTONS(portno);
    //addr.sin_addr.s_addr = inet_addr(SOCK_TARGET_HOST);
    serv_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr *)server->h_addr_list[0])));
  
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
    {
        p_dbg("yeelink connect error\n");
        close(sockfd);
        return -1;
    }
	
    opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(int)) < 0)
    {
        p_dbg("yeelink setsockopt error\n");
        close(sockfd);
        return -1;
    }

    return sockfd;
}
static void process_recv_html(char * body)
{
    char *value = NULL;
	char *stream = NULL;
    int16 num = 0;
	/*
	//int i = 0;
	do
	{
	    value = strstr(body,"value");
	    if (value == NULL)
	    {
		    break;
	    }

		num = get_number(value + 7);
		if(num < 0)
		{
            break;
		}
        body = value + 7;
		
		stream = strstr(body, "id");
		if (stream == NULL)
	    {
		    break;
	    }

		body = stream + 4;
		
		if(update_stream(body, num) < 0)
		{
            break;
		}
		
	}while(*body != 0);
	
	if(myhttp[0].change)
	{
	    lamp_pwm_set(myhttp[0].value);
		myhttp[0].change = 0;
	}
	if(myhttp[1].change)
	{
	    update_relay_status(5, myhttp[1].value);
		myhttp[1].change = 0;
	}
	if(myhttp[2].change)
	{
	    update_relay_status(6, myhttp[2].value);
		myhttp[2].change = 0;
	}
	if(myhttp[3].change)
	{
	    update_relay_status(7, myhttp[3].value);
		myhttp[3].change = 0;
	}
	if(myhttp[4].change)
	{
	    update_relay_status(8, myhttp[4].value);
		myhttp[4].change = 0;
	}

	*/
}

int http_recv_func(int arg)
{
    int sockfd = arg;
    int count = read(sockfd, request, YBUFSIZE);
    if (count <= 0)
    {
        p_dbg("yeelink conn closed [%d]\n", count);
        return -1;
    }
	
    process_recv_html(request);
    return 0;
}

int send_http_request(int sock)
{
   ///devices/9444919/datapoints?datastream_id=ledctl,relaya,relayb,relayc,relayd&limit=1
	memset(request,0,YBUFSIZE);
        //get data
    strcat(request,"GET /devices/");
    strcat(request,DEV_ID);
    strcat(request,"/datapoints?datastream_id=");//注意后面必须加上\r\n
    strcat(request,LEDCTL_ID);
	strcat(request,",");
	strcat(request,RELAYA_ID);
	strcat(request,",");
	strcat(request,RELAYB_ID);
	strcat(request,",");
	strcat(request,RELAYC_ID);
	strcat(request,",");
	strcat(request,RELAYD_ID);
	
    strcat(request,"&limit=1 HTTP/1.1\r\n");
    strcat(request,"api-key:");
    strcat(request,API_KEY);
    strcat(request,"\r\n");
    strcat(request,"Host:");
    strcat(request,HTTP_ADDR);
    strcat(request,"\r\n");
    strcat(request,"Connection: Keep-Alive\r\n");
    strcat(request,"Cache-Control: no-cache\r\n\r\n");

	//example for send data
	/*
        strcat(text,"{\"datastreams\":[{");
        strcat(text,"\"id\":\"sys_time\",");
        strcat(text,"\"datapoints\":[");
        strcat(text,"{");
        sprintf(tmp, "\"value\":%d", 100);
        strcat(text,tmp);
        strcat(text,"}]}]}");
    
        request[0] = 0;
        strcat(request,"POST /devices/");
        strcat(request,PDEV_ID);
        strcat(request,"/datapoints HTTP/1.1\r\n");//注意后面必须加上\r\n
        strcat(request,"api-key:");
        strcat(request,PAPI_KEY);
        strcat(request,"\r\n");
        strcat(request,"Host:");
        strcat(request,YEELINK_ADDR);
        strcat(request,"\r\n");
        sprintf(tmp,"Content-Length:%d\r\n\r\n", strlen(text));//计算JSON串长度
        strcat(request,tmp);
        strcat(request,text);
	*/
    p_dbg("send to server [%s]\n", request);
	return send(sock, request, strlen(request), MSG_DONTWAIT);
}

int http_write_func(int arg)
{
    unsigned int now = sys_now();
    int sockfd = arg;
    int ret = 0;
	static int close_active = 0;
    /*request data every 1 Seconds*/
    if(now - sys_timer < 1500)
    {
        sys_msleep(100);
		return 0;
    }

	close_active++;
	
	if(close_active == 5)
	{
        close_active = 0;
		return -1;           //close connection actively
	}
    
    sys_timer = now;

    ret = send_http_request(sockfd);
    return ret;
}

int http_client_process(void* arg)
{
    int sockfd = *(int*)arg;
    fd_set readset;
    fd_set writeset;
    int i, maxfdp1;
	
    for (;;)
    {
        maxfdp1 = sockfd+1;
	
        /* Determine what sockets need to be in readset */
        FD_ZERO(&readset);
        FD_ZERO(&writeset);
        FD_SET(sockfd, &readset);
        FD_SET(sockfd, &writeset);

        i = select(maxfdp1, &readset, &writeset, 0, 0);
        
        if (i == 0)
            continue;
		
        if (FD_ISSET(sockfd, &readset))
        {
            /* This socket is ready for reading*/
            if (http_recv_func(sockfd) < 0)
                break;
        }
        /* This socket is ready for writing*/
        if (FD_ISSET(sockfd, &writeset))
        {
            if (http_write_func(sockfd) < 0)
                break;
        }
		
    }

    return -1;
}

void http_thread(void * arg)
{
    int sockfd;
		
    sys_timer = sys_now();
    p_dbg("http_thread in\n");
    while(1)
    {
	    /* create a socket and connect to server */
        sockfd = http_open(HTTP_ADDR, HTTP_PORT);
        if (sockfd < 0)
        {
            p_dbg("server open error\n");
            continue;
        }
        
        http_client_process((void*)&sockfd);

        /*recv error happen, just close socket*/
        close(sockfd);
        p_dbg("close and try again\n");
    }
    
    //return 0;
}

void init_http_client(void)
{
    sys_thread_new("yeelink_thread", http_thread, 0, 512, TCPIP_THREAD_PRIO + 2);
    p_dbg("[task]yeelink client created\n");
}

#endif



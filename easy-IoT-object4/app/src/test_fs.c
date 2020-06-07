/*
*********************************************************************************************************
*   U盘读写,这部分包括 USB驱动,FATFS, 两部分
*   USB驱动:\drivers\stm32f4xx\usb目录
*   FATFS:\ff11a
*   这两部分代码再整个系统中比较独立,主要依赖os_api,不需要的话删掉即可
*
*
----------------------------------------------------------------------
File        : test_fs.c
Purpose     : 
---------------------------END-OF-HEADER------------------------------
*/


#define DEBUG

#include "drivers.h"
#include "app.h"
#include "api.h"

#include "ff.h"
#include "mad.h"
FATFS USBDISKFatFs;
FIL test_file;
const char test_file_name[] = "0:test.txt";
uint8_t *test_buff = NULL;
FILINFO fileinfo;	
DIR dir;

timer_t fs_test_timer;
void start_mp3_thread(void);

void test_fs_speed()
{
	uint32_t i, nb, time1, time2, speed;
	
	if(!test_buff)
		return;
	
	memset(test_buff, 't', 1024);
	p_dbg("start write");
	time1 = os_time_get();
	for(i =0; i < 10*1024; i++)
		f_write(&test_file, test_buff, 1024, &nb);
	time2 = os_time_get();
	p_dbg("time:%d ms", time2 - time1);
	speed = 10*1024*1024/((time2 - time1)/1000);
	p_dbg("speed:%d byte/s", speed); 
	p_dbg("end write");
	f_sync(&test_file);
	
	p_dbg("start read");
	f_lseek(&test_file, 0);
	for(i =0; i < 10*1024; i++)
		f_read(&test_file, test_buff, 1024, &nb);
	p_dbg("time:%d ms", time2 - time1);
	speed = 10*1024*1024/((time2 - time1)/1000);
	p_dbg("speed:%d byte/s", speed); 
	p_dbg("end read");
}

 //遍历文件
 //path:路径
 //返回值:执行结果
u8 mf_scan_files(u8 * path)
{
	FRESULT res;	  
    char *fn;   /* This function is assuming non-Unicode cfg. */
#if _USE_LFN
 	fileinfo.lfsize = _MAX_LFN * 2 + 1;
	fileinfo.lfname = mem_malloc(fileinfo.lfsize);
#endif		  

    res = f_opendir(&dir,(const TCHAR*)path); //打开一个目录
    if (res == FR_OK) 
	{	
		printf("\r\n"); 
		while(1)
		{
	        res = f_readdir(&dir, &fileinfo);                   //读取目录下的一个文件
	        if (res != FR_OK || fileinfo.fname[0] == 0) break;  //错误了/到末尾了,退出
	        //if (fileinfo.fname[0] == '.') continue;             //忽略上级目录
#if _USE_LFN
        	fn = *fileinfo.lfname ? fileinfo.lfname : fileinfo.fname;
#else							   
        	fn = fileinfo.fname;
#endif	                                              /* It is a file. */
			printf("%s/", path);//打印路径	
			printf("%s, size:%u\r\n",  fn, fileinfo.fsize);//打印文件名	  
		}

	    printf("close DIR ret=[%d]\n",f_closedir(&dir));
    }	  
	mem_free(fileinfo.lfname);
    return res;	  
}

struct mp3node
{
    struct mp3node* next;
    char *mp3name;
};
struct mp3node *mp3list = 0;
struct mp3node *mp3play = 0;

u8 scan_mp3_files(u8 * path)
{
	FRESULT res;	  
    char *fn;   /* This function is assuming non-Unicode cfg. */
	struct mp3node * node = NULL;
#if _USE_LFN
 	fileinfo.lfsize = _MAX_LFN * 2 + 1;
	fileinfo.lfname = mem_malloc(fileinfo.lfsize);
#endif		  

    res = f_opendir(&dir,(const TCHAR*)path); //打开一个目录
    if (res == FR_OK) 
	{	
		printf("\r\n"); 
		while(1)
		{
	        res = f_readdir(&dir, &fileinfo);                   //读取目录下的一个文件
	        if (res != FR_OK || fileinfo.fname[0] == 0) break;  //错误了/到末尾了,退出
	        //if (fileinfo.fname[0] == '.') continue;             //忽略上级目录
#if _USE_LFN
        	fn = *fileinfo.lfname ? fileinfo.lfname : fileinfo.fname;
#else							   
        	fn = fileinfo.fname;
#endif	                                              /* It is a file. */
			printf("%s, size:%u\r\n",  fn, fileinfo.fsize);//打印文件名
			if(strstr(fn, ".mp3") == NULL) break;
			
			node = 	mem_calloc(sizeof(struct mp3node), 1);
            if(node == NULL) break;

			node->mp3name = mem_calloc(strlen(fn) + 1, 1);
			if(node->mp3name == NULL)
			{
                mem_free(node);
				break;
			}

			memcpy(node->mp3name, fn, strlen(fn));
            
			node->next = mp3list;
			mp3list = node;
			if(!mp3play) mp3play = mp3list;
			
		}

	    printf("close DIR ret=[%d]\n",f_closedir(&dir));
    }	  
    mem_free(fileinfo.lfname);
    return res;	  
}

void free_mp3_files(void)
{
    struct mp3node * node = mp3list;
	while(node)
	{
        if(node->mp3name)
        {
            mem_free(node->mp3name);
			node->mp3name = NULL;
		}

		mp3list = node->next;
		mem_free(node);
		node = mp3list;
	}

	mp3list = 0;
	mp3play = 0;
}

char *cur_mp3_file_name(void)
{
    if(mp3play == NULL) return NULL;
	else return mp3play->mp3name;
}
void test_save_jpeg(void);
 void fs_test_timer_cb(void *arg)
 {
// 	send_work_event(TEST_SAVE_JPEG);
 }

void test_fs_open_create_file()
{
	int ret;
	ret = f_open(&test_file,"0:test.txt", FA_READ | FA_WRITE | FA_CREATE_ALWAYS);
	if(ret)
		p_err("打开文件出错 %d", ret);
	else
		p_dbg("打开文件成功");

	fs_test_timer = timer_setup(2000, 1, fs_test_timer_cb, 0);
	add_timer(fs_test_timer);
}


void test_fs_read_file()
{
	uint32_t ret, nb;
	
	if(!test_buff)
		return;
	
	ret = f_read(&test_file, test_buff, 32, &nb);
	if(ret)
		p_err("读取文件出错 %d", ret);
	else{
		p_dbg("读取文件成功");
		p_dbg("读取%d byte", nb);
		if(nb == 0)
		{
			p_dbg("已经读到了文件尾,现在把指针移到文件头");
			f_lseek(&test_file, 0);
		}else
			dump_hex("data", test_buff, 32);
	}
}

void test_fs_write_file()
{
	static char write_char = 'a';
	uint32_t ret, nb;
	
	if(!test_buff)
		return;

	memset(test_buff, write_char++, 32);
	ret = f_write(&test_file, test_buff, 32, &nb);
	if(ret)
		p_err("写文件出错 %d", ret);
	else{
		p_dbg("写文件成功");
		p_dbg("写入%d byte", nb);
	}
}


void test_usb_device_read()
{
	uint32_t ret, i;

	p_dbg_enter;
	if(!test_buff)
		return;

	p_dbg("start:%d", os_time_get());
	for(i = 0; i < 2*1024*1024; i++)
	{
		ret = disk_read(0, test_buff, i, 1);
		if(ret)
			break;
	}
	p_dbg("end:%d, %d", os_time_get(), i);
	p_dbg_exit;
}

//open dir firstly before call this function
u8 mf_readdir(void)
{
	u8 res;
	char *fn;			 
#if _USE_LFN
 	fileinfo.lfsize = _MAX_LFN * 2 + 1;
	fileinfo.lfname = mem_malloc(fileinfo.lfsize);
#endif		  
	res=f_readdir(&dir,&fileinfo);//?áè?ò??????tμ?D??￠
	if(res!=FR_OK||fileinfo.fname[0]==0)
	{
		mem_free(fileinfo.lfname);
		return res;//?áíêá?.
	}
#if _USE_LFN
	fn=*fileinfo.lfname ? fileinfo.lfname : fileinfo.fname;
#else
	fn=fileinfo.fname;;
#endif	
	printf("\r\n DIR info:\r\n");

	printf("dir.id:%d\r\n",dir.id);
	printf("dir.index:%d\r\n",dir.index);
	printf("dir.sclust:%d\r\n",dir.sclust);
	printf("dir.clust:%d\r\n",dir.clust);
	printf("dir.sect:%d\r\n",dir.sect);	  

	printf("\r\n");
	printf("File Name is:%s\r\n",fn);
	printf("File Size is:%d\r\n",fileinfo.fsize);
	printf("File data is:%d\r\n",fileinfo.fdate);
	printf("File time is:%d\r\n",fileinfo.ftime);
	printf("File Attr is:%d\r\n",fileinfo.fattrib);
	printf("\r\n");
	mem_free(fileinfo.lfname);
	return 0;
}		

void test_save_jpeg()
{
	int ret, len;
	uint32_t nb;
	FIL f_img;
	uint8_t *p_img;
	char f_name[64];

	snprintf(f_name, 64, "img_%d.jpg", os_time_get());
	//新建一个文件
	ret = f_open(&f_img, f_name, FA_READ | FA_WRITE | FA_CREATE_ALWAYS);
	if(ret){
		p_err("打开文件出错 %d", ret);
		return;
	}	
	//打开捕获
//	start_capture_img();
	ret = msgrcv(camera_cfg.img_queue, (void**)&p_img, 100); //等待接收一个图片
	if(ret == 0 && p_img)
	{
		memcpy(&len, p_img, 4);
		p_dbg("img len:%d", len);
		ret = f_write(&f_img, p_img + 4, len, &nb);
		if(!ret && len == nb)
			p_dbg("%s saved ok, 发送 'w' 切换分辨率, 发送 'F' 列出文件", f_name);
		else
			p_err("%s saved err", f_name);
		
		mem_free(p_img); //释放图片内存
	}else
		p_err("no img");
	//关闭捕获
//	stop_capture_img();
	//关闭文件
	f_close(&f_img);
	
}

void test_mount_fs()
{
	FRESULT ret;
	int flag = 0;

	if(!test_buff)
	{
		test_buff = (uint8_t*)mem_malloc(1024);
		flag = 1;
	}
	ret = f_mount(&USBDISKFatFs, "0:", 1); 
	if(ret)
		p_err("Load file system fail");
	else
	{
		p_dbg("Load file system success");
		p_dbg("Sectors per cluster:%d", USBDISKFatFs.csize);
		p_dbg("Sectors per FAT:%d", USBDISKFatFs.fsize);
		p_dbg("Number of FAT copies:%d", USBDISKFatFs.n_fats);
		p_dbg("Number of root directory entries:%d", USBDISKFatFs.n_rootdir);
		//p_dbg("Bytes per sector", USBDISKFatFs.ssize);
		p_dbg("Number of FAT entries:%d", USBDISKFatFs.n_fatent);

		scan_mp3_files((u8*)"0:music");
		#if !BAIDU_AI_VOICE_ENABLE
		if(flag) start_mp3_thread();
               #endif
	}
	
	p_dbg("ret %d", ret);
}

void test_unmount_fs()
{
	FRESULT ret;
	free_mp3_files();
	ret = f_mount(NULL, "0:", 1); 	
	p_dbg("test_unmount_fs ret %d", ret);
}

/*
*播放mp3
*循环播放music目录下的所有歌曲
*
*/

extern WEB_CFG *Wcfg;
//char musicname[100] = {0,};
void music_thread(void *arg)
{
	int res;
	char *fn, *buff; 
	uint32_t nb;
	FIL file;
#if _USE_LFN
 	fileinfo.lfsize = _MAX_LFN * 2 + 1;
	fileinfo.lfname = mem_malloc(fileinfo.lfsize);	
#endif
	buff = (char*)test_buff;	//make sure buffer is allocated before
	
	if(!buff)
	{
		goto end;
	}

	while(1)
	{
		sleep(2000);
               #if BAIDU_AI_VOICE_ENABLE
		if(USBDISKFatFs.fs_type && audio_cfg.playing_mp3)
		{
			//p_dbg("open dir: music");
	    	//res = f_opendir(&dir,(const TCHAR*)"0:music"); //打开目录
	    	res = FR_OK;
	    	if (res == FR_OK) 
			{	
				while(1)
				{   
				    #if 0
					res = f_readdir(&dir, &fileinfo);                   //读取目录内容,获取文件名
				    if (res != FR_OK || fileinfo.fname[0] == 0) {
						p_dbg("dir end");
						break;  
				    }
#if _USE_LFN
			        fn = *fileinfo.lfname ? fileinfo.lfname : fileinfo.fname;
#else							   
			        fn = fileinfo.fname;
#endif	                                              /* It is a file. */
					#endif
					fn = cur_mp3_file_name();
					if(!fn) break;
					
					printf("play:%s\r\n",  fn);//打印文件信息

					while(1)
					{
					    sprintf(buff, "0:music/%s\0", fn); //打开一个mp3文件
					    printf("open music:%s\n", buff);
					
					    res = f_open(&file, buff, FA_READ);
					    if(res)
					    {
						    p_err("music f_open err %d", res);
						    if(!audio_cfg.playing_mp3)
								goto end;
							
						    if(audio_cfg.play_next){
								audio_cfg.play_next = 0;
								p_dbg("next");
								break;
						    }

							goto end;
					    }
					    else
						{
						    f_lseek(&file, 0);	//
						    init_mp3_format();	//初始libmad结构
						    reset_mp3_stat();		//
						    while(1) //开始读取文件播放
						    {
							    if(!audio_cfg.playing_mp3)
								    goto end;
							
							    if(audio_cfg.play_next){
								    //audio_cfg.play_next = 0;
								    p_dbg("next");
								    break;
							    }
							    if(audio_cfg.pause)
							    {
								   sleep(1000);
								   continue;
							    }
								
							    res = f_read(&file, buff, 1024, &nb);	//一次读取1k内容
							    if(res){
								    p_err("f_read %d", res);
								    break;
							    }else
							    {
								    if(nb)
								    {
									   play_mp3_stream((uint8_t*)buff, nb);	//解码-播放
								    }else
									break;	//文件读取完毕,退出
							    }
							
						    }
						    deinit_mp3_format();	//
                            f_close(&file);
						    if(audio_cfg.play_next){
							    audio_cfg.play_next = 0;
							    p_dbg("next");
							    break;
						    }
					    }
					    sleep(1000);  //等待1秒,继续播放当前歌曲
					}
					sleep(1000);
				} 

				//printf("play music close DIR ret=[%d]\n", f_closedir(&dir));
			}	  
		}
            #endif
end:
		//mem_free(fileinfo.lfname);
		//thread_exit(thread_myself());
		//printf("delay some time\n");
            
	}

}

void start_mp3_thread()
{
#if !BAIDU_AI_VOICE_ENABLE
    thread_create(music_thread, 0, TASK_MUSIC_PRIO, 0, TASK_MUSIC_STACK_SIZE, "music_thread");
#endif
}
void test_play_mp3()
{
#if SUPPORT_AUDIO
    if(audio_cfg.playing_mp3) return;
	
	
	mp3play = mp3list;
	
	#if	SUPPORT_WEBSERVER
	if(mp3play)
        sprintf(Wcfg->music.play, "%s\0", mp3play->mp3name);
	#endif
	
    audio_cfg.playing_mp3 = 1;
#endif
}

void test_stop_mp3()
{
#if SUPPORT_AUDIO
	audio_cfg.playing_mp3 = 0;
#endif
}

void test_next_mp3()
{
#if SUPPORT_AUDIO
	audio_cfg.play_next = 1;
	
	if(mp3play)
	{
        mp3play = mp3play->next;
    }

	if(!mp3play) mp3play = mp3list;
	
	#if	SUPPORT_WEBSERVER
	if(mp3play)
        sprintf(Wcfg->music.play, "%s\0", mp3play->mp3name);
	#endif
#endif
}



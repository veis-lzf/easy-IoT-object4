#define DEBUG

#include "drivers.h"
#include "api.h"
#include "app.h"
#include "audio_low_level.h"

#include "audio/adpcm.h"
#include "audio/audio.h"
#include "debug.h"
#include "mad.h"

#include "ff.h"
#if SUPPORT_AUDIO
#define ENABLE_MP3	1

extern struct WAVE_FORMAT_HEAD pcm_format;
#if ENABLE_MP3
extern struct MP3_FORMAT mp3_format;
#endif
struct AUDIO_CFG audio_cfg =
{
	0, 0,
};

extern struct AUDIO_DEC_INPUT dec_input;
extern struct AUDIO_DEC_OUTPUT dec_output;
extern struct AUDIO_ADC adc;


void pause_mp3(BOOL autoresume);
void continue_mp3(void);

/*
初始化
 */
void audio_dev_init()
{
	memset(&audio_cfg, 0, sizeof(struct AUDIO_CFG));
	//memset(&pcm_format, 0, sizeof(struct WAVE_FORMAT_HEAD));
#if ENABLE_MP3
	memset(&mp3_format, 0, sizeof(struct MP3_FORMAT));
#endif
	//pcm_format.sync = false;
	//audio_cfg.pcm = &pcm_format;

	audio_cfg.dec_input = &dec_input;
	audio_cfg.dec_output = &dec_output;
	audio_cfg.adc = &adc;
	audio_cfg.volume = 100;
	return ;
}

void speex_init(void)     
{

	audio_cfg.complexity = 1;
	audio_cfg.vbr = 0;
	audio_cfg.enh = 1;
	audio_cfg.quality = web_cfg.audio.audio_qua;
	
	/* Speex encoding initializations */ 
	speex_bits_init(&audio_cfg.enc_bits);
	audio_cfg.enc_state = speex_encoder_init(&speex_nb_mode);	//?-′??￡ê?
	speex_encoder_ctl(audio_cfg.enc_state, SPEEX_SET_VBR, &audio_cfg.vbr);	 //?é±??ù?ê
	speex_encoder_ctl(audio_cfg.enc_state, SPEEX_SET_QUALITY,&audio_cfg.quality);		  //?êá?2?êy
	speex_encoder_ctl(audio_cfg.enc_state, SPEEX_SET_COMPLEXITY, &audio_cfg.complexity);	//?′?ó?è?μ


	/* speex decoding intilalization */
	speex_bits_init(&audio_cfg.dec_bits);
	audio_cfg.dec_state = speex_decoder_init(&speex_nb_mode);		//?-′??￡ê?
	speex_decoder_ctl(audio_cfg.dec_state, SPEEX_SET_ENH, &audio_cfg.enh);  //?a??D?????

	speex_encoder_ctl(audio_cfg.enc_state, SPEEX_GET_FRAME_SIZE, &audio_cfg.enc_frame_size);  
  	//speex_decoder_ctl(audio_cfg.dec_state, SPEEX_GET_FRAME_SIZE, &audio_cfg.dec_frame_size);  

	switch(audio_cfg.quality)
	{
		case 1:
			audio_cfg.dec_frame_size = 10;
			break;
		case 2:
			audio_cfg.dec_frame_size = 15;
			break;
		case 4:
			audio_cfg.dec_frame_size = 20;
			break;
		case 6:
			audio_cfg.dec_frame_size = 28;
			break;
		case 8:
			audio_cfg.dec_frame_size = 38;
			break;
	}
  	
	p_dbg("enc_frame_size:%d, dec_frame_size:%d", audio_cfg.enc_frame_size, audio_cfg.dec_frame_size);
	speex_bits_reset(&audio_cfg.dec_bits);
	speex_bits_reset(&audio_cfg.enc_bits);

	
}

void speex_deinit(void)
{
	if(audio_cfg.enc_state){
		speex_encoder_destroy(audio_cfg.enc_state);
		audio_cfg.enc_state = 0;
	}
	
	if(audio_cfg.dec_state){
		speex_decoder_destroy(audio_cfg.dec_state);
		audio_cfg.dec_state = 0;
	}

	speex_bits_destroy(&audio_cfg.enc_bits);
}

int iot_speex_encode(uint16_t *in_buff, int len/*short unit*/, uint8_t *out_buff, int out_buff_size/*byte unit*/)
{
	int i, ret;
	int frames = len/ audio_cfg.enc_frame_size/*160*/;
	if(frames == 0)
		return 0;

	//p_dbg_enter;

	speex_bits_reset(&audio_cfg.enc_bits);
	for(i = 0; i < frames; i++)
	{
		speex_encode_int(audio_cfg.enc_state, (spx_int16_t*)in_buff + i*audio_cfg.enc_frame_size, &audio_cfg.enc_bits);
	}
	ret = speex_bits_write(&audio_cfg.enc_bits, (char *)out_buff, out_buff_size);
	//p_dbg("encode ret:%d", ret);
	
	return ret;
}

int iot_speex_decode(uint8_t *in_buff, int len/*byte unit*/, uint16_t *out_buff, int out_buff_size/*byte unit*/)
{
	int i, ret, out_size = 0;
	int frames = len/ audio_cfg.dec_frame_size;
	if(frames == 0)
		return 0;

	//p_dbg_enter;
	for(i = 0; i < frames; i++)
	{
		//speex_bits_reset(&audio_cfg.dec_bits);
		speex_bits_read_from(&audio_cfg.dec_bits, (char *)in_buff + i*audio_cfg.dec_frame_size, audio_cfg.dec_frame_size);
		ret = speex_decode_int(audio_cfg.dec_state, &audio_cfg.dec_bits, (spx_int16_t*)out_buff);
		out_buff += ret;
		out_buff_size -= ret;
		out_size += ret;
		if(out_buff_size <= 0)
			break;
	}
	p_dbg("decode size;%d", out_size);
	return out_size;
}
/*
打开音频驱动
 */
int audio_dev_open()
{
	p_dbg_enter;
	audio_cfg.audio_dev_open = 0;

	if (dac_low_level_open())        //音频输出
	{
		p_err("%s err", __FUNCTION__);
		return  - 1;
	}

	if (adc_low_level_open())      //音频输入
	{
		p_err("%s err1", __FUNCTION__);
		return  - 1;
	}
	
	//speex_init();    //don't enable speex
	#if BAIDU_AI_VOICE_ENABLE    //init a queue for baidu ai service
	msgget(&audio_cfg.ai_queue, 5);                                    //创建一个消息队列   audio_cfg.ai_queue  
    #endif
	audio_cfg.audio_dev_open = 1;

	return 0;
}



/*
关闭音频驱动
 */
int audio_dev_close()
{
	audio_cfg.audio_dev_open = 0;
	//speex_deinit();
	#if BAIDU_AI_VOICE_ENABLE 
	msgfree(audio_cfg.ai_queue);                //释放队列中的消息    audio_cfg.ai_queue   
    #endif
	dac_low_level_close();                      //关闭DAC
	adc_low_level_close();                      //关闭ADC
	return 0;
}

int play_speex_stream(uint8_t *buff, int size);
int handle_audio_stream(unsigned char *buff, int size)
{
	return play_speex_stream(buff, size);
}

int16_t speex_out_buff[SPEEX_ENC_FRAME_SIZE];
int16_t stereo_out[16];
int play_speex_stream(uint8_t *buff, int size)
{
	int frame_size = /*audio_cfg.dec_frame_size*/SPEEX_DEC_FRAME_SIZE;
	int frames = size/frame_size;
	int i,j,k;		

	pause_mp3(1);
	for(i = 0; i < frames; i++){

		
		speex_bits_read_from(&audio_cfg.dec_bits, (char *)buff + frame_size*i, frame_size);
		speex_decode_int(audio_cfg.dec_state, &audio_cfg.dec_bits, (spx_int16_t*)speex_out_buff);
		
		for(j = 0,k = 0;j < SPEEX_ENC_FRAME_SIZE; j++)
		{
			//speex_out_buff[j] =  speex_out_buff[j]/16;
			stereo_out[k] = speex_out_buff[j]/16 + 2048;
			/*
			if(stereo_out[k*2] >= 4096 || stereo_out[k*2] < 0)
				stereo_out[k*2] = stereo_out[k*2 + 1] = last_value;
			else
				stereo_out[k*2 + 1] = last_value = stereo_out[k*2];
			*/
			k++;
			if(k == 16){
				k = 0;
				push_to_play_buff((uint8_t *)stereo_out, 2*16);
			}
		}
	}

	return 0;
}

#if 0
int play_pcm_stream(uint8_t *buff, int size)
{
	if (size > 1024 || !audio_cfg.dec_input)
	//size 肯定 <= 1024
		return  - 1;

	if (audio_cfg.dec_input->data_len + size <= audio_cfg.dec_input->buff_len)
	{
		memcpy(audio_cfg.dec_input->data + audio_cfg.dec_input->data_len, buff, size);
		audio_cfg.dec_input->data_len += size;
	}
	else
	{
		audio_cfg.dec_input->data_len = 0;
	}

	return do_play_pcm_stream();
}
#endif
#if 1

int play_mp3_stream(uint8_t *buff, int size)     //dac输出数据   buff
{
	int ret, remain = size;
	if(
#if ENABLE_MP3
	!mp3_format.init || 
#endif
	!buff)
		return -1;
	
again:	
	if(dec_input.data_len + remain >  dec_input.buff_len)           //dec_input.buff_len = AUDIO_DAC_INPUT_BUFF_SIZE;   3200
	{
		size = dec_input.buff_len - dec_input.data_len;           //dec_input.data_len = 0;    一开始为0    超过3200后的数据先不接收
	}else
		size = remain;                                            //数据没有3200长

	if(size == 0){
		dec_input.data_len = 0;
		p_err("size == 0");
		return 0;
	}
	
	memcpy(dec_input.data + dec_input.data_len, buff, size);
	dec_input.data_len += size;                                //dec_input.data_len  ：加上size
	
	remain -= size;                                              //减去播放的数据长度

#if ENABLE_MP3
	ret = MpegAudioDecoder(&dec_input);      //dac输出数据   dec_input
	#else
	ret = 0;
#endif
	
	if(remain)                                                  ////减去播放的数据长度,直到全部播放完毕
		goto again;
	
	return ret;
}
#endif

#if 0
void switch_audio_mode()
{
	if(!audio_cfg.audio_dev_open)
	{
		p_dbg("请先打开音频");
		return;
	}
	audio_cfg.play_mp3 = !audio_cfg.play_mp3;
	if(audio_cfg.play_mp3){

		p_dbg("mp3播放模式");
#if ENABLE_MP3
		if(!mp3_format.init)
		{
			if(init_mp3_format())
				p_err("init_mp3_format err\n");
		}
		reset_mp3_stat();
#endif
	}else{
		p_dbg("pcm播放模式(语音对讲)");
#if ENABLE_MP3
		deinit_mp3_format();
#endif
	}
}
#endif

int is_data_equ(uint8_t* data, uint8_t value, int len)
{
	int i;
	for(i = 0 ;i < len ;i++)
	{
		if(data[i] != value)
			return 0;
	}

	return 1;
}

void playback_record_audio()
{
	int i, frames;
	uint8_t *tmp_buf;
	p_dbg_enter;

	if(audio_cfg.recording_to_flash_addr <= AUDIO_RECORD_BASE_ADDR)
	{
		p_err("no record data");
		return;
	}

	frames = (audio_cfg.recording_to_flash_addr - AUDIO_RECORD_BASE_ADDR)/(SPEEX_DEC_FRAME_SIZE*5);

	tmp_buf = (uint8_t*)mem_malloc((SPEEX_DEC_FRAME_SIZE*5));
	if(!tmp_buf)
		return;
	
	p_dbg("frame:%d", frames);
	for(i = 0; i < frames; i++)
	{
		m25p80_read(AUDIO_RECORD_BASE_ADDR + i*(SPEEX_DEC_FRAME_SIZE*5), (SPEEX_DEC_FRAME_SIZE*5), tmp_buf);

		play_speex_stream(tmp_buf, (SPEEX_DEC_FRAME_SIZE*5));
	}

	mem_free(tmp_buf);
}

void record_to_flash_timeout_cb(void *arg)            //回调函数
{
	p_dbg_enter;
	audio_cfg.recording_to_flash = FALSE;
	
	timer_free(audio_cfg.recording_to_flash_timer);     //删除这个定时任务
	audio_cfg.recording_to_flash_timer = NULL;

	//send_cmd_to_self('l'); //回放命令
    update_ai_event();                         //释放信号量  ai_audio_wait
//    IND2_OFF;
	p_dbg("stop record_to_flash");
}

/*
*录音到本地flash，20秒后超时
*/
void audio_record_to_flash()
{
	p_dbg_enter;

	if(audio_cfg.recording_to_flash)
	{
		p_err("record in progress");
		return;
	}

	audio_cfg.recording_to_flash = TRUE;

	m25p80_erase(AUDIO_RECORD_BASE_ADDR, AUDIO_RECORD_ROOM_SIZE);            //擦除录音在flash的最后32k空间
	audio_cfg.recording_to_flash_addr = AUDIO_RECORD_BASE_ADDR;               //录音保存在flash的最后32k空间               
	audio_cfg.recording_to_flash_timer = timer_setup(100, 0, record_to_flash_timeout_cb, 0);     //创建一个单次定时器
	if(audio_cfg.recording_to_flash_timer)
		mod_timer(audio_cfg.recording_to_flash_timer, 2*1000);                        //改变定时时间  系统节拍为10ms,这里改为20S
}

DECLARE_MONITOR_ITEM("pcm_push_cnt", pcm_push_cnt);
//拷贝到dac发送缓冲区,如满了则延时等待
void push_to_play_buff(uint8_t *val, int size)                    //拷贝到dac发送缓冲区,如满了则延时等待
{
	int ret;
	uint32_t irq_flg;
	if (!val || !size || !audio_cfg.dec_output)
		return ;

	if (!audio_cfg.audio_dev_open || !audio_cfg.dec_output->data)
	{
		return ;
	}
	mutex_lock(audio_cfg.push_mutex);     //等待一个互斥信号
    
again: 
	irq_flg = local_irq_save();
	dec_output.need_wait = 0;
	dec_output.play_delay_cnt = 0;
	if (dec_output.int_flag < 2)
	{
		if ((dec_output.write_pos + size) <= AUDIO_DMA_BUFF_SIZE / 2)
		{

			memcpy(dec_output.data + dec_output.int_flag *(AUDIO_DMA_BUFF_SIZE / 2) + dec_output.write_pos, val, size);
			dec_output.write_pos += size;
			INC_MONITOR_VALUE(pcm_push_cnt);
		}
		else
		{
			clear_wait_event(dec_output.dac_tx_waitq);   
			dec_output.need_wait = 1;
		}
	}
	local_irq_restore(irq_flg);

	if (dec_output.need_wait)
	{
		ret = wait_event_timeout(dec_output.dac_tx_waitq, 2000);          //等待信号量    dec_output.dac_tx_waitq
		if (ret == WAIT_EVENT_TIMEOUT)
		{
			p_err("dac_tx_waitq timeout");
		}
		else
		{

			goto again;
		}
	}
	mutex_unlock(audio_cfg.push_mutex);
}

void pause_mp3(BOOL autoresume)
{
	if(audio_cfg.pause){
		audio_cfg.auto_resume = autoresume;
		del_timer(audio_cfg.mp3_delay_timer);
		add_timer(audio_cfg.mp3_delay_timer);
		return;
	}

	audio_cfg.pause = 1;
	audio_cfg.auto_resume = autoresume;
	dac_switch_samplerate(8000);
	del_timer(audio_cfg.mp3_delay_timer);
	add_timer(audio_cfg.mp3_delay_timer);
}

void continue_mp3(void)
{
	if(!audio_cfg.pause)
		return;

	audio_cfg.pause = 0;
	audio_cfg.auto_resume = 0;
	dac_switch_samplerate(audio_cfg.dec_input->samplerate);
}

void mp3_delay_timer_cb(void *arg)
{
	if(audio_cfg.pause && audio_cfg.auto_resume)
		continue_mp3();
	
}

void open_audio()
{
	int ret;

    if(audio_cfg.audio_dev_open)
        audio_dev_close(); 
    
	ret = audio_dev_open();
	if (ret != 0)
	{
		p_err("open err");
		return ;
	}

	audio_cfg.volume = 100;                                                //音量
	audio_cfg.dec_input->samplerate = web_cfg.audio.audio_fre;
	audio_cfg.adc->samplerate = web_cfg.audio.audio_fre;                   //AUDIO_SAMPLERATE 16000;    samplerate   采样率

	audio_cfg.recording_to_flash_addr = AUDIO_RECORD_BASE_ADDR;            //录音保存在flash的最后32k空间

	adc_switch_samplerate(audio_cfg.adc->samplerate);                      //adc改变采样率
	dac_switch_samplerate(audio_cfg.dec_input->samplerate);                 //改变dac的采样率

	//init_mp3_format();
	//reset_mp3_stat();
	audio_cfg.push_mutex = mutex_init("");                                  //建立一个互斥量
	audio_cfg.mp3_delay_timer = timer_setup(1000, 0, mp3_delay_timer_cb, 0);        
	

}
#endif


#ifndef _AUDIO_LOW_H
#define _AUDIO_LOW_H


//使用片内DAC或者片外I2S开关
#define USE_I2S_MODE	0	//I2S和camera接口有冲突，所以默认只使用dac, I2S驱动不再维护

#define AUDIO_DAC_DMA 	DMA1_Stream5

#define AUDIO_SAMPLERATE8000			8000
#define AUDIO_SAMPLERATE16000			16000
#define AUDIO_SAMPLERATE32000			32000
#define AUDIO_SAMPLERATE44100			44100
#define AUDIO_SAMPLERATE48000			48000

#define DEFAULT_PLAY_SAMPLERATE		AUDIO_SAMPLERATE32000
#define DEFAULT_RECORD_SAMPLERATE		AUDIO_SAMPLERATE32000

int get_ai_audio_pcm_data_len(void);
int get_ai_audio_pcm_data(int offset, int len, char* data);
int send_and_encode_text_event(char * data, int len);
int get_voice_ai_event(char ** data);
int dac_low_level_open(void);
void dac_low_level_close(void);
int adc_low_level_open(void);
void adc_low_level_close(void);
int dac_switch_samplerate(int val);
int adc_switch_samplerate(int val);
void dac_close_channel(int channel);
void audio_button_event(int event);

void dac_close(void);
int dac_open(void);

#endif


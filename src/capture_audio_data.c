/*
 进行音频采集，采集pcm数据并直接保存pcm数据
 音频参数： 
	 声道数：		2
	 采样位数：	16bit、LE格式
	 采样频率：	44100Hz
*/
#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <signal.h>
#include <pthread.h>
#include "capture_audio_data.h"


FILE *pcm_data_file=NULL;
int buffer_frames;
snd_pcm_t *capture_handle;
snd_pcm_format_t format=SND_PCM_FORMAT_S16_LE;
struct audio_capture_buffers* audio_buff;
extern struct audio_capture_buffers* audio_encode_buff;
extern struct audio_capture_buffers* audio_encode_buff_local;
extern struct audio_capture_buffers* audio_encode_buff_free;//用来释放节点

int capture_audio_data_init( char *audio_dev,char* pcm_filename);
int capture_audio_data(snd_pcm_t *capture_handle,int buffer_frames);

int capture_audio_data_init( char *audio_dev,char* pcm_filename)
{
	int i;
	int err;
	
	buffer_frames = 128;
	unsigned int rate = 22050;// 常用的采样频率: 44100Hz 、16000HZ、8000HZ、48000HZ、22050HZ
	capture_handle;// 一个指向PCM设备的句柄
	snd_pcm_hw_params_t *hw_params; //此结构包含有关硬件的信息，可用于指定PCM流的配置
	
	/*注册信号捕获退出接口*/
	printf("进入main\n");
	/*PCM的采样格式在pcm.h文件里有定义*/
	format=SND_PCM_FORMAT_S16_LE; // 采样位数：16bit、LE格式
	
	/*打开音频采集卡硬件，并判断硬件是否打开成功，若打开失败则打印出错误提示*/
	
	if ((err = snd_pcm_open (&capture_handle, audio_dev,SND_PCM_STREAM_CAPTURE,0))<0) 
	{
		printf("无法打开音频设备: %s (%s)\n",  audio_dev,snd_strerror (err));
		exit(1);
	}
	printf("音频接口打开成功.\n");
	
	/*创建一个保存PCM数据的文件*/
	if((pcm_data_file = fopen(pcm_filename, "wb")) == NULL)
	{
		printf("无法创建%s音频文件.\n",pcm_filename);
		exit(1);
	} 
	printf("用于录制的音频文件已打开.\n");
 
	/*分配硬件参数结构对象，并判断是否分配成功*/
	if((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) 
	{
		printf("无法分配硬件参数结构 (%s)\n",snd_strerror(err));
		exit(1);
	}
	printf("硬件参数结构已分配成功.\n");
	
	/*按照默认设置对硬件对象进行设置，并判断是否设置成功*/
	if((err=snd_pcm_hw_params_any(capture_handle,hw_params)) < 0) 
	{
		printf("无法初始化硬件参数结构 (%s)\n", snd_strerror(err));
		exit(1);
	}
	printf("硬件参数结构初始化成功.\n");
 
	/*
		设置数据为交叉模式，并判断是否设置成功
		interleaved/non interleaved:交叉/非交叉模式。
		表示在多声道数据传输的过程中是采样交叉的模式还是非交叉的模式。
		对多声道数据，如果采样交叉模式，使用一块buffer即可，其中各声道的数据交叉传输；
		如果使用非交叉模式，需要为各声道分别分配一个buffer，各声道数据分别传输。
	*/
	if((err = snd_pcm_hw_params_set_access (capture_handle,hw_params,SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) 
	{
		printf("无法设置访问类型(%s)\n",snd_strerror(err));
		exit(1);
	}
	printf("访问类型设置成功.\n");
 
	/*设置数据编码格式，并判断是否设置成功*/
	if ((err=snd_pcm_hw_params_set_format(capture_handle, hw_params,format)) < 0) 
	{
		printf("无法设置格式 (%s)\n",snd_strerror(err));
		exit(1);
	}
	fprintf(stdout, "PCM数据格式设置成功.\n");
 
	/*设置采样频率，并判断是否设置成功*/
	if((err=snd_pcm_hw_params_set_rate_near (capture_handle,hw_params,&rate,0))<0) 
	{
		printf("无法设置采样率(%s)\n",snd_strerror(err));
		exit(1);
	}
	printf("采样率设置成功\n");
 
	/*设置声道，并判断是否设置成功*/
	if((err = snd_pcm_hw_params_set_channels(capture_handle, hw_params,1)) < 0) 
	{
		printf("无法设置声道数(%s)\n",snd_strerror(err));
		exit(1);
	}
	printf("声道数设置成功.\n");
 
	/*将配置写入驱动程序中，并判断是否配置成功*/
	if ((err=snd_pcm_hw_params (capture_handle,hw_params))<0) 
	{
		printf("无法向驱动程序设置参数(%s)\n",snd_strerror(err));
		exit(1);
	}
	printf("参数设置成功.\n");
	/*使采集卡处于空闲状态*/
	snd_pcm_hw_params_free(hw_params);
 
	/*准备音频接口,并判断是否准备好*/
	if((err=snd_pcm_prepare(capture_handle))<0) 
	{
		printf("无法使用音频接口 (%s)\n",snd_strerror(err));
		exit(1);
	}
	printf("音频接口准备好.\n");
	
	audio_buff=(struct audio_capture_buffers*)malloc(sizeof(struct audio_capture_buffers));//创建一个结构体数组
	//audio_buff->next=NULL;
	audio_buff->flag=-2;//表示程序开始时的第一个节点
	audio_encode_buff=audio_buff;//保存头节点
	audio_encode_buff_local=audio_buff;
	audio_encode_buff_free=audio_buff;
	printf("缓冲区分配成功.\n");

	return 0;
}

int capture_audio_data(snd_pcm_t *capture_handle,int buffer_frames)
{
	int err;
	int i;
	const int frame_num=4;
	int a_audio_frame_size=128*snd_pcm_format_width(format)/8*2;//一帧音频数据的大小512 byte

	//因为frame样本数固定为1024,而双通道，每个采样点2byte，所以一次要发送1024*2*2=4096 byte数据给frame->data[0];

	/*开始采集音频pcm数据*/
	printf("开始采集数据...\n");
	while(1)
	{
		/*创建缓冲队列*/
		audio_buff->next=(struct audio_capture_buffers*)malloc(sizeof(struct audio_capture_buffers));//创建链表节点
		audio_buff=audio_buff->next;
		audio_buff->flag=0;
		audio_buff->next=NULL;
		audio_buff->buff=malloc(frame_num*a_audio_frame_size);
		for(i=0;i<frame_num;i++)
		{
			if((err=snd_pcm_readi(capture_handle,audio_buff->buff+a_audio_frame_size*i,buffer_frames))!=buffer_frames) 
			{
				printf("从音频接口读取失败(%s)\n",snd_strerror(err));
				exit(1);
			}
		}
		audio_buff->flag=1;//表示已经读取数据完成
		
		/*写数据到文件*/
		fwrite(audio_buff->buff,a_audio_frame_size*frame_num,1,pcm_data_file);
	}
 
 
	/*关闭音频采集卡硬件*/
	snd_pcm_close(capture_handle);
 
	/*关闭文件流*/
	fclose(pcm_data_file);

	return 0;
}


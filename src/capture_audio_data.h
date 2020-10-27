#ifndef _CAPTURE_AUDIO_DATA_H_
#define _CAPTURE_AUDIO_DATA_H_

struct audio_capture_buffers//缓冲队列结构体
{
	int flag;
	char* buff;
	struct audio_capture_buffers* next;//指向下一节点的指针
	
};

#endif

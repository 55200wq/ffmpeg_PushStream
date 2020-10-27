cc = gcc

x264_lib = /home/wq/work/software/x264-master/_INSTALL/lib
x264_include = /home/wq/work/software/x264-master/_INSTALL/include
ffmpeg_lib = ${FFMPEG_PATH}/lib
ffmpeg_include = ${FFMPEG_PATH}/include
alsa_lib = /home/wq/work/software/alsa-lib-1.2.4/_INSTALL/lib
alsa_include = /home/wq/work/software/alsa-lib-1.2.4/_INSTALL/include

cflags = -I${ffmpeg_include} -L${ffmpeg_lib} -I${x264_include} -L${x264_lib} -I${alsa_include} -L${alsa_lib}
libs = -lavcodec -lavfilter -lavutil -lswresample -lavdevice -lavformat -lpostproc -lx264 -lm -lpthread -lasound -lswscale
obj = ffmpeg_encode_video_audio.o capture_audio_data.o Image_color_conversion.o ASCII_8_16.o
app:${obj}
	${cc} $^ -o $@ ${cflags} ${libs}
%.o: %.c
	$(CC) -c $< -o $@ ${cflags} ${libs}

.PHONY:clean
clean:
	rm ./*.o -f

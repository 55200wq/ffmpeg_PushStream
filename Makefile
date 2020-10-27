ffmpeg_lib = ${FFMPEG_PATH}/lib
ffmpeg_include = ${FFMPEG_PATH}/include
all:
	echo ${ffmpeg_lib}
	echo ${ffmpeg_include}
	make -C src/
.PHONY:clean
clean:
	make -C src/ clean


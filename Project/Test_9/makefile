	common_sources=	project.cpp
	
	common_includes =
		
	opencv_flags =	-L/usr/lib	-lopencv_core	-lopencv_flann	-lopencv_video 

	common_flags =	-g \
			-pg \
			-O0 \
			-pthread \
			-lm 

	CC=g++
	flags =	$(common_flags) 

	sources =	$(common_sources)
	includes =	$(common_includes)
	objects:= $(sources:.cpp=.o)
	deps:= $(sources:.cpp=.d)	

.PHONY:	compile-all \
	clean \
	build \
	video

build:	project.elf	

%.elf:%.o
	$(CC)	$(flags)	$(includes)	-o	$@	$<	-lrt	`pkg-config --libs opencv`	$(opencv_flags)	

%.i:%.cpp	
	$(CC)	$(flags)	$(includes)	-S	$<	-o	$@

%.asm:%.cpp	
	$(CC)	$(flags)	$(includes)	-S	$<	-o	$@

%.o:%.cpp	
	$(CC)	$(flags)	$(includes)	-c	$<	-o	$@	-lrt

%.o:%.s	
	$(CC)	$(flags)	$(includes)	-c	$<	-o	$@

%.d:%.cpp	
	$(CC)	$(flags)	$(includes)	-M	$<	-o	$@	

%.d:%.s	
	$(CC)	$(flags)	$(includes)	-M	$<	-o	$@

compile-all:	$(objects) 

clean :
	rm	-f	*.o	*.s	*.i	*.o	*.d	*.elf *.swp	*.ppm	*.jpg	*.out

video :
	ffmpeg	-r	1	-pattern_type	glob	-i	'frame_*.ppm'	-c:v	libx264	timelapse.avi

	common_sources=	project.cpp \
			thread_operations.cpp 
			
	common_includes =	thread_operations.hpp
		
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
	build 

build:	project.elf	

project.elf:
	$(CC)	$(flags)	-o	$@	$(sources)	$(includes)	-lrt	`pkg-config --libs opencv`	$(opencv_flags)	

%.elf:%.o	$(includes)	
	$(CC)	$(flags)	-o	$@	$<	-lrt	`pkg-config --libs opencv`	$(opencv_flags)	

%.i:%.cpp	
	$(CC)	$(flags)	$(includes)	-S	$<	-o	$@

%.asm:%.cpp	
	$(CC)	$(flags)	$(includes)	-S	$<	-o	$@

%.o:%.cpp	
	$(CC)	$(flags)	$(includes)	$<	-o	$@	-lrt

%.o:%.s	
	$(CC)	$(flags)	$(includes)	$<	-o	$@

%.d:%.cpp	
	$(CC)	$(flags)	$(includes)	-M	$<	-o	$@	

%.d:%.s	
	$(CC)	$(flags)	$(includes)	-M	$<	-o	$@

compile-all:	$(objects) 

clean :
	rm	-f	*.o	*.s	*.i	*.o	*.d	*.elf *.swp

	common_sources=	capture.cpp
	
	common_includes =
		
	opencv_flags

	common_flags =	-g \
			-O0 \
			-pthread \
			-lm 

	CC=g++
	flags =	$(common_flags)
	sources =	$(common_sources)
	includes =	$(common_includes)
	objects:= $(sources:.c=.o)
	deps:= $(sources:.c=.d)	

.PHONY:	compile-all \
	clean \
	build 

build:	capture.elf	

%.elf:%.o
	$(CC)	$(flags)	$(includes)	-o	$@	$<	-lrt

%.i:%.c	
	$(CC)	$(flags)	$(includes)	-S	$<	-o	$@

%.asm:%.c	
	$(CC)	$(flags)	$(includes)	-S	$<	-o	$@

%.o:%.c	
	$(CC)	$(flags)	$(includes)	-c	$<	-o	$@	-lrt

%.o:%.s	
	$(CC)	$(flags)	$(includes)	-c	$<	-o	$@

%.d:%.c	
	$(CC)	$(flags)	$(includes)	-M	$<	-o	$@	

%.d:%.s	
	$(CC)	$(flags)	$(includes)	-M	$<	-o	$@

compile-all:	$(objects) 

clean :
	rm	-f	*.o	*.s	*.i	*.o	*.d	*.elf *.swp

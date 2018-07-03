	common_sources=	pthread3ok.c \
			deadlock.c \
			deadlock_timeout.c \
			pthread3.c
	
	common_includes =

	common_flags =	-g \
			-O0 \
			-lpthread\
			-pthread \
			-lrt \
			-DLINUX

	CC=gcc
	flags =	$(common_flags)
	sources =	$(common_sources)
	includes =	$(common_includes)
	objects:= $(sources:.c=.o)
	deps:= $(sources:.c=.d)	

.PHONY:	compile-all \
	clean \
	build 

build:	pthread3ok.elf	pthread3.elf	deadlock.elf	deadlock_timeout.elf

%.elf:%.o
	$(CC)	$(flags)	$(includes)	-o	$@	$<	

%.i:%.c	
	$(CC)	$(flags)	$(includes)	-S	$<	-o	$@

%.asm:%.c	
	$(CC)	$(flags)	$(includes)	-S	$<	-o	$@

%.o:%.c	
	$(CC)	$(flags)	$(includes)	-c	$<	-o	$@

%.o:%.s	
	$(CC)	$(flags)	$(includes)	-c	$<	-o	$@

%.d:%.c	
	$(CC)	$(flags)	$(includes)	-M	$<	-o	$@	

%.d:%.s	
	$(CC)	$(flags)	$(includes)	-M	$<	-o	$@

compile-all:	$(objects) 

clean :
	rm	-f	*.o	*.s	*.i	*.o	*.d	*.elf *.swp

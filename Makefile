CFLAGS = -D_POSIX_SOURCE -Wall -g -I. -std=c99 

all: reorg cruncher

reorg: structs.h reorg.c 
	gcc ${CFLAGS} -o reorg reorg.c 

cruncher: structs.h cruncher.c
	gcc ${CFLAGS} -o cruncher cruncher.c 

clean:
	rm reorg cruncher

CFLAGS = -D_POSIX_SOURCE -Wall -g -I. -std=c99 

all: reorg reorg2 cruncher

reorg: structs.h reorg.c 
	gcc ${CFLAGS} -o reorg reorg.c 
reorg2: structs.h reorg2.c 
	gcc ${CFLAGS} -o reorg2 reorg2.c 
cruncher: structs.h cruncher.c
	gcc ${CFLAGS} -o cruncher cruncher.c 

clean:
	rm reorg reorg2 cruncher

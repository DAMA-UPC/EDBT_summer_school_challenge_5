

all: reorg

reogr: reorg.h reorg.c 
	g++ -c reorg.c -o reorg -I. 

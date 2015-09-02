

all: reorg

reorg: reorg.h reorg.c 
	gcc -g -I. -o reorg reorg.c 

clean:
	rm reorg

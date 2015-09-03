

all: reorg

reorg: structs.h reorg.c 
	gcc -Wall -O3 -I. -o reorg reorg.c 

clean:
	rm reorg

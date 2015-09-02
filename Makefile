

all: reorg

reorg: structs.h reorg.c 
	gcc -g -I. -o reorg reorg.c 

clean:
	rm reorg

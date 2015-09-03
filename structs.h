

#ifndef REORG_H
#define REORG_H

typedef struct {
	unsigned long person_id;
	unsigned short birthday;
	unsigned short location;
	unsigned long knows_first;
	unsigned short knows_n;
	unsigned long interest_first;
	unsigned short interest_n;
} PersonIn;

typedef struct {
  unsigned short  birthday;
  unsigned short  location;
  unsigned int    knows_first;
  unsigned int    interest_first;
} Person;

typedef struct{
	//Query.id|A1|A2|A3|A4|D1|D2
	unsigned short  query_id;
	unsigned short a1;
	unsigned short a2;
	unsigned short a3;
	unsigned short a4;
	unsigned short d1;
	unsigned short d2;
}Query;

typedef struct {
  unsigned int first;
  unsigned short n;
} Interest;

typedef struct {
  unsigned int first;
  unsigned short n;
} Knows;

typedef struct {
	int query_id;
	int score;
	unsigned long p;
	unsigned long f;
} Result;

#endif

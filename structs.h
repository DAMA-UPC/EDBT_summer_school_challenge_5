

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

typedef struct {
  unsigned int first;
  unsigned short n;
} Interest;

typedef struct {
  unsigned int first;
  unsigned short n;
} Knows;

/*typedef struct {
	unsigned long person_id;
	unsigned long knows_id;
	unsigned char score;
} Result;
*/

#endif

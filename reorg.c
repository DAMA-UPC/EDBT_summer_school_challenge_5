

#include "reorg.h"
#include <stdlib.h>
#include <stdio.h>


int main( int argc, char** argv ) 
{
	if(argc != 2) {
		printf("Invalid usage\n");
	}
	char persons_file_name[256];
	char knows_file_name[256];
	sprintf(persons_file_name, "%s/%s", argv[1], "persons.bin");
	sprintf(knows_file_name, "%s/%s", argv[1], "knows.bin");

	FILE* persons_file = fopen(persons_file_name, "rb");
	if( !persons_file ) {
		fprintf(stderr, "ERROR opening %s\n", persons_file_name);
		exit(1);
	}

	FILE* knows_file = fopen(knows_file_name, "rb");
	if( !knows_file ) {
		fprintf(stderr, "ERROR opening %s\n", knows_file_name);
		exit(1);
	}

	fseek (persons_file, 0, SEEK_END);   // non-portable
	size_t persons_file_size = ftell (persons_file);
	long num_persons = persons_file_size / sizeof(PersonIn);

/*	PersonIn* persons_in  = (PersonIn*)malloc(persons_file_size);
	if( !persons_in ) {
		fprintf(stderr, "ERROR while creating persons array");
		exit(1);
	}
	fread(persons_in, sizeof(PersonIn), num_persons, persons_file);
	*/

	int i = 0;
	for( i = 0; i < num_persons; ++i) {
		PersonIn person;
		fread(&person, sizeof(PersonIn), 1, persons_file);
		if( i % 10000 == 0 ) {
			printf("Read %d persons. This person id %lu\n", i, person.person_id);
		}
	}

	//free(persons_in);
	fclose(persons_file);
	fclose(knows_file);
  return 0;
}

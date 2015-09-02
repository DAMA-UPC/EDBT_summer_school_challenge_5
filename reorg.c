

#include "reorg.h"
#include "stdin.h"
#include "stdlib.h"

int main( int argc, char** argv ) 
{
	char persons_file_name[256];
	char knows_file_name[256];
	sprintf(persons_file_name, "%s/%s", argv[1], "persons.bin");
	sprintf(knows_file_name, "%s/%s", argv[1], "knows.bin");

	FILE* persons_file = fopen(persons_file_name, "rb");
	FILE* knows_file = fopen(knows_file_name, "rb");

	fseek (persons_file, 0, SEEK_END);   // non-portable
	size_t persons_file_size = ftell (persons_file);
	long num_persons = persons_file_size / sizeof(PersonIn);

	PersonIn* persons_in  = (PersonIn*)malloc(persons_file_size);
	fread(persons_in, sizeof(PersonIn), num_pesons, persons_file);

	for( int i = 0; i < num_persons; ++i) {
		PersonIn* person = &persons_in[i];
		if( i % 10000 == 0 ) {
			printf("Read %d persons. This person id %lu\n", i, person->person_id);
		}
	}

	free(persons_in);
	fclose(persons_file);
	fclose(knows_file);
  return 0;
}



#include "structs.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


int main( int argc, char** argv ) 
{
  if(argc != 2) {
    printf("Invalid usage\n");
  }
  char persons_file_name[256];
  char knows_file_name[256];
  char out_persons_file_name[256];
  char out_knows_file_name[256];
  sprintf(persons_file_name, "%s/%s", argv[1], "person.bin");
  sprintf(knows_file_name, "%s/%s", argv[1], "knows.bin");
  sprintf(out_persons_file_name, "%s/%s", argv[1], "refined_person.bin");
  sprintf(out_knows_file_name, "%s/%s", argv[1], "refined_knows.bin");

  long num_knows = 0;
  long num_knows_location = 0;
  long num_filtered_knows = 0;

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

  FILE* out_persons_file = fopen(out_persons_file_name, "wb");
  if( !out_persons_file ) {
    fprintf(stderr, "ERROR opening %s\n", out_persons_file_name);
    exit(1);
  }

  FILE* out_knows_file = fopen(out_knows_file_name, "wb");
  if( !out_knows_file ) {
    fprintf(stderr, "ERROR opening %s\n", out_knows_file_name);
    exit(1);
  }

  fseek (persons_file, 0, SEEK_END);   // non-portable
  size_t persons_file_size = ftell (persons_file);
  long num_persons = persons_file_size / sizeof(PersonIn);
  fseek (persons_file, 0, SEEK_SET);   // non-portable

  PersonIn* persons_in  = (PersonIn*)malloc(persons_file_size);
  if( !persons_in ) {
    fprintf(stderr, "ERROR while creating persons array");
    exit(1);
  }
  fread(persons_in, sizeof(PersonIn), num_persons, persons_file);

  long knows_buffer_size1 = 5000;
  unsigned int* knows_buffer1 = (unsigned int*)malloc(sizeof(unsigned int)*knows_buffer_size1);
  long knows_buffer_size2 = 5000;
  unsigned int* knows_buffer2 = (unsigned int*)malloc(sizeof(unsigned int)*knows_buffer_size2);
  unsigned int num_out_knows = 0;
  int i = 0;
  for( ; i < num_persons; ++i) {
    PersonIn* person = &persons_in[i];
    if( i % 10000 == 0 ) {
      printf("Read %d persons. This person id %lu and city %lu\n", i, person->person_id,
          person->location);
    }

    fseek(knows_file, person->knows_first*sizeof(unsigned int), SEEK_SET );
    if( person->knows_n > knows_buffer_size1 ) {
      knows_buffer_size1 *= 2;
      knows_buffer1 = (unsigned int*)realloc(knows_buffer1, knows_buffer_size1);
    }
    fread(knows_buffer1, sizeof(unsigned int), person->knows_n, knows_file);

    unsigned int num_neighbors_found = 0;
    int j = 0;
    for( ;j < person->knows_n; ++j) {
      PersonIn* other = &persons_in[knows_buffer1[j]];
      num_knows++;
      if( other->location == person->location ) {
        num_knows_location++;

        fseek(knows_file, other->knows_first*sizeof(unsigned int), SEEK_SET );
        if( other->knows_n > knows_buffer_size2 ) {
          knows_buffer_size2 *= 2;
          knows_buffer2 = (unsigned int*)realloc(knows_buffer2, knows_buffer_size2);
        }
        fread(knows_buffer2, sizeof(unsigned int), other->knows_n, knows_file);

        int k = 0;
        for(; k < other->knows_n; ++k) {
          if( knows_buffer2[k] == i ) {
            num_neighbors_found++;
            break;
          }
        }

        if ( k < other->knows_n ) {
          num_filtered_knows++;
          fwrite(&knows_buffer1[j], sizeof(unsigned int), 1, out_knows_file);
          num_out_knows++;
        }
      }
    }
    PersonIn out_person;
    memcpy(&out_person, person, sizeof(PersonIn));
    out_person.knows_n = num_neighbors_found;
    out_person.knows_first = num_out_knows;
    fwrite(&out_person, sizeof(PersonIn), 1, out_persons_file);
  }
  free(knows_buffer1);
  free(knows_buffer2);

  printf("Number of knows: %lu\n", num_knows);
  printf("Number of knows location: %lu\n", num_knows_location);
  printf("Number of filtered knows: %lu\n", num_filtered_knows);
  printf("Percentage of filtered knows: %f\n", num_filtered_knows*100.0f/num_knows);

  free(persons_in);
  fclose(persons_file);
  fclose(knows_file);
  fclose(out_persons_file);
  fclose(out_knows_file);
  return 0;
}

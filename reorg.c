

#include "structs.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
  unsigned int person_index;
  unsigned int position;
} Tuple ;

PersonIn* persons_in;

FILE* in_persons_file; 
FILE* in_knows_file;
FILE* out_persons_file;
FILE* out_knows_file;
FILE* out_interest_file;
FILE* out_birthday_file;
FILE* out_location_file;
FILE* temp_knows_file;

int tuple_comparator( const void* a, const void* b) {
  int res = persons_in[((Tuple*)a)->person_index].birthday - persons_in[((Tuple*)b)->person_index].birthday;
  if( res ) return res;
  return ((Tuple*)a)->person_index - ((Tuple*)b)->person_index;
}


int main( int argc, char** argv ) {

  if(argc != 2) {
    printf("Invalid usage\n");
  }

  /** Filenames **/
  char in_persons_file_name[256];
  char in_knows_file_name[256];
  char out_persons_file_name[256];
  char out_knows_file_name[256];
  char out_interest_file_name[256];
  char out_birthday_file_name[256];
  char out_location_file_name[256];
  sprintf(in_persons_file_name, "%s/%s", argv[1], "person.bin");
  sprintf(in_knows_file_name, "%s/%s", argv[1], "knows.bin");
  sprintf(out_persons_file_name, "%s/%s", argv[1], "refined_person.bin");
  sprintf(out_knows_file_name, "%s/%s", argv[1], "refined_knows.bin");
  sprintf(out_interest_file_name, "%s/%s", argv[1], "refined_interest.bin");
  sprintf(out_birthday_file_name, "%s/%s", argv[1], "refined_birthday.bin");
  sprintf(out_location_file_name, "%s/%s", argv[1], "refined_location.bin");

  /** Statistics variables **/
  long num_knows = 0;
  long num_knows_location = 0;
  long num_out_knows = 0;
  long num_out_persons = 0;

  /** Openning input and output files **/
  in_persons_file = fopen(in_persons_file_name, "rb");
  if( !in_persons_file ) {
    fprintf(stderr, "ERROR opening %s\n", in_persons_file_name);
    exit(1);
  }

  in_knows_file = fopen(in_knows_file_name, "rb");
  if( !in_knows_file ) {
    fprintf(stderr, "ERROR opening %s\n", in_knows_file_name);
    exit(1);
  }

  out_persons_file = fopen(out_persons_file_name, "wb");
  if( !out_persons_file ) {
    fprintf(stderr, "ERROR opening %s\n", out_persons_file_name);
    exit(1);
  }

  out_knows_file = fopen(out_knows_file_name, "wb");
  if( !out_knows_file ) {
    fprintf(stderr, "ERROR opening %s\n", out_knows_file_name);
    exit(1);
  }

  out_interest_file = fopen(out_interest_file_name, "wb");
  if( !out_interest_file ) {
    fprintf(stderr, "ERROR opening %s\n", out_interest_file_name);
    exit(1);
  }

  out_birthday_file = fopen(out_birthday_file_name, "wb");
  if( !out_birthday_file ) {
    fprintf(stderr, "ERROR opening %s\n", out_birthday_file_name);
    exit(1);
  }

  out_location_file = fopen(out_location_file_name, "wb");
  if( !out_location_file ) {
    fprintf(stderr, "ERROR opening %s\n", out_location_file_name);
    exit(1);
  }

  temp_knows_file = fopen(".tmp_knows", "wb");
  if( !temp_knows_file ) {
    fprintf(stderr, "ERROR opening %s\n", ".tmp_knows");
    exit(1);
  }

  /** Computing number of persons **/
  fseek (in_persons_file, 0, SEEK_END);   
  size_t in_persons_file_size = ftell (in_persons_file);
  long num_persons = in_persons_file_size / sizeof(PersonIn);
  fseek (in_persons_file, 0, SEEK_SET);   

  /** Loading persons into memory **/
  persons_in  = (PersonIn*)malloc(in_persons_file_size);
  if( !persons_in ) {
    fprintf(stderr, "ERROR while creating persons array");
    exit(1);
  }
  fread(persons_in, sizeof(PersonIn), num_persons, in_persons_file);

  unsigned int* selected_persons_new_index = (unsigned int*) malloc(sizeof(unsigned int)*num_persons);
  memset(selected_persons_new_index,0xff, sizeof(unsigned int)*num_persons); 
  unsigned int* selected_persons_knows_start = (unsigned int*) malloc(sizeof(unsigned int)*num_persons);
  unsigned int* selected_persons_knows_n = (unsigned int*) malloc(sizeof(unsigned int)*num_persons);

  /** Creating knows buffers **/
  long knows_buffer_size1 = 5000;
  unsigned int* knows_buffer1 = (unsigned int*)malloc(sizeof(unsigned int)*knows_buffer_size1);
  long knows_buffer_size2 = 5000;
  unsigned int* knows_buffer2 = (unsigned int*)malloc(sizeof(unsigned int)*knows_buffer_size2);

  int i = 0;
  for( ; i < num_persons; ++i) {
    PersonIn* person = &persons_in[i];
    if( i % 10000 == 0 ) {
      printf("Read %d persons. This person id %lu and city %lu\n", i, person->person_id,
          person->location);
    }

    /** Resizing knows buffer 1 if necessary **/
    fseek(in_knows_file, person->knows_first*sizeof(unsigned int), SEEK_SET );
    if( person->knows_n > knows_buffer_size1 ) {
      knows_buffer_size1 *= 2;
      knows_buffer1 = (unsigned int*)realloc(knows_buffer1, knows_buffer_size1);
    }
    fread(knows_buffer1, sizeof(unsigned int), person->knows_n, in_knows_file);

    unsigned int num_neighbors_found = 0;
    int j = 0;
    for( ;j < person->knows_n; ++j) {
      PersonIn* other = &persons_in[knows_buffer1[j]];
      num_knows++;
      if( other->location == person->location ) {
        num_knows_location++;

        /** Resizing knows buffer 1 if necessary **/
        fseek(in_knows_file, other->knows_first*sizeof(unsigned int), SEEK_SET );
        if( other->knows_n > knows_buffer_size2 ) {
          knows_buffer_size2 *= 2;
          knows_buffer2 = (unsigned int*)realloc(knows_buffer2, knows_buffer_size2);
        }
        fread(knows_buffer2, sizeof(unsigned int), other->knows_n, in_knows_file);

        int k = 0;
        for(; k < other->knows_n; ++k) {
          if( knows_buffer2[k] == i ) {
            num_neighbors_found++;
            break;
          }
        }

        if ( k < other->knows_n ) {
          if( selected_persons_new_index[i] == 0xffffffff ) {
            selected_persons_new_index[i] = num_out_persons++; // we increment first because 0 is a reserved number
          }

          if( selected_persons_new_index[knows_buffer1[j]] == 0xffffffff ) {
            selected_persons_new_index[knows_buffer1[j]] = num_out_persons++; // we increment first because 0 is a reserved number
          }

          unsigned int new_index = selected_persons_new_index[knows_buffer1[j]]; // assigning new index to person
          fwrite(&new_index, sizeof(unsigned int), 1, temp_knows_file);
          num_out_knows++;
        }
      }
    }
    selected_persons_knows_start[i] = num_out_knows; // storing the new knows offset 
    selected_persons_knows_n[i] = num_out_knows - ( i != 0 ? selected_persons_knows_start[i-1] : 0); 
    
    /*PersonIn out_person;
    memcpy(&out_person, person, sizeof(PersonIn));
    out_person.knows_n = num_neighbors_found;
    out_person.knows_first = num_out_knows;
    fwrite(&out_person, sizeof(PersonIn), 1, out_persons_file);
    */

  }
  free(knows_buffer1);
  free(knows_buffer2);
  fclose(temp_knows_file);

  Tuple* selected_persons = (Tuple*)malloc(sizeof(Tuple)*num_out_persons);
  i = 0;
  for( ; i < num_persons; ++i ) {
    if(selected_persons_new_index[i] != 0xffffffff) {
      selected_persons[selected_persons_new_index[i]].person_index = i;
      selected_persons[selected_persons_new_index[i]].position = selected_persons_new_index[i];
    }
  }
  qsort(selected_persons, num_out_persons, sizeof(Tuple), tuple_comparator);
  unsigned int* old_to_new_map = (unsigned int*)malloc(sizeof(unsigned int)*num_out_persons);
  i = 0;
  for(; i < num_out_persons;++i) {
    old_to_new_map[selected_persons[i].position] = i;
  }


  /** translating identifier from the one before sorting to the new one **/
  temp_knows_file = fopen(".tmp_knows", "rb");
  if( !temp_knows_file ) {
    fprintf(stderr, "ERROR opening %s\n", ".tmp_knows");
    exit(1);
  }
  unsigned int* knows_buffer_in = (unsigned int*)malloc(sizeof(unsigned int)*knows_buffer_size1);
  unsigned int* knows_buffer_out = (unsigned int*)malloc(sizeof(unsigned int)*knows_buffer_size1);
  fseek(temp_knows_file, 0, SEEK_SET);
  size_t num_elements_read;
  while(num_elements_read = fread(knows_buffer_in, sizeof(unsigned int), knows_buffer_size1, temp_knows_file)) {
    i = 0;
    for(; i < num_elements_read; ++i) {
      knows_buffer_out[i] = old_to_new_map[knows_buffer_in[i]]; 
    }
    fwrite(knows_buffer_out, sizeof(unsigned int), num_elements_read, out_knows_file );
  }

  free(knows_buffer_in);
  free(knows_buffer_out);
  fclose(temp_knows_file);

  i = 0;
  for( ; i < num_out_persons; ++i) {
    fwrite(&persons_in[selected_persons[i].person_index].person_id, sizeof(unsigned int), 1, out_persons_file);
    fwrite(&persons_in[selected_persons[i].person_index].birthday, sizeof(unsigned short), 1, out_birthday_file);
    Interest interest;
    interest.first = persons_in[selected_persons[i].person_index].interest_first;
    interest.n = persons_in[selected_persons[i].person_index].interest_n;
    fwrite(&interest, sizeof(Interest), 1, out_interest_file);
    Knows knows;
    knows.first = selected_persons_knows_start[selected_persons[i].person_index]; 
    knows.n = selected_persons_knows_n[selected_persons[i].person_index]; 
    fwrite(&knows, sizeof(Knows), 1, out_knows_file);
    fwrite(&persons_in[selected_persons[i].person_index].location, sizeof(unsigned short), 1, out_location_file);
  }

  free(old_to_new_map);
  free(selected_persons);
  free(selected_persons_new_index);
  free(selected_persons_knows_start);
  free(selected_persons_knows_n);

  printf("Number of knows: %lu\n", num_knows);
  printf("Number of knows location: %lu\n", num_knows_location);
  printf("Number of filtered knows: %lu\n", num_out_knows);
  printf("Percentage of filtered knows: %f\n", num_out_knows*100.0f/num_knows);

  free(persons_in);
  fclose(in_persons_file);
  fclose(in_knows_file);
  fclose(out_persons_file);
  fclose(out_knows_file);
  fclose(out_interest_file);
  fclose(out_birthday_file);
  fclose(out_location_file);
  return 0;
}



#include "structs.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdio.h>
 
#define EDGE_BUFFER_SIZE 1048576

/** Forward declarations **/

void remove_diff_location( const char* out_person_file_name, const char* out_knows_file_name ); 

void remove_no_reciprocal( const char* out_persons_file_name, const char* out_knows_file_name );

void* mmap_file( const char* file_name, size_t* file_size, FILE** file);

void munmap_file(void* filemap, size_t size, FILE* file );

void partition_persons_file();

typedef struct {
  unsigned int old_index;
  unsigned int new_index;
} Tuple ;



Person* in_persons;
unsigned int* in_knows;
unsigned int num_persons;
unsigned int num_knows;

FILE* in_persons_file; 
FILE* in_knows_file;
FILE* out_persons_file;
FILE* out_knows_file;
FILE* out_knows_index_file;
FILE* out_interest_index_file;
FILE* out_birthday_attr_file;
FILE* out_location_attr_file;
FILE* temp_knows_file;

int tuple_comparator( const void* a, const void* b) {
  Tuple* ra = (Tuple*)a;
  Tuple* rb = (Tuple*)b;
  return ra->new_index - rb->new_index;
}

int person_birthday_comparator( const void* a, const void* b) {
  Person* ra = (Person*)a;
  Person* rb = (Person*)b;
  return ra->birthday - rb->birthday;
}

/** This function creates a reduced knows file where only reciprocal friends living at the same city are kept **/
int main( int argc, char** argv ) {
  if(argc != 2) {
    printf("Invalid usage\n");
  }

  /** Filenames **/
  char in_persons_file_name[256];
  char in_knows_file_name[256];
  sprintf(in_persons_file_name, "%s/%s", argv[1], "person.bin");
  sprintf(in_knows_file_name, "%s/%s", argv[1], "knows.bin");

  size_t size;
  FILE* in_persons_file;
  in_persons = (Person*)mmap_file (in_persons_file_name, &size, &in_persons_file );
  num_persons = size / sizeof(Person);
  FILE* in_knows_file;
  in_knows = (unsigned int*)mmap_file (in_knows_file_name, &size, &in_knows_file );
  num_knows = size / sizeof(unsigned int);

  remove_diff_location(".temp_persons_file", ".temp_knows_file");

  munmap_file(in_persons, num_persons*sizeof(Person), in_persons_file);
  munmap_file(in_knows, num_knows*sizeof(unsigned int), in_knows_file);

  in_persons = (Person*)mmap_file (".temp_persons_file", &size, &in_persons_file );
  num_persons = size / sizeof(Person);
  in_knows = (unsigned int*)mmap_file (".temp_knows_file", &size, &in_knows_file );
  num_knows = size / sizeof(unsigned int);

  remove_no_reciprocal(".temp_persons_file2", "knows.bin");


  munmap_file(in_persons, num_persons*sizeof(Person), in_persons_file);
  munmap_file(in_knows, num_knows*sizeof(unsigned int), in_knows_file);

  in_persons = (Person*)mmap_file (".temp_persons_file2", &size, &in_persons_file );
  num_persons = size / sizeof(Person);
  in_knows = (unsigned int*)mmap_file ("knows.bin", &size, &in_knows_file );
  num_knows = size / sizeof(unsigned int);

  partition_persons_file();

  munmap_file(in_persons, num_persons*sizeof(Person), in_persons_file);
  munmap_file(in_knows, num_knows*sizeof(unsigned int), in_knows_file);
  return 0;
}

void* mmap_file( const char* file_name, size_t* file_size, FILE** file) {

  *file  = fopen(file_name, "rb");
  if( !*file ) {
    fprintf(stderr, "ERROR opening %s\n", file_name);
    exit(1);
  }

  /** Computing number of persons **/
  fseek (*file, 0, SEEK_END);   
  *file_size = ftell (*file);
  fseek (*file, 0, SEEK_SET);   
  return mmap (NULL, *file_size, PROT_READ, MAP_PRIVATE, fileno(*file), 0);

}

void munmap_file(void* filemap, size_t size, FILE* file ) {
  munmap(filemap, size);
  fclose(file);
}

void remove_diff_location( const char* out_persons_file_name, const char* out_knows_file_name ) {

  printf("Removing unnecessary persons from knows file, based on location\n");

  Tuple* persons_new_ids = (Tuple*)malloc(sizeof(Tuple)*num_persons);
  memset(persons_new_ids, 0xff, sizeof(Tuple)*num_persons);
  Knows* knows = (Knows*)malloc(sizeof(Knows)*num_persons);

  FILE* out_knows_file = fopen(out_knows_file_name, "wb");

  int num_output_knows = 0;
  int num_touched_persons = 0;
  int i, j;
  for( i = 0; i < num_persons; ++i ){
    if( i % 10000 == 0 ) 
      printf("Processed %d Persons\n", i);

    Person* person = &in_persons[i];
    unsigned int* knows_person = &in_knows[person->knows_first];
    if( persons_new_ids[i].new_index == 0xffffffff ) {              // if id not already set 
      persons_new_ids[i].old_index = i;
      persons_new_ids[i].new_index = num_touched_persons++;
    }
    knows[i].first = num_output_knows;
    unsigned int num_output_local_knows = 0;
    for( j = 0; j < person->knows_n; ++j ) {
      unsigned int other_index = knows_person[j];
      Person* other = &in_persons[other_index];
      if(other->location == person->location) { 
        if( persons_new_ids[other_index].new_index == 0xffffffff ) { // if id not already set
          persons_new_ids[other_index].old_index = other_index;
          persons_new_ids[other_index].new_index = num_touched_persons++;
        }
        fwrite(&persons_new_ids[other_index].new_index, sizeof(unsigned int), 1, out_knows_file);
        num_output_local_knows++;
        num_output_knows++;
      }
    }
    knows[i].n = num_output_local_knows;
  }
  fclose(out_knows_file);

  qsort(persons_new_ids, num_persons, sizeof(Tuple), tuple_comparator); 

  FILE* out_persons_file = fopen(out_persons_file_name, "wb");
  Person person;
  for(i = 0; i < num_persons;++i){
    if( persons_new_ids[i].new_index != 0xffffffff ) {
      unsigned int index = persons_new_ids[i].old_index;
      person = in_persons[index];
      person.knows_first = knows[index].first;
      person.knows_n = knows[index].n;
      fwrite(&person, sizeof(Person), 1, out_persons_file);
    }
  }
  fclose(out_persons_file);
  free(knows);
  free(persons_new_ids);
}

void remove_no_reciprocal( const char* out_persons_file_name, const char* out_knows_file_name ) {

  printf("Removing unnecessary knows with no reciprocal\n");

  Tuple* persons_new_ids = (Tuple*)malloc(sizeof(Tuple)*num_persons);
  memset(persons_new_ids, 0xff, sizeof(Tuple)*num_persons);
  Knows* knows = (Knows*)malloc(sizeof(Knows)*num_persons);

  FILE* out_knows_file = fopen(out_knows_file_name, "wb");

  int num_output_knows = 0;
  int num_touched_persons = 0;
  int i, j, k;
  for( i = 0; i < num_persons; ++i ){
    if( i % 10000 == 0 ) 
      printf("Processed %d Persons\n", i);

    Person* person = &in_persons[i];
    unsigned int* knows_person = &in_knows[person->knows_first];
    knows[i].first = num_output_knows;
    unsigned int num_output_local_knows = 0;
    for( j = 0; j < person->knows_n; ++j ) {
      unsigned int other_index = knows_person[j];
      Person* other = &in_persons[other_index];
      unsigned int* knows_other = &in_knows[other->knows_first];
      for(k = 0; k < other->knows_n; ++k) {
        if(knows_other[k] == i){
          break;
        }
      }
      if(k < other->knows_n) { 
        if( persons_new_ids[other_index].new_index == 0xffffffff ) { // if id not already set
          persons_new_ids[other_index].old_index = other_index;
          persons_new_ids[other_index].new_index = num_touched_persons++;
        }
        fwrite(&persons_new_ids[other_index].new_index, sizeof(unsigned int), 1, out_knows_file);
        num_output_local_knows++;
        num_output_knows++;
      }
    }
    knows[i].n = num_output_local_knows;

    if( persons_new_ids[i].new_index == 0xffffffff && num_output_local_knows > 0) {              // if id not already set 
      persons_new_ids[i].old_index = i;
      persons_new_ids[i].new_index = num_touched_persons++;
    }
  }
  fclose(out_knows_file);

  qsort(persons_new_ids, num_persons, sizeof(Tuple), tuple_comparator); 

  FILE* out_persons_file = fopen(out_persons_file_name, "wb");
  Person person;
  for(i = 0; i < num_persons;++i){
    if( persons_new_ids[i].new_index != 0xffffffff ) {
      unsigned int index = persons_new_ids[i].old_index;
      person = in_persons[index];
      person.knows_first = knows[index].first;
      person.knows_n = knows[index].n;
      fwrite(&person, sizeof(Person), 1, out_persons_file);
    }
  }
  fclose(out_persons_file);
  free(knows);
  free(persons_new_ids);

}

void partition_persons_file() {
  FILE* person_id_file = fopen("person_id.bin","wb");
  FILE* knows_index_file = fopen("knows_index.bin","wb");
  FILE* interest_index_file = fopen("interest_index.bin","wb");
  FILE* birthday_file = fopen("birthday.bin","wb");
  int i;
  for(i = 0; i < num_persons;++i) {
    Knows k;
    k.first = in_persons[i].knows_first;
    k.n = in_persons[i].knows_n;
    fwrite(&k, sizeof(Knows), 1, knows_index_file);
    fwrite(&in_persons[i].birthday, sizeof(unsigned short), 1, birthday_file);
    Interest interest;
    interest.first = in_persons[i].interest_first;
    interest.n = in_persons[i].interest_n;
    fwrite(&interest, sizeof(Interest), 1, interest_index_file);
    fwrite(&in_persons[i].person_id, sizeof(unsigned long), 1, person_id_file);
  }
  fclose(person_id_file);
  fclose(knows_index_file);
  fclose(interest_index_file);
  fclose(birthday_file);
}

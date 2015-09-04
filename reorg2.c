

#include "structs.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdio.h>
 
#define EDGE_BUFFER_SIZE 1048576

typedef struct {
  unsigned int person_index;
  unsigned int position;
} Tuple ;


typedef struct {
  unsigned int tail;
  unsigned int head;
} Edge;

PersonIn* in_persons;
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
  int res = in_persons[((Tuple*)a)->person_index].birthday - in_persons[((Tuple*)b)->person_index].birthday;
  if( res ) return res;
  return ((Tuple*)a)->person_index - ((Tuple*)b)->person_index;
}

int person_birthday_comparator( const void* a, const void* b) {
  PersonIn* ra = (PersonIn*)a;
  PersonIn* rb = (PersonIn*)b;
  return ra->birthday - rb->birthday;
}

int unsigned_int_comparator( const void* a, const void* b) {
  return ((int)(*(unsigned int*)a)) - ((int)(*(unsigned int*)b));
}

int edge_tail_comparator( const void* a, const void* b) {
  int res = ((unsigned long)((Edge*)a)->tail) - ((unsigned long)((Edge*)b)->tail);
  if(res)
    res = ((unsigned long)((Edge*)a)->head) - ((unsigned long)((Edge*)b)->head);
  return res;
}

int edge_head_comparator( const void* a, const void* b) {
  int res = ((unsigned long)((Edge*)a)->head) - ((unsigned long)((Edge*)b)->head);
  if(res)
    res = ((unsigned long)((Edge*)a)->tail) - ((unsigned long)((Edge*)b)->tail);
  return res;
}

/** This function creates a reduced knows file where only reciprocal friends living at the same city are kept **/
void create_reduced_knows_file(const char* out_knows_file_name, Knows** new_knows_array ) {

  *new_knows_array = (Knows*)malloc(sizeof(Knows)*num_persons);
  memset(*new_knows_array, 0xff, sizeof(Knows)*num_persons); 
  FILE* tmp_edge_file = fopen(".tmp_edges", "wb");
  FILE* tmp_edge_file2 = fopen(".tmp_edges2", "wb");
  int total_written_edges = 0;
  int num_edges = 0;
  Edge* edge_buffer = (Edge*)malloc(sizeof(Edge)*EDGE_BUFFER_SIZE);

  printf("Filtering edge by location");
  int i = 0;
  for( ; i < num_persons; ++i) {
    PersonIn* person = &in_persons[i];
    unsigned int* knows_person = &in_knows[person->knows_first];
//    unsigned int num_neighbors_found = 0;
    int j = 0;
    for( ;j < person->knows_n; ++j) {
      unsigned int other_index = knows_person[j];
      PersonIn* other = &in_persons[other_index];
      if( other->location == person->location ) {
        edge_buffer[num_edges].tail = i; 
        edge_buffer[num_edges].head = other_index; 
        total_written_edges++;
        num_edges++;
        if( num_edges >= EDGE_BUFFER_SIZE ) {
          fwrite(edge_buffer, sizeof(Edge), num_edges, tmp_edge_file);
          num_edges = 0;
        }
      }
    }
  }

  if(num_edges) {
    fwrite(edge_buffer, sizeof(Edge), num_edges, tmp_edge_file);
  }
  free(edge_buffer);
  fclose(tmp_edge_file);

  printf("Reading filtered edges\n");
  edge_buffer = (Edge*)malloc(sizeof(Edge)*total_written_edges);
  tmp_edge_file = fopen(".tmp_edges", "rb");
  fread(edge_buffer, sizeof(Edge), total_written_edges, tmp_edge_file);
  fclose(tmp_edge_file);
  printf("Sorting by head\n");
  qsort(edge_buffer, total_written_edges, sizeof(Edge), edge_head_comparator);
  printf("Filtering edges by reciprocity\n");
  int total_written_edges2 = 0;
  i = 0;
  for (; i < total_written_edges; ++i){
    Edge* edge = &edge_buffer[i];
    unsigned int n = in_persons[edge->head].knows_n;
    unsigned int first = in_persons[edge->head].knows_first;
    unsigned int* knows = &in_knows[first];
    int j =0;
    for( ; j < n; ++j) {
      if( knows[j] == edge->tail ) {
        fwrite(edge,sizeof(Edge),1,tmp_edge_file2);
        total_written_edges2++;
        break;
      }
    }
  }
  fclose(tmp_edge_file2);
  tmp_edge_file2 = fopen(".tmp_edges2", "rb");
  fread(edge_buffer, sizeof(Edge), total_written_edges2, tmp_edge_file2);
  fclose(tmp_edge_file2);

  printf("Sorting by tail\n");
  qsort(edge_buffer, total_written_edges2, sizeof(Edge), edge_tail_comparator);

  FILE* out_knows_file = fopen(out_knows_file_name, "wb");
  i =0;
  unsigned int current_edge = 0xffffffff;
  for(;i < total_written_edges2; ++i) {
    int num_written = 0;
    current_edge = edge_buffer[i].tail;
    while(current_edge == edge_buffer[i].tail && i < total_written_edges2){
      fwrite(&(edge_buffer[i].head), sizeof(unsigned int), 1, out_knows_file);
      ++i;
      ++num_written;
    }
    (*new_knows_array)[current_edge].n = num_written;
    (*new_knows_array)[current_edge].first = i-num_written;
  }
  fclose(out_knows_file);
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
  char out_knows_index_file_name[256];
  char out_interest_index_file_name[256];
  char out_birthday_attr_file_name[256];
  char out_location_attr_file_name[256];
  sprintf(in_persons_file_name, "%s/%s", argv[1], "person.bin");
  sprintf(in_knows_file_name, "%s/%s", argv[1], "knows.bin");
  sprintf(out_persons_file_name, "%s/%s", argv[1], "refined_person.bin");
  sprintf(out_knows_file_name, "%s/%s", argv[1], "refined_knows.bin");
  sprintf(out_knows_index_file_name, "%s/%s", argv[1], "knows_index.bin");
  sprintf(out_interest_index_file_name, "%s/%s", argv[1], "interest_index.bin");
  sprintf(out_birthday_attr_file_name, "%s/%s", argv[1], "birthday_attr.bin");
  sprintf(out_location_attr_file_name, "%s/%s", argv[1], "location_attr.bin");

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

  out_knows_index_file = fopen(out_knows_index_file_name, "wb");
  if( !out_knows_index_file ) {
    fprintf(stderr, "ERROR opening %s\n", out_knows_index_file_name);
    exit(1);
  }

  out_interest_index_file = fopen(out_interest_index_file_name, "wb");
  if( !out_interest_index_file ) {
    fprintf(stderr, "ERROR opening %s\n", out_interest_index_file_name);
    exit(1);
  }

  out_birthday_attr_file = fopen(out_birthday_attr_file_name, "wb");
  if( !out_birthday_attr_file ) {
    fprintf(stderr, "ERROR opening %s\n", out_birthday_attr_file_name);
    exit(1);
  }

  out_location_attr_file = fopen(out_location_attr_file_name, "wb");
  if( !out_location_attr_file ) {
    fprintf(stderr, "ERROR opening %s\n", out_location_attr_file_name);
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
  num_persons = in_persons_file_size / sizeof(PersonIn);
  fseek (in_persons_file, 0, SEEK_SET);   

  /** Computing number of knows **/
  fseek (in_knows_file, 0, SEEK_END);   
  size_t in_knows_file_size = ftell (in_knows_file);
  num_knows = in_knows_file_size / sizeof(unsigned int);
  fseek (in_knows_file, 0, SEEK_SET);   

  in_persons = (PersonIn*)mmap (NULL, num_persons*sizeof(PersonIn), PROT_READ, MAP_PRIVATE, fileno(in_persons_file), 0);
  in_knows = (unsigned int*)mmap (NULL, num_knows*sizeof(unsigned int), PROT_READ, MAP_PRIVATE, fileno(in_knows_file), 0);

  PersonIn* person_buffer = (PersonIn*)malloc(sizeof(PersonIn)*num_persons);
  memcpy(person_buffer, in_persons, sizeof(PersonIn)*num_persons);
//  qsort(person_buffer, num_persons, sizeof(PersonIn), person_birthday_comparator);

  Knows* knows = (Knows*)malloc(sizeof(Knows)*num_persons);

  int num_exported_edges = 0;
  int i =0;
  for(;i < num_persons; ++i) {
    PersonIn* person = &person_buffer[i];
    unsigned int* knows_person = &in_knows[person_buffer[i].knows_first];
    knows[i].first = num_exported_edges;
    int num_with_same_location = 0;
    int j = 0;
    for(; j < person_buffer[i].knows_n; ++j) {
      unsigned int other_index = knows_person[j];
      PersonIn* other = &person_buffer[other_index];
      if(person->location == other->location){
        fwrite(&knows_person[j], sizeof(unsigned int), 1, out_knows_file);
        num_exported_edges++;
        num_with_same_location++;
      }
    }
    knows[i].n = num_with_same_location;
  }

  i = 0;
  for( ; i < num_persons; ++i) {
    fwrite(&person_buffer[i].person_id, sizeof(unsigned long), 1, out_persons_file);
    fwrite(&person_buffer[i].birthday, sizeof(unsigned short), 1, out_birthday_attr_file);
    Interest interest;
    interest.first = person_buffer[i].interest_first;
    interest.n = person_buffer[i].interest_n;
    fwrite(&interest, sizeof(Interest), 1, out_interest_index_file);
    fwrite(&knows[i], sizeof(Knows), 1, out_knows_index_file);
    fwrite(&person_buffer[i].location, sizeof(unsigned short), 1, out_location_attr_file);
  }
  free(person_buffer);

  munmap(in_persons, num_persons*sizeof(PersonIn));
  munmap(in_knows, num_knows*sizeof(unsigned int));
  fclose(in_persons_file);
  fclose(in_knows_file);
  fclose(out_persons_file);
  fclose(out_knows_file);
  fclose(out_knows_index_file);
  fclose(out_interest_index_file);
  fclose(out_birthday_attr_file);
  fclose(out_location_attr_file);
  return 0;
}

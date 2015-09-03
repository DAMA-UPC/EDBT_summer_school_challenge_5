

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

  Knows* new_knows;
  create_reduced_knows_file(".filtered_knows", &new_knows);

  munmap(in_knows, num_knows*sizeof(unsigned int));
  in_knows = (unsigned int*)mmap (NULL, num_knows*sizeof(unsigned int), PROT_READ, MAP_PRIVATE, fileno(in_knows_file), 0);
  fclose(in_knows_file);

  in_knows_file = fopen(".filtered_knows", "rb");
  fseek (in_knows_file, 0, SEEK_END);   
  in_knows_file_size = ftell (in_knows_file);
  num_knows = in_knows_file_size / sizeof(unsigned int);
  fseek (in_knows_file, 0, SEEK_SET);   
  in_knows = (unsigned int*)mmap (NULL, num_knows*sizeof(unsigned int), PROT_READ, MAP_PRIVATE, fileno(in_knows_file), 0);

  unsigned int* selected_persons_new_index = (unsigned int*) malloc(sizeof(unsigned int)*num_persons);
  memset(selected_persons_new_index,0xff, sizeof(unsigned int)*num_persons); 
  unsigned int* selected_persons_knows_start = (unsigned int*) malloc(sizeof(unsigned int)*num_persons);
  unsigned int* selected_persons_knows_n = (unsigned int*) malloc(sizeof(unsigned int)*num_persons);

  unsigned int num_out_persons = 0;
  int i = 0;
  for( ; i < num_persons; ++i) {
    PersonIn* person = &in_persons[i];
    if( i % 10000 == 0 ) {
      printf("Read %d persons. This person id %lu and city %hu\n", i, person->person_id,
          person->location);
    }
    if( new_knows[i].first != 0xffffffff ) {
      unsigned int* knows_person = &in_knows[new_knows[i].first];

      if( selected_persons_new_index[i] == 0xffffffff ) {
        selected_persons_new_index[i] = num_out_persons++; // we increment first because 0 is a reserved number
      }

      int j = 0;
      for( ;j < new_knows[i].n; ++j) {
        unsigned int other_index = knows_person[j];

        if( selected_persons_new_index[other_index] == 0xffffffff ) {
          selected_persons_new_index[other_index] = num_out_persons++; // we increment first because 0 is a reserved number
        }

        unsigned int new_index = selected_persons_new_index[other_index]; // assigning new index to person
        fwrite(&new_index, sizeof(unsigned int), 1, temp_knows_file);
      }
      selected_persons_knows_start[i] = new_knows[i].first; // storing the new knows offset 
      selected_persons_knows_n[i] = new_knows[i].n; 
    }
  }
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


  // translating identifier from the one before sorting to the new one 
  temp_knows_file = fopen(".tmp_knows", "rb");
  if( !temp_knows_file ) {
    fprintf(stderr, "ERROR opening %s\n", ".tmp_knows");
    exit(1);
  }
  unsigned int knows_buffer_size = 1024;
  unsigned int* knows_buffer_in = (unsigned int*)malloc(sizeof(unsigned int)*knows_buffer_size);
  unsigned int* knows_buffer_out = (unsigned int*)malloc(sizeof(unsigned int)*knows_buffer_size);
  fseek(temp_knows_file, 0, SEEK_SET);
  size_t num_elements_read;
  while((num_elements_read = fread(knows_buffer_in, sizeof(unsigned int), knows_buffer_size, temp_knows_file))) {
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
    fwrite(&in_persons[selected_persons[i].person_index].person_id, sizeof(unsigned long), 1, out_persons_file);
    fwrite(&in_persons[selected_persons[i].person_index].birthday, sizeof(unsigned short), 1, out_birthday_attr_file);
    Interest interest;
    interest.first = in_persons[selected_persons[i].person_index].interest_first;
    interest.n = in_persons[selected_persons[i].person_index].interest_n;
    fwrite(&interest, sizeof(Interest), 1, out_interest_index_file);
    Knows knows;
    knows.first = selected_persons_knows_start[selected_persons[i].person_index]; 
    knows.n = selected_persons_knows_n[selected_persons[i].person_index]; 
    fwrite(&knows, sizeof(Knows), 1, out_knows_index_file);
    fwrite(&in_persons[selected_persons[i].person_index].location, sizeof(unsigned short), 1, out_location_attr_file);
  }

  free(old_to_new_map);
  free(selected_persons);
  free(selected_persons_new_index);
  free(selected_persons_knows_start);
  free(selected_persons_knows_n);
  free(new_knows);

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

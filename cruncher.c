#include "structs.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
int main( int argc, char** argv ) 
{
	if(argc< 2) {
		printf("Invalid usage\n");
		return -1;
	}  

  char persons_file_name[256];
  char knows_file_name[256];
  char interest_file_name[256];

  sprintf(persons_file_name, "%s/%s", argv[1], "refined_person.bin");
  sprintf(knows_file_name, "%s/%s", argv[1], "refined_knows.bin");
  sprintf(interest_file_name,"%s%s",argv[1],"interest.bin");

  FILE* persons_file = fopen(persons_file_name, "rb");
  if( !persons_file ) {
	  fprintf(stderr, "ERROR opening %s\n", persons_file_name);
	  exit(1);
  }
  else printf("%s opened correctly \n", persons_file_name);

  FILE* knows_file = fopen(knows_file_name, "rb");
  if( !knows_file ) {
	  fprintf(stderr, "ERROR opening %s\n", knows_file_name);
	  exit(1);
  }
  else printf("%s opened correctly \n", knows_file_name);

  FILE* interest_file = fopen(interest_file_name, "rb");
  if( !knows_file ) {
	  fprintf(stderr, "ERROR opening %s\n", interest_file_name);
	  exit(1);
  }
  else printf("%s opened correctly \n", interest_file_name);

  char* queries_file_name = argv[2];
  
  FILE* queries_file = fopen(queries_file_name, "rb");
  if( !queries_file ) {
	  fprintf(stderr, "ERROR opening %s\n", queries_file_name);
	  exit(1);
  }
  else printf("%s opened correctly \n", queries_file_name);
                                                
  fseek(persons_file,0,SEEK_END);
  size_t persons_file_size = ftell(persons_file);
  long num_persons = persons_file_size / sizeof(Person);
  Person* persons  = (Person*)malloc(persons_file_size);
  if( !persons ) {
	fprintf(stderr, "ERROR while creating persons array");
	exit(1);
  }
  fread(persons, sizeof(Person), num_persons, persons_file); 
  
  char query[256];
  char *token;
  unsigned long queryParams[7];

  while( fgets (query, 256, queries_file)!=NULL ) 
  {

	  token = strtok(query, "|");
	  int i=0;
	  while( token != NULL ) 
	  {
          queryParams[i++]=atol(token);
	      token = strtok(NULL, "|");
	  }
	  int j = 0 ; 
	  for(;j<7;j++)
		  printf("%lu, ",queryParams[j]);
	  printf("\n");
  }




}

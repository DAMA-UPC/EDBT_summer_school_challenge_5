#include <stdlib.h>
#include "structs.h"
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <stdbool.h>
unsigned short getDate(char* stringDate); 
int compare(const void* a,const void* b)
{
	Result* r1 = (Result*) a;
	Result* r2 = (Result*) b;

	int dif_query = r1->query_id - r2->query_id;
	if(dif_query!=0)
		return dif_query;

	int dif_score = r1->score - r2->score;
	if(dif_score!=0)
		return -dif_score;
    int dif_p = r1->p-r2->p;
	if(dif_p!=0)
		return dif_p;
	int dif_f = r1->f-r2->f;
	return dif_f;
}

int main( int argc, char** argv ) 
{
	if(argc< 2) {
		printf("Invalid usage\n");
		return -1;
	}  

	char persons_file_name[256];
	char refined_knows_file_name[256];
	char knows_file_name[256];
	char birthday_file_name[256];
	char refined_interest_file_name[256];
	char interest_file_name[256];


	sprintf(persons_file_name, "%s/%s", argv[1], "refined_person.bin");
	sprintf(refined_knows_file_name, "%s/%s", argv[1], "knows_index.bin");
	sprintf(knows_file_name, "%s/%s", argv[1], "refined_knows.bin");
	sprintf(interest_file_name,"%s%s",argv[1],"interest.bin");
	sprintf(birthday_file_name,"%s%s",argv[1],"birthday_attr.bin");
	sprintf(refined_interest_file_name,"%s%s", argv[1],"interest_index.bin");

	long num_persons;

	FILE* persons_file = fopen(persons_file_name, "rb");
	if( !persons_file ) {
		fprintf(stderr, "ERROR opening %s\n", persons_file_name);
		exit(1);
	}
	/* else printf("%s opened correctly \n", persons_file_name); */

	FILE* refined_knows_file = fopen(refined_knows_file_name, "rb");
	if( !refined_knows_file ) {
		fprintf(stderr, "ERROR opening %s\n", refined_knows_file_name);
		exit(1);
	}
	/* else printf("%s opened correctly \n", refined_knows_file_name); */

	FILE* knows_file = fopen(knows_file_name, "rb");
	if( !knows_file ) {
		fprintf(stderr, "ERROR opening %s\n", knows_file_name);
		exit(1);
	}
	/* else printf("%s opened correctly \n", knows_file_name); */

	FILE* interest_file = fopen(interest_file_name, "rb");
	if( !refined_knows_file ) {
		fprintf(stderr, "ERROR opening %s\n", interest_file_name);
		exit(1);
	}
	/* else printf("%s opened correctly \n", interest_file_name); */

	fseek(persons_file,0,SEEK_END);
	size_t persons_file_size = ftell(persons_file);
	fseek(persons_file,0,SEEK_SET);
	num_persons= persons_file_size / sizeof(unsigned long);
	/* printf("Number Persons: %ld\n", num_persons); */
	unsigned long* persons  = (unsigned long*)malloc(persons_file_size);
	if( !persons ) {
		fprintf(stderr, "ERROR while creating persons array");
		exit(1);
	}
	fread(persons, sizeof(unsigned long), num_persons, persons_file); 




	char* queries_file_name = argv[2];

	FILE* queries_file = fopen(queries_file_name, "rb");
	if( !queries_file ) {
		fprintf(stderr, "ERROR opening %s\n", queries_file_name);
		exit(1);
	}
	/* else printf("%s opened correctly \n", queries_file_name); */


	char query[256];
	char *token;
	char* queryParams[7];
	int theoretical_num_queries = 100;
	int numQueries = 0 ;
	Query* queries = (Query*) malloc(sizeof(Query)*theoretical_num_queries);

	//Carrega dels parametres de les consultes
	while( fgets (query, 256, queries_file)!=NULL ) 
	{
		token = strtok(query, "|");
		int i=0;
		while( token != NULL ) 
		{
			queryParams[i++]=token;
			token = strtok(NULL, "|");
		}

		Query q;
		q.query_id =(unsigned short)atoi( queryParams[0]);
		q.a1 = (unsigned short)atoi(queryParams[1]);
		q.a2 = (unsigned short)atoi(queryParams[2]);
		q.a3 = (unsigned short)atoi(queryParams[3]);
		q.a4 = (unsigned short)atoi(queryParams[4]);
		q.d1 = getDate(queryParams[5]);
		q.d2 = getDate(queryParams[6]);
		if (numQueries==theoretical_num_queries)
		{
			theoretical_num_queries=numQueries;
			queries = (Query*)realloc(queries,theoretical_num_queries*sizeof(Query));
		}
		queries[numQueries++]=q;
	}


	int i = 0;


	FILE* birthday_file = fopen(birthday_file_name, "rb");
	if( !birthday_file ) {
		fprintf(stderr, "ERROR opening %s\n", birthday_file_name);
		exit(1);
	} 
	fseek (birthday_file, 0, SEEK_END);   // non-portable
	size_t birthday_file_size = ftell (birthday_file);
	fseek (birthday_file, 0, SEEK_SET);   // non-portable

	unsigned short* birthdays = (unsigned short*) malloc (birthday_file_size);
	if(!birthdays)
	{
		fprintf(stderr,"ERROR while creating birthdays array");
		exit(1);
	}
	fread(birthdays,sizeof(unsigned short),num_persons,birthday_file);

	FILE* refined_interest_file = fopen(refined_interest_file_name, "rb");
	if( !refined_interest_file ) {
		fprintf(stderr, "ERROR opening %s\n", refined_interest_file_name);
		exit(1);
	} 
	fseek (refined_interest_file, 0, SEEK_END);   // non-portable
	size_t refined_interest_file_size = ftell (refined_interest_file);
	fseek (refined_interest_file, 0, SEEK_SET);   // non-portable

	Interest* refined_interests = (Interest*) malloc (refined_interest_file_size);
	if(!refined_interests)
	{
		fprintf(stderr,"ERROR while creating refined_interests array");
		exit(1);
	}
	fread(refined_interests,sizeof(Interest),num_persons,refined_interest_file);


	fseek (refined_knows_file, 0, SEEK_END);   // non-portable
	size_t refined_knows_file_size = ftell (refined_interest_file);
	fseek (refined_knows_file, 0, SEEK_SET);   // non-portable

	Knows* refined_knows = (Knows*) malloc (refined_knows_file_size);
	if(!refined_interests)
	{
		fprintf(stderr,"ERROR while creating refined_knows array");
		exit(1);
	}
	fread(refined_knows,sizeof(Knows),num_persons,refined_knows_file);



	int birthday_offset;
	int first_found =-1;
	int last_found = -1;
	int interest_buffer_size=20;
	unsigned short* interest_buffer = (unsigned short*)malloc(sizeof(unsigned short)*interest_buffer_size);
	int interest_buffer_size2=20;
	unsigned short* interest_buffer2 = (unsigned short*)malloc(sizeof(unsigned short)*interest_buffer_size2);
	int knows_buffer_size=20;
	unsigned short* knows_buffer = (unsigned short*)malloc(sizeof(unsigned short)*knows_buffer_size);
                                                                      
	int num_results=3000;
	int current_num_results=0;
	Result* results = (Result*) malloc(sizeof(Result)*num_results);
	for(      ;i<numQueries;i++)
	{
		Query q = queries[i];
		/* printf("Resolving query id: %u; date_1= %hu, date_2 = %hu\n", q.query_id,q.d1,q.d2); */


		first_found=-1;
		last_found = -1 ;
		//Identificar el conjunt de persones dins el rang  
		for(birthday_offset = 0 ; birthday_offset<num_persons; birthday_offset++)
		{
			if(first_found==-1 && birthdays[birthday_offset]>=q.d1)
				first_found = birthday_offset;
			if(last_found==-1 && birthdays[birthday_offset]>q.d2)  
			{
				last_found=birthday_offset-1;
				break;
			}
		}
		if(last_found==-1 && birthday_offset == num_persons)
			last_found=birthday_offset-1;
		/* printf ("First Person : %d\nSecond Person: %d\n\n",first_found, last_found); */
		int person_offset;
		//Per cada persona, calcular l'score
		for (person_offset=first_found;person_offset<last_found;person_offset++)
		{            
			bool valid_person=true;
			int score=0;
			Interest* interest = &refined_interests[person_offset];                        
			if(interest->n > interest_buffer_size)                                                  
			{
				interest_buffer_size=interest->n;
				interest_buffer = (unsigned short* ) realloc ( interest_buffer, interest_buffer_size*sizeof(unsigned short));
			}
			fseek(interest_file,interest->first*sizeof(unsigned short),SEEK_SET);
			fread(interest_buffer, sizeof(unsigned short), interest->n, interest_file);
			int interest_offset = 0 ; 

			//   printf("Interests of person: %d\n",person_offset);
			for(; interest_offset < interest->n ;interest_offset++)
			{   
				if(interest_buffer[interest_offset]==q.a1)
				{
					valid_person=false;
					break;
				}      
				else
					if(interest_buffer[interest_offset]==q.a2||interest_buffer[interest_offset]==q.a3||interest_buffer[interest_offset]==q.a4)
						score++;
			}
			if(score==0) valid_person = false;
			if(valid_person)
			{
				Knows* knows = &refined_knows[person_offset];
				if(knows->n>knows_buffer_size)
				{
					knows_buffer_size=knows->n;
					knows_buffer=(unsigned short*) realloc(knows_buffer, knows_buffer_size*sizeof(unsigned short));
				}
				int knows_offset = 0 ;
				for(;knows_offset<knows->n;knows_offset++)
				{

					interest = &refined_interests[knows_offset];                        
					if(interest->n > interest_buffer_size2)                                                  
					{
						interest_buffer_size2=interest->n;                            
						/* printf("Interest n = %hu; interest_buffer_size2 = %d \n",interest->n,interest_buffer_size2); */
						interest_buffer2 = (unsigned short* )realloc(interest_buffer2, interest_buffer_size2*sizeof(unsigned short));
						if (interest_buffer2 ==NULL) 
							printf("ERROR - couldn't roallocate interest_buffer2");
					}
					fseek(interest_file,interest->first*sizeof(unsigned short),SEEK_SET);
					fread(interest_buffer2, sizeof(unsigned short), interest->n, interest_file);

					//   printf("Interests of person: %d\n",person_offset);
					valid_person=false;
					int interest_offset = 0 ; 
					for(; interest_offset < interest->n ;interest_offset++)
					{   
						if(interest_buffer2[interest_offset]==q.a1)
						{
							valid_person=true;
							break;
						}      
					} 
					if(valid_person)
					{

						Result r;
						r.query_id = i+1;
						r.score=score;
						r.p=persons[person_offset];
						r.f=persons[knows_offset];
						results[current_num_results++]=r;
						if(current_num_results>num_results)
						{
							num_results*=2;
							results=(Result*)realloc(results,num_results*sizeof(Result));
						}                  
						/* printf("%d|%d|%lu|%lu\n",(i+1),score,persons[person_offset],persons[knows_offset]); */
					}
				}
			}
		}  
	}
	printf("Number of results: %d\n",current_num_results);
	qsort(results,current_num_results,sizeof(Result),compare);
	int results_offset;
	for(results_offset = 0; results_offset< current_num_results; results_offset++)
	{
		printf("%d|%d|%lu|%lu\n",results[results_offset].query_id,results[results_offset].score,results[results_offset].p,results[results_offset].f);
	}
	free(interest_buffer);
	free(interest_buffer2);
	free(knows_buffer);
	free(queries);
	free(persons);



	fclose(persons_file);
	fclose(refined_knows_file);
	fclose(knows_file);
	fclose(queries_file);
	fclose(interest_file);
	fclose(refined_interest_file);
}


unsigned short getDate(char* stringDate) 
{
	char* s1;
	char* s2;
	char* token = strtok(stringDate, "-");
	int i=0;
	while( token != NULL ) 
	{

		if(i==1) s1=token;
		if(i==2) s2=token;
		i++;
		token = strtok(NULL, "-");
	}                            
	char resultString[8];
	sprintf(resultString,"%s%s",s1,s2); 

	return (unsigned short) atoi(resultString);
}

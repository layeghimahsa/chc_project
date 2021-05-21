#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <string.h>

#include "2by2sim.h"
#include "cpu.h"

//array of 1024 lines with 64 bit words
char main_mem[MAIN_MEM_SIZE][INSTRUCTION_SIZE];

pthread_mutex_t mem_lock;  //main mem mutex

/* MUTEX commands

int pthread_mutex_init(pthread_mutex_t *restrict mutex, const pthread_mutexattr_t *restrict attr);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
int pthread_mutex_destroy(pthread_mutex_t *mutex);

*/



int populate(){


    
    int index = 0;

    char buffer[INSTRUCTION_SIZE];
    FILE *file;
  
    file = fopen("../code.txt", "r");

    if ( file == NULL )
    {
        printf( "the file failed to open \n" ) ;
	return 0;
    }


    while( fgets(buffer, INSTRUCTION_SIZE, file) != NULL ) {

	//printf("%s\n", buffer);
    	strcpy(main_mem[index],buffer);
    	index++;
    }
       
    fclose(file) ;
    return 1;
	
}


int main()
{
    printf("***SIMULATION START***\n\n");

    if(populate() == 0){
	printf("Failed to populate memory\n");
	printf("\n\n***SIMULATION EXIT WITH ERROR***\n\n");
	return 0;
    }

    //create mutex
    if (pthread_mutex_init(&mem_lock, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }
  
    
    pthread_t thread_id[4];
    pthread_create(&(thread_id[0]), NULL, &CPU_start, NULL);
    pthread_create(&(thread_id[1]), NULL, &CPU_start, NULL);
    pthread_create(&(thread_id[2]), NULL, &CPU_start, NULL);
    pthread_create(&(thread_id[3]), NULL, &CPU_start, NULL);

    /***********************/
    /*** task scheduling ***/
    /***********************/

	
    /***********************/
    /**** Simulation end ***/
    /***********************/
	
    pthread_join(thread_id[0], NULL);
    pthread_join(thread_id[1], NULL);
    pthread_join(thread_id[2], NULL);
    pthread_join(thread_id[3], NULL);

    pthread_mutex_destroy(&mem_lock);

    printf("***SIMULATION COMPLETE***\n\n");
    
    return 0;
}




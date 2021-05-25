#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <pthread.h>
#include <assert.h>
#include <string.h>

#include "2by2sim.h"
#include "cpu.h"

//array of 1024 lines with 64 bit words
int main_mem[MAIN_MEM_SIZE];
int cpu_generated;
int cpu_available[NUM_CPU] = {0,0,0,0};
pthread_mutex_t mem_lock;  //main mem mutex


/* MUTEX commands

int pthread_mutex_init(pthread_mutex_t *restrict mutex, const pthread_mutexattr_t *restrict attr);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
int pthread_mutex_destroy(pthread_mutex_t *mutex);

*/

int code[] = {//End main:
0x7fffffff,
0x0,
0x7,
0x20,
0xc,
0x0,
0x1,
0x8,
0x7fffffff,
0x0,
0x7,
0x20,
0xc,
0x0,
0x1,
0x4,
0x7fffffff,
0x2,
0xfffffffc,
0x24,
0x3,
0x2,
0x0,
0x0,
0x0
//Start main @(0):
};

int populate(){


   /* 
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
    	//strcpy(main_mem[index],buffer);
	main_mem[index] = buffer;
    	index++;
    }
       
    fclose(file) ; 
*/

    for(int i = 0; i<25; i++){
	main_mem[i] = code[i];
    } 
    
    return 1;
	
}




struct cpu *select_task(){
	
	struct cpu *CPU = (struct cpu *)malloc(sizeof(struct cpu));
	

	for(int i = 0; i < MAIN_MEM_SIZE; i++){
		if(main_mem[i] == 2147483647){  //0x7ffffffff
			
		}
	}

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
   

    
    cpu_generated = 0;
    pthread_t thread_id[NUM_CPU];
    int not_last_node = 1;
    while(not_last_node){
	
	if(cpu_generated == NUM_CPU){
		//sleep(0.01);
	}else{
		struct cpu *task = select_task();
		if(task == NULL){

		}else{
			int ready;
			for(ready = 0; ready<NUM_CPU; ready++){
				if(cpu_available[ready] == 0)
					break;
			} 
			task->cpu_num = ready;
			cpu_available[ready] = 1;
			pthread_create(&(thread_id[ready]), NULL, &CPU_start, &task);
			cpu_generated++;
		}
	}
	sleep(0.01);


    }

    /*
    
    pthread_t thread_id[4];
    pthread_create(&(thread_id[0]), NULL, &CPU_start, &cpu_generated);
    sleep(0.1);
    cpu_generated++;
    pthread_create(&(thread_id[1]), NULL, &CPU_start, &cpu_generated);
    cpu_generated++;
    sleep(0.1);
    pthread_create(&(thread_id[2]), NULL, &CPU_start, &cpu_generated);
    cpu_generated++;
    sleep(0.1);
    pthread_create(&(thread_id[3]), NULL, &CPU_start, &cpu_generated);
    sleep(0.1);
    */

    /***********************/
    /*** task scheduling ***/
    /***********************/

    /*
	-64:	0x7fffffff      //new node label
	-60:	0x0		//number of dependencies 
	-5c:	0x7		//value (const 7)
	-58:	0x20		//end address + 1 (next node in graph)
	-54:	0xc		//operation
	-50:	0x0             //number of arguments 
	-4c:	0x1             //expansion or not flag 
	-48:	0x8             //result destination
	-44:	0x7fffffff
	-40:	0x0
	-3c:	0x7
	-38:	0x20
	-34:	0xc
	-30:	0x0
	-2c:	0x1
	-28:	0x4
	-24:	0x7fffffff
	-20:	0x2
	-1c:	0xfffffffc
	-18:	0x24
	-14:	0x3
	-10:	0x2
	-c:	0x0
	-8:	0x0
	-4:	0x0
    */
    //printf("CPU num: %d\n" , thread_id[0].cpu_num);
    //char* selected_task = select_task();
    /***********************/
    /**** Simulation end ***/
    /***********************/
	
    pthread_join(thread_id[0], NULL);
    pthread_join(thread_id[1], NULL);
    pthread_join(thread_id[2], NULL);
    pthread_join(thread_id[3], NULL);

    pthread_mutex_destroy(&mem_lock);

    printf("\n\n***SIMULATION COMPLETE***\n\n");

    //for(int i = 0; i<25; i++){
//	printf("code[%d]: %x\n", i ,code[i]); 
    //}
    return 0;
}


































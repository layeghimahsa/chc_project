#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <pthread.h>
#include <string.h>
#include "cpu.h"
#include "2by2sim.h"

int cpu_num;


void *CPU_start(struct cpu CPU){
	
	struct memory *mem;
	//execute(mem, CPU);


}

void execute(struct memory *mem, struct cpu *CPU)
{
    //sleep(1);
    /** to be implemented **/
    printf("CPU Start!  \n");
    
    //printf("TEST MAIN MEM ACCES: main_mem[0] -> %s\n",main_mem[0]);
    printf("CPU %d READY\n",cpu_num);
    //fetch_task();

    return;
}
   
void fetch_task(){
	printf("CPU %d fetching task\n",cpu_num);
	pthread_mutex_lock(&mem_lock);
	
	for(int i = 0; i<MAIN_MEM_SIZE; i++){
		//if(strcmp(main_mem[0], "0x7fffffff")==0){}		

	}

	pthread_mutex_unlock(&mem_lock);
}




















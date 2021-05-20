#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <pthread.h>

#include "cpu.h"
#include "2by2sim.h"



void *CPU_start(){
	
	struct memory *mem;
        struct cpu CPU;

	execute(mem);


}

void execute(struct memory *mem)
{
    //sleep(1);
    /** to be implemented **/
    printf("CPU Start!");
    
    pthread_mutex_lock(&mem_lock);
    printf("TEST MAIN MEM ACCES: main_mem[0] -> %s\n",main_mem[0]);
    pthread_mutex_unlock(&mem_lock);
    return;
}
   

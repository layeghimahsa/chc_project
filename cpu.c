#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <pthread.h>
#include <string.h>
#include "cpu.h"
#include "2by2sim.h"

int cpu_num;


void *CPU_start(struct cpu *CPU){
	
	/*printf("\nCPU NUM %d\n",CPU->cpu_num);
	printf("	Task Addr: %d\n",CPU->addr);
	printf("	Task size: %d\n\n",CPU->size);

	int stack[CPU->size];
	int i = 1;
	int j = CPU->addr+1;

	pthread_mutex_lock(&mem_lock);	
	stack[0] = main_mem[CPU->addr];
	while(main_mem[j] != 2147483647){
		stack[i] = main_mem[j];
		i++;
		j++;
	}
	

	pthread_mutex_unlock(&mem_lock);

	for(int i = 0; i<CPU->size; i++){
		printf("stack[%d]: %d\n", i ,stack[i]); 
        }
	//execute(mem, CPU);
	//print_code_stack();
	//struct datum_ir *IR_node; */
}





















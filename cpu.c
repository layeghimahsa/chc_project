#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <pthread.h>
#include <string.h>
#include "cpu.h"
#include "2by2sim.h"

int cpu_num;


void *CPU_start(struct cpu *CPU){
	
	printf("CPU NUM %d\n",CPU->cpu_num);
	printf("	Task Addr: %d\n",CPU->addr);
	printf("	Task size: %d\n",CPU->size);
	//execute(mem, CPU);
	//print_code_stack();
	//struct datum_ir *IR_node;
}




















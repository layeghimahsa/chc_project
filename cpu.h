#ifndef CPU_H
#define CPU_H

#define MAX_MEM 1024*64
#include <stdbool.h>

int cpu_num;


struct cpu{

	int cpu_number;
	int code[64]; //chunk of code
	int node_size; //determines the size of a node
	bool has_dependent;
	int dependents_num; //specify the number of dependency if has any

	struct cpu *cpu_source;
	struct cpu *cpu_dest;
	//struct cpu *next;
};


void *CPU_start();
//void execute(struct memory *mem,struct cpu *CPU);
void fetch_task();

#endif

#ifndef CPU_H
#define CPU_H

#define MAX_MEM 1024*64

int cpu_num;


struct cpu{
	int assinged_cpu;
	int cpu_dest; 
	
	int dest_node;

	int code[64];
	int node_size;

	struct cpu *next;
};


void *CPU_start();
//void execute(struct memory *mem,struct cpu *CPU);
void fetch_task();

#endif

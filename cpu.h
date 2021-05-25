#ifndef CPU_H
#define CPU_H

#define MAX_MEM 1024*64

int cpu_num;


struct cpu{
	int cpu_num;
	
	unsigned int PC;
	unsigned int SP;
	unsigned int BP;
	
	int cpu_memory;
		
	struct cpu *next; // in case we need to have access to other cpus, but because we use threads it might not be needed... not sure!

};


void *CPU_start();
//void execute(struct memory *mem,struct cpu *CPU);
void fetch_task();

#endif

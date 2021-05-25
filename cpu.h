#ifndef CPU_H
#define CPU_H

#define MAX_MEM 1024*64

int cpu_num;


struct cpu{
	int cpu_num;
	
	int addr;

};


void *CPU_start();
//void execute(struct memory *mem,struct cpu *CPU);
void fetch_task();

#endif

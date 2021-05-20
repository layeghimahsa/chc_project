#ifndef CPU_H
#define CPU_H

#define MAX_MEM 1024*64

struct memory{
	
	
	int data[MAX_MEM];
	
};

struct cpu{
	
	
	unsigned int PC;
	unsigned int SP;
	unsigned int BP;
	
	struct memory cpu_memory;
		
	struct cpu *next; // in case we need to have access to other cpus, but because we use threads it might not be needed... not sure!

};


void *CPU_start();
void execute(struct memory *mem);

#endif

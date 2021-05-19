#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <pthread.h>


extern int main_mem[mem_size];

struct memory{
	
	static unsigned int MAX_MEM = 1024 * 64;
	int data[MAX_MEM];
	
};

struct cpu{
	
	
	unsigned int PC;
	unsigned int SP;
	unsigned int BP;
	
	struct memory cpu_memory;
		
	struct cpu *next; // in case we need to have access to other cpus, but because we use threads it might not be needed... not sure!

};


void execute(memory &mem)
{
    sleep(1);
    /** to be implemented **/
    return;
}
   

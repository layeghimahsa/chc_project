#ifndef TWOBYTWO_SIM_H
#define TWOBYTWO_SIM_H


//main mem size in bytes
#define MAIN_MEM_SIZE 1024
//instruction set size in bytes
#define INSTRUCTION_SIZE 8
#define NUM_CPU 4

//variables shared to CPU
extern int main_mem[MAIN_MEM_SIZE];
extern pthread_mutex_t mem_lock;
extern int cpu_assigned[4];
extern int cpu_generated;

//function to populate main mem with instruction set of function
int populate();

int select_task();
int size(int addr);
struct cpu *generate_list();

#endif

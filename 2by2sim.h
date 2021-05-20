#ifndef TWOBYTWO_SIM_H
#define TWOBYTWO_SIM_H


//main mem size in bytes
#define MAIN_MEM_SIZE 1024
//instruction set size in bytes
#define INSTRUCTION_SIZE 8


//variables shared to CPU
extern char main_mem[MAIN_MEM_SIZE][INSTRUCTION_SIZE];
extern pthread_mutex_t mem_lock;

//function to populate main mem with instruction set of function
int populate();

#endif

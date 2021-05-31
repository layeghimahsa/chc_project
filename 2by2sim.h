#ifndef TWOBYTWO_SIM_H
#define TWOBYTWO_SIM_H


//main mem size in bytes
#define MAIN_MEM_SIZE 1024
//instruction set size in bytes
#define INSTRUCTION_SIZE 8
#define NUM_CPU 4

//variables shared to CPU
//extern int main_mem[MAIN_MEM_SIZE];
extern pthread_mutex_t mem_lock;
extern int cpu_assigned[4];
extern int cpu_generated;


int size(int addr); //determin the size of a node 
int find_dest_node(int end); //fid a given nodes destination node in list (used to help scheduling) 
struct cpu *generate_list(); //generates a list of cpu strcts that will be used to launch threads
void schedule_nodes(struct cpu *list); //map nodes to cpu's. this is the crux and backbone of the sim
void refactor_destinations(struct cpu *current, struct cpu *top, int node_num); //set cpu dest and address for destination node stack
void print_nodes(struct cpu *list); //PRETTY PRINTER! 

#endif

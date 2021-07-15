#ifndef TWOBYTWO_SIM_H
#define TWOBYTWO_SIM_H


//main mem size in bytes
//#define MAIN_MEM_SIZE 1024
//instruction set size in bytes
//#define INSTRUCTION_SIZE 8
//#define QUEUE_LENGTH 10

#define CPU_AVAILABLE 0
#define CPU_UNAVAILABLE 1
#define CPU_IDLE 2

#define DUMMY_NODE -5

#define NODE_BEGIN_FLAG 2147483647

#define TEMP_A 0 //tells cpu thats its destination node hasent been assinged yet


// The queue which is a pointer to the front and rear node
struct Queue {
    int size;
    struct Message_capsul *front, *rear;
};

//variables shared to CPU
//extern int main_mem[MAIN_MEM_SIZE];
extern pthread_mutex_t mem_lock;

extern struct Queue* cpu_queue1;
extern struct Queue* cpu_queue2;
extern struct Queue* cpu_queue3;
extern struct Queue* cpu_queue4;



int size(int addr); //determin the size of a node
int find_dest_node(int end); //fid a given nodes destination node in list (used to help scheduling)
struct AGP_node *generate_list(); //generates a list of cpu strcts that will be used to launch threads
//void schedule_nodes(); //map nodes to cpu's. this is the crux and backbone of the sim
void refactor_destinations(struct AGP_node *current, struct AGP_node *top); //set cpu dest and address for destination node stack
void print_nodes(struct AGP_node *nodes); //PRETTY PRINTER!
void propagate_death(int node_num); //remove failed if or els nodes that no longer have to be processed
void mark_as_dead(int node_num);
void writeMem(int ind, int val);
struct AGP_node * schedule_me(int cpu_num);
//queue related functions
struct Message_capsul* newNode(struct Message_capsul *out);
struct Queue* createQueue();
void enQueue(struct Queue* q, struct Message_capsul *out);
struct Message_capsul* deQueue(struct Queue* q);
void nodes_never_ran();

#endif

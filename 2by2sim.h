#ifndef TWOBYTWO_SIM_H
#define TWOBYTWO_SIM_H


//main mem size in bytes
#define MAIN_MEM_SIZE 1024
//instruction set size in bytes
#define INSTRUCTION_SIZE 8
#define NUM_CPU 4
#define QUEUE_LENGTH 10


// A linked list node
/*struct QNode {
    int value;
    int dest; //holds the cpu_dest
    struct QNode* next;
};*/
  
// The queue which is a pointer to the front and rear node
struct Queue {
    int size;
    struct cpu_out *front, *rear;
};

//variables shared to CPU
//extern int main_mem[MAIN_MEM_SIZE];
extern pthread_mutex_t mem_lock;
extern int cpu_assigned[4];
extern int cpu_generated;

extern struct Queue* cpu_queue1;
extern struct Queue* cpu_queue2;
extern struct Queue* cpu_queue3;
extern struct Queue* cpu_queue4;

/*extern int queue_cpu1[QUEUE_LENGTH];
extern int queue_cpu2[QUEUE_LENGTH];
extern int queue_cpu3[QUEUE_LENGTH];
extern int queue_cpu4[QUEUE_LENGTH];*/



int size(int addr); //determin the size of a node 
int find_dest_node(int end); //fid a given nodes destination node in list (used to help scheduling) 
struct cpu *generate_list(); //generates a list of cpu strcts that will be used to launch threads
void initialize_connection(struct cpu *list);
void schedule_nodes(struct cpu *list); //map nodes to cpu's. this is the crux and backbone of the sim
void refactor_destinations(struct cpu *current, struct cpu *top, int node_num); //set cpu dest and address for destination node stack
void print_nodes(struct cpu *list); //PRETTY PRINTER! 

//queue related functions
struct cpu_out* newNode(struct cpu_out *out);
struct Queue* createQueue();
void enQueue(struct Queue* q, struct cpu_out *out);
struct cpu_out* deQueue(struct Queue* q);

#endif

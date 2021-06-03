#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <pthread.h>
#include <assert.h>
#include <string.h>

#include "2by2sim.h"
#include "cpu.h"

//array of 1024 lines with 64 bit words
int cpu_generated;
int cpu_available[NUM_CPU+1] = {0,0,0,0,0};
pthread_mutex_t mem_lock;  //main mem mutex

struct Queue* cpu_queue1;
struct Queue* cpu_queue2;
struct Queue* cpu_queue3;
struct Queue* cpu_queue4;


/* MUTEX commands

int pthread_mutex_init(pthread_mutex_t *restrict mutex, const pthread_mutexattr_t *restrict attr);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
int pthread_mutex_destroy(pthread_mutex_t *mutex);

*/

//DO NOT REMOVE THE LINE BELLOW!! File may become corrupt if it is (used to write code array in)
//CODE BEGINE//
int code[] = {//End main:
0x7fffffff,
0x0,
0x7,
0x20,
0xc,
0x0,
0x1,
0xc,
0x7fffffff,
0x0,
0x7,
0x20,
0xc,
0x0,
0x1,
0x8,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0xffffffff
//Start main @(0):
};
int code_size = 26;
int dictionary[][3] = {{0,26,3}
};
//CODE END//
//DO NOT REMOVE THE LINE ABOVE!!//

int size(int addr){
	//find size
	int i = addr + 1;
	int size = 1;
	while(code[i] != 2147483647 && i < code_size){
		size++;
		i++;
	}
	return size;
}

int find_dest_node(int end){

	int dest = code_size - (end/4);
	int count = 0;
	for(int i = 0; i <= dest; i++){
		if(code[i] == 2147483647){
			count++;
		}
	}
	return count;
}


struct cpu *generate_list(int i){

	if(i >= code_size){
		return NULL;
	} else {
		struct cpu *return_node = (struct cpu *)malloc(sizeof(struct cpu));
		return_node->node_size = size(i); 
		return_node->code_address = i;
		
		for(int j=0; j<return_node->node_size; j++){
			return_node->code[j] = code[i];
			i++;
		}
		
		if(return_node->code[return_node->node_size-1] == 0){
			return generate_list(i);//dont create a useless node
		}else if(return_node->code[return_node->node_size-1] == -1){
			return_node->dest_node = -99; //write to mem
		}else{
			return_node->dest_node = find_dest_node(return_node->code[return_node->node_size-1]);
		}
		
		
		return_node->assigned_cpu = -1;
		return_node->cpu_dest = -1;

		return_node->next = generate_list(i);
		

		return return_node;
	}
}

//this is the function that mappes nodes to cpu
//likely the root of any preformance
//first iteration is very simple and mappes in a linear fassion; node 1 -> cpu 1 , node 2 -> cpu 2 ... node n -> cpu -> n
void schedule_nodes(struct cpu *list){
	
	int cpu_scheduled = 0;
	struct cpu *current = list;

	while(current != NULL && cpu_scheduled <= NUM_CPU){
		printf("cpu_scheduled: %d\n",cpu_scheduled+1);
		current->assigned_cpu = cpu_scheduled + 1;
		cpu_scheduled++;
		current = current->next;
	}
}

void refactor_destinations(struct cpu *current, struct cpu *top, int node_num ){
	if(current == NULL){

	}else{
		if(current->dest_node == -99){ //return to main mem since there are no dependants
			current->cpu_dest = -99; //main mem
		}else{
			struct cpu *temp = (struct cpu *)malloc(sizeof(struct cpu));
			int node_count;
			if(node_num < current->dest_node){
				temp = current;
				node_count = node_num;
			}else{
				temp = top;
				node_count = 1;
			}
			
			while(node_count != current->dest_node){
				temp = temp->next; node_count++;
			}
			current->cpu_dest = temp->assigned_cpu;
			
			//now we must cange the satck destination to match the node stack rather than the full code stack
			
			int dest = code_size - (current->code[current->node_size-1]/4) - 1;
			int count = 0;
			while(code[dest] != 2147483647){
				count++; dest--;
			}
			dest = (current->node_size - count + 1)*4;
			printf("DEST: %d\n", dest);
			current->code[current->node_size-1] = dest;
		}
		refactor_destinations(current->next, top, node_num+1);
	}
}

void print_nodes(struct cpu *list){
	if(list == NULL){

	}else{
		printf("\n\nNODE: \n");
		printf(" - CPU assigned: %d\n",list->assigned_cpu);
		printf(" - result dest CPU: %d\n",list->cpu_dest);
		printf(" - node size: %d\n",list->node_size);
		printf(" - dest node: %d\n",list->dest_node);
		printf(" - code:\n");
		for(int i = 0; i< list->node_size; i++){
			printf("    code[%d]: %d\n",i,list->code[i]);
		}
		print_nodes(list->next);
	}
}


int main()
{
    printf("***SIMULATION START***\n\n");

    //create mutex
    if (pthread_mutex_init(&mem_lock, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }
   
    
    for(int i = 0; i<code_size; i++){
	printf("code[%d]: %d\n", i ,code[i]); 
    }
    
    cpu_generated = 0;
    pthread_t thread_id[NUM_CPU];
    int not_last_node = 1;


    printf("\n\nCREATING NODE LIST\n\n"); 
    struct cpu *graph = generate_list(0);
    //print_nodes(graph);


    //creating cpu queues
    cpu_queue1 = createQueue();
    cpu_queue2 = createQueue();
    cpu_queue3 = createQueue();
    cpu_queue4 = createQueue();

    /***********************/
    /*** task scheduling ***/
    /***********************/
    /*
	-64:	0x7fffffff      //new node label
	-60:	0x0		//number of dependencies 
	-5c:	0x7		//value (const 7)
	-58:	0x20		//end address + 1 (next node in graph)
	-54:	0xc		//operation
	-50:	0x0             //number of arguments 
	-4c:	0x1             //expansion or not flag 
	-48:	0x8             //result destination
	-44:	0x7fffffff
	-40:	0x0
	-3c:	0x7
	-38:	0x20
	-34:	0xc
	-30:	0x0
	-2c:	0x1
	-28:	0x4
	-24:	0x7fffffff
	-20:	0x2
	-1c:	0xfffffffc
	-18:	0x24
	-14:	0x3
	-10:	0x2
	-c:	0x0
	-8:	0x0
	-4:	0x0
    */

    printf("\n\nSCHEDULING NODES\n\n");    
    //fuction that schedules nodes to cpu (currently very simple)
    //this function is likely going to determine the preformance of the whole design
    schedule_nodes(graph);
    //print_nodes(graph);

    printf("\n\nREFACTORING NODE DESTINATIONS\n\n");   
    refactor_destinations(graph, graph, 1);
    //print_nodes(graph);

    printf("\n\nLAUNCHING THREADS!!!\n\n"); 
    //simple thread launch since we know more core than nodes 
    struct cpu *list = graph; 
    while(cpu_generated < NUM_CPU && list != NULL){
	pthread_create(&(thread_id[list->assigned_cpu]), NULL, &CPU_start, list);
	cpu_available[list->assigned_cpu] = 1;
	cpu_generated++;
	list = list->next;
    }
    int count = 1;
    while(cpu_generated < NUM_CPU){
	if(cpu_available[count] == 0){
		struct cpu *temp = (struct cpu *)malloc(sizeof(struct cpu));
		temp->assigned_cpu = count;
		temp->code[1] = 1;
		pthread_create(&(thread_id[count]), NULL, &CPU_start, temp);
		cpu_available[count] = 2;
		cpu_generated++;
	}
	count++;
	
	
    }

    /***********************/
    /**** Simulation end ***/
    /***********************/

    //wait for all active cpu threads to finish 
    for(int i = 0; i<NUM_CPU; i++){
	if(cpu_available[i] == 1){
    		pthread_join(thread_id[i], NULL);
	}
    }
    for(int i = 0; i<NUM_CPU; i++){
	if(cpu_available[i] == 2){
    		pthread_cancel(thread_id[i]);
	}
    }
    pthread_mutex_destroy(&mem_lock);

    printf("\n\n***SIMULATION COMPLETE***\n\n");
    return 0;
}


































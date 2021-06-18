#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <pthread.h>
#include <assert.h>
#include <string.h>

#include "2by2sim.h"
#include "cpu.h"

//dynamic code array (starts as copy of original code)
int *runtime_code;
//cpu status array
int *cpu_status;
//used to keep track of node numbers
int list_index = 1;
//This is the number of dead nodes (0 destinations) that were removed (needed for node_dest allignment
int nodes_removed; 
//the list of all tasks (that have more than 0 destinations)
struct AGP_node *program_APG_node_list; 
//main mutex
pthread_mutex_t mem_lock;  


//DO NOT REMOVE THE LINE BELLOW!! File may become corrupt if it is (used to write code array in)
//CODE BEGINE//
const int code[] = {//End main:
0x7fffffff,
0x0,
0x7,
0x28,
0xc,
0x0,
0x3,
0x8,
0xc,
0x7c,
0x7fffffff,
0x0,
0x7,
0x20,
0xc,
0x0,
0x1,
0x78,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0x34,
0x7fffffff,
0x0,
0x16,
0x20,
0xc,
0x0,
0x1,
0x30,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0xffffffff,
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
int code_size = 56;
int dictionary[][3] = {{0,56,6}
};
//CODE END//
//DO NOT REMOVE THE LINE ABOVE!!//

int size(int addr){
	//find size
	int i = addr + 1;
	int size = 1;
	while(code[i] != NODE_BEGIN_FLAG && i < code_size){ 
		size++;
		i++;
	}
	return size;
}

int find_dest_node(int end){

	int dest = code_size - (end/4);
	int count = 0;
	for(int i = 0; i <= dest; i++){
		if(code[i] == NODE_BEGIN_FLAG){
			count++;
		}
	}
	return count;
}


struct AGP_node *generate_list(int i){  

	if(i >= code_size){
		return NULL;
	} else {
		struct AGP_node *return_node = (struct AGP_node *)malloc(sizeof(struct AGP_node));
		return_node->node_size = size(i); 
		return_node->code_address = i;
		return_node->depend = NULL;
		return_node->node_num = list_index;
		return_node->assigned_cpu = UNDEFINED;
		for(int j=0; j<return_node->node_size; j++){
			return_node->code[j] = code[i];
			i++;
		}

		return_node->num_dest = return_node->code[(6+return_node->code[1])];
		printf("Num of return nodes: %d\n",return_node->num_dest);

		if(return_node->code[4] != code_expansion){
			if(return_node->num_dest == 0){
				nodes_removed++;
				return generate_list(i);//dont create a useless node
			}else{
				
				struct Destination *dest = (struct Destination *)malloc(sizeof(struct Destination));
				struct Destination *temp = dest;

				for(int i = 1; i <= return_node->num_dest; i++){
					if(return_node->code[return_node->node_size-i] == -1){
						temp->node_dest = OUTPUT; //write to mem
					}else{
						temp->node_dest = find_dest_node(return_node->code[return_node->node_size-i]);
					}
					temp->cpu_dest = UNDEFINED;
					temp->next = (struct Destination *)malloc(sizeof(struct Destination));
					temp = temp->next;
				}
				//temp = NULL;
				free(temp);
				return_node->dest = dest;
			}
		}else{
			//code expansion setup
		}
		list_index++;
		return_node->next = generate_list(i);

		return return_node;
	}
}

void generate_lookup_table(struct CPU *current, struct Queue **Q){


	switch(current->cpu_num){
			
		case 1:
			current->look_up[0] = Q[0]; //1
			current->look_up[1] = Q[1]; //2
			current->look_up[2] = Q[2]; //3
			current->look_up[3] = Q[1]; //cant send to 4
			break;
		case 2:
			current->look_up[0] = Q[0]; 
			current->look_up[1] = Q[1];
			current->look_up[2] = Q[3]; //cant send to 3
			current->look_up[3] = Q[3]; 
			break;
		case 3:
			current->look_up[0] = Q[0];
			current->look_up[1] = Q[0]; //cant send to 2
			current->look_up[2] = Q[2];
			current->look_up[3] = Q[3]; 
			break;
		case 4:
			current->look_up[0] = Q[2]; //cant send to 1
			current->look_up[1] = Q[1];
			current->look_up[2] = Q[2];
			current->look_up[3] = Q[3]; 
			break;
		default:
			printf("shouldn't happen");
			break;
	}
}

void refactor_destinations(struct AGP_node *current, struct AGP_node *top){
	if(current == NULL){
		printf("cant refactor a null node!!!\n");
	}else{
		struct Destination *dest_struct = current->dest; //getting the list of destinations
		for(int i = 1; i<=current->num_dest;i++){
			if(dest_struct->node_dest == OUTPUT){ //return to main mem since there are no dependants
				dest_struct->cpu_dest = OUTPUT; //main mem
			}else{
				dest_struct->node_dest -= nodes_removed; //updating node_dest in case of any removed nodes
				struct AGP_node *temp = top;
				int node_count = 1;
				
				while(node_count != dest_struct->node_dest){
					temp = temp->next; node_count++;
				}

				//if the destination isnt assigned, the current node must hold the value
				if(temp->assigned_cpu == UNDEFINED)
					dest_struct->cpu_dest = UNKNOWN;
				else
					dest_struct->cpu_dest = temp->assigned_cpu;
				
				//now we must change the satck destination to match the node stack rather than the full code stack
				//this is done even if the cpu isnt assinged yet
				int dest = code_size - (current->code[current->node_size-i]/4) - 1;
				int count = 0;
				while(code[dest] != NODE_BEGIN_FLAG){
					count++; dest--;
				}
				dest = (temp->node_size - count -1)*4;
				printf("DEST: %d\n", dest);
				current->code[current->node_size-i] = dest;

			}

			dest_struct = dest_struct->next;
			
		}
		//refactor_destinations(current->next, top, node_num+1);
	}
}


struct AGP_node *schedule_me(int cpu_num){

	//initial while that we will traverse through and try to find the node we want to schedule
	//count number of possible to be scheduled nodes!
	//if count == 0, create dumy cpu (temp->code[1] = 1; seb) struct that has multiple dependencies, set the cpu status cpu_status [cpu_num-1] = IDLE
	//else (we can schedule), pick the first node we run into (later, nodes which SHOULD be ran, e.g. priority or deadlines) , right now FIFO;  check if -1 ->>> it measns it is still unassigned
	//runtime refactor -> change dest to either cpu numebr or temp (means hold the value) 
	//check number of dependencies, if 0 return cpu. if has dependables, do they know their destination? if yes, you can return cpu. if No, make a list of those who have your dependables, return 
	// structure cpu
	
	
	struct AGP_node *current = program_APG_node_list;
	int unode_num = 0; //number of unscheduled nodes
	
	/*finding unscheduled nodes and store them into a new list*/
	while(current != NULL){ 
		
		if(current->assigned_cpu == UNDEFINED){
			unode_num++;
			break;
		}
		
		current = current->next;
	}
	//if there is no node to be left to be scheduled
	if(unode_num == 0){
		printf("no more nodes to assign!! sending CPU %d a dummy node\n",cpu_num);
		struct AGP_node *dummy = (struct AGP_node *)malloc(sizeof(struct AGP_node));
		dummy->assigned_cpu = cpu_num;
		dummy->code[1] = 1;
		cpu_status[cpu_num-1] = CPU_IDLE; //there are no nodes left! go to idle mode.
		return dummy;
	} else{ //there is some unassigned nodes
		
		//runtime_refactor(); 
		
		if(current->code[1] == 0){//if the node has no dependent
			current->assigned_cpu = cpu_num;
			refactor_destinations(current, program_APG_node_list);
			cpu_status [cpu_num-1] = CPU_UNAVAILABLE;
			struct AGP_node *return_node = (struct AGP_node *)malloc(sizeof(struct AGP_node));
			*return_node = *current;
			return return_node;
		} else{ //if the node has dependables
		
			struct AGP_node *temp = program_APG_node_list;
			
			struct Dependables *depe = (struct Dependables *)malloc(sizeof(struct Dependables));
			struct Dependables *dep = depe;

			while(temp != NULL){
				struct Destination *dest = temp->dest;

				for(int i = 0; i< temp->num_dest; i++){
					if(dest->node_dest == current->node_num){
						if(dest->cpu_dest == UNDEFINED || dest->cpu_dest == UNKNOWN){
							dep->cpu_num = temp->assigned_cpu; //cpu that has that variable
							dep->node_needed = temp->node_num; //variable name to be requested
							dep->next = (struct Dependables *)malloc(sizeof(struct Dependables));
							dep = dep->next;
						}
					}
					dest = dest->next;
				}
				temp = temp->next;
			}
			current->depend = depe;
			//return the cpu.
			current->assigned_cpu = cpu_num;
			refactor_destinations(current, program_APG_node_list);
			cpu_status [cpu_num-1] = CPU_UNAVAILABLE;
			//return copy of node, not actual node
			struct AGP_node *return_node = (struct AGP_node *)malloc(sizeof(struct AGP_node));
			*return_node = *current;
			return return_node;
			
		}
	
	}

}


void writeMem(int ind, int val){
	runtime_code[ind] = val;
	printf("WRITING BACK TO MEMORY...\n");
	printf("code[%d] = %d\n",ind, runtime_code[ind]);
}


void print_nodes(struct AGP_node *nodes){
	if(nodes == NULL){

	}else{
		printf("\n\nNODE: \n");
		printf(" - CPU assigned: %d\n",nodes->assigned_cpu);
		printf(" - Node number: %d\n",nodes->node_num);
		printf(" - code main mem addr: %d\n", nodes->code_address);
		printf(" - node size: %d\n",nodes->node_size);
		printf(" - number of dest: %d\n",nodes->num_dest);
		printf(" - code:\n");
		for(int i = 0; i< nodes->node_size; i++){
			printf("    code[%d]: %d\n",i,nodes->code[i]);
		}
		struct Destination *temp = nodes->dest;
		for(int i = 0; i < nodes->num_dest; i++){
			printf(" - Destination %d:\n    node dest: %d\n    cpu dest: %d\n",i,temp->node_dest, temp->cpu_dest);
			temp = temp->next;
		}
		print_nodes(nodes->next);
	}
}


int main(int argc, char **argv)
{
    printf("***SIMULATION START***\n\n");


    if(argc < 1){
	printf("You must enter the number of CPU you want\nex: ./sim 4 (to run a 2x2 sim with 4 cores)");
	return 1;
    }

    //create mutex
    if (pthread_mutex_init(&mem_lock, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }
   
    int NUM_CPU = atoi(argv[1]);
    if(NUM_CPU < 1){
	printf("NODE NUM %d\n",NUM_CPU);
	printf("YOU MUST HAVE AT LEAST 1 CPU\n");
	return 1;
    }
    
    printf("CREATING A %dx%d SIMULATION\n",NUM_CPU/2,NUM_CPU/2);


    printf("\n\nSETTING UP ENVIRONMENT\n\n"); 


    //create array of thread id
    pthread_t thread_id[NUM_CPU];

    nodes_removed = 0; 

    //create status array 
    cpu_status = (int *)malloc(sizeof(int) * NUM_CPU);

    for(int i = 0; i<NUM_CPU; i++){
	cpu_status[i] = CPU_AVAILABLE;
    }

    //instantiate queues for all CPUs
    struct Queue *cpu_queues[NUM_CPU];
    for(int i = 0; i<NUM_CPU; i++){
	cpu_queues[i] = createQueue();
    }

    //create cpu struct 
    struct CPU *cpus[NUM_CPU];
    for(int i = 0; i<NUM_CPU; i++){
	struct CPU *cpu_t = (struct CPU*)malloc(sizeof(struct CPU));
        cpu_t->cpu_num = i+1;
	generate_lookup_table(cpu_t, cpu_queues);
	cpus[i] = cpu_t;
    }

    //TODO: copy static code array into a new array that can be modified 
    runtime_code = (int *)malloc(sizeof(int) *code_size);
    for(int i = 0; i<code_size; i++){
	printf("code[%d]: %d\n", i ,code[i]); 
	runtime_code[i] = code[i];
    }

    printf("\n\nCREATING NODE LIST\n\n"); 
    program_APG_node_list = generate_list(0);
    print_nodes(program_APG_node_list);

    printf("\n\nSCHEDULING NODES\n\n");    
    for(int i = 0; i<NUM_CPU; i++){
	cpus[i]->node_to_execute = schedule_me(cpus[i]->cpu_num);
    }
  
    printf("\n\nREFACTORING NODE DESTINATIONS\n\n");   
    for(int i = 0; i<NUM_CPU; i++){
    	refactor_destinations(cpus[i]->node_to_execute, program_APG_node_list);
    }
    print_nodes(program_APG_node_list);

    printf("\n\nLAUNCHING THREADS!!!\n\n"); 
    for(int i = 0; i<NUM_CPU; i++){
	pthread_create(&(thread_id[cpus[i]->cpu_num-1]), NULL, &CPU_start, cpus[i]);
        if(cpus[i]->node_to_execute->node_num == DUMMY_NODE)
		cpu_status[cpus[i]->cpu_num-1] = CPU_IDLE;
	else
		cpu_status[cpus[i]->cpu_num-1] = CPU_UNAVAILABLE;
    }
    
    /***********************/
    /**** Simulation end ***/
    /***********************/

    //wait for all active cpu threads to finish
    int num_cpu_idle = 0;
    while(num_cpu_idle < NUM_CPU){
	num_cpu_idle = 0;
	for(int i = 0; i<NUM_CPU; i++){
		if(cpu_status[i] == CPU_IDLE)
	    		num_cpu_idle++;
        }
	
	//can do other busy work while sim continues '\/('_')\/' 
	
    }

    for(int i = 0; i<NUM_CPU; i++){
	pthread_cancel(thread_id[i]); //cancel all threads 
	pthread_join(thread_id[i], NULL); //wait for all threads to clean and cancel safely 
		
    }

    pthread_mutex_destroy(&mem_lock);
    
    
    puts("\nPRINTING CODE ARRAY\n"); // want to check if result 14 is written to memory (code array)
    for(int i = 0; i<code_size; i++){
	printf("code[%d]: %d\n", i ,runtime_code[i]); 
    }


    printf("\n\n***SIMULATION COMPLETE***\n\n");
    return 0;
}

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
































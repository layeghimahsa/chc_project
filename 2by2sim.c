#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <pthread.h>
#include <assert.h>
#include <string.h>

#include "2by2sim.h"
#include "cpu.h"

//array of 1024 lines with 64 bit words
int main_mem[MAIN_MEM_SIZE];
int cpu_generated;
int cpu_available[NUM_CPU] = {0,0,0,0};
pthread_mutex_t mem_lock;  //main mem mutex


/* MUTEX commands

int pthread_mutex_init(pthread_mutex_t *restrict mutex, const pthread_mutexattr_t *restrict attr);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
int pthread_mutex_destroy(pthread_mutex_t *mutex);

*/

//CODE BEGINE//
int code[] = {//End main:
0x7fffffff,
0x0,
0x7,
0x20,
0xc,
0x0,
0x1,
0x8,
0x7fffffff,
0x0,
0x7,
0x20,
0xc,
0x0,
0x1,
0x4,
0x7fffffff,
0x2,
0xfffffffc,
0x24,
0x3,
0x2,
0x0,
0x0,
0x0
//Start main @(0):
};
int code_size = 25;
int dictionary[][3] = {{0,25,3}
};
//CODE END//


int populate(){

    int i;
    for( i = 0; i<25; i++){
	main_mem[i] = code[i];
    } 
    main_mem[i+1] = -99;

    return 1;
	
}




int select_task(){
	int i = 0;
	while(main_mem[i] != -99){
		if(main_mem[i] == 2147483647){  //0x7ffffffff
			if(main_mem[i+1] == 0){
				main_mem[i+1] = -1; //assigned
				
				
				return i;
			}else{
				i += 5;
			}	
		}
		i++;
	}
	printf("no task found\n");
	return -1;

}


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
	}else{
	struct cpu *return_node = (struct cpu *)malloc(sizeof(struct cpu));
	return_node->node_size = size(i); 
	
	for(int j=0; j<return_node->node_size; j++){
		return_node->code[j] = code[i];
		i++;
	}
	
	return_node->dest_node = find_dest_node(return_node->code[return_node->node_size-1]);
	
	return_node->assinged_cpu = -1;
	return_node->cpu_dest = -1;

	
	return_node->next = generate_list(i);
	

	return return_node;
	}
}

void print_list(struct cpu *list){
	if(list == NULL){

	}else{
		printf("\n\nNODE: \n");
		printf(" - CPU assigned: %d\n",list->assinged_cpu);
		printf(" - result dest CPU: %d\n",list->cpu_dest);
		printf(" - node size: %d\n",list->node_size);
		printf(" - dest node: %d\n",list->dest_node);
		printf(" - code:\n");
		for(int i = 0; i< list->node_size; i++){
			printf("    code[%d]: %d\n",i,list->code[i]);
		}
		print_list(list->next);
	}
}

int main()
{
    printf("***SIMULATION START***\n\n");

    if(populate() == 0){
	printf("Failed to populate memory\n");
	printf("\n\n***SIMULATION EXIT WITH ERROR***\n\n");
	return 0;
    }

    //create mutex
    if (pthread_mutex_init(&mem_lock, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }
   
    
    for(int i = 0; i<30; i++){
	printf("code[%d]: %d\n", i ,main_mem[i]); 
    }
    
    cpu_generated = 0;
    pthread_t thread_id[NUM_CPU];
    int not_last_node = 1;

    struct cpu *graph = generate_list(0);
    print_list(graph);






    /*
    while(not_last_node){
	
	if(cpu_generated == NUM_CPU){
		//sleep(0.01);
	}else{
		//struct cpu *task = select_task();
		struct cpu *task = (struct cpu *)malloc(sizeof(struct cpu));
		task->addr = select_task();
		task->size = size(task->addr);
		if(task->addr == -1){
		}else{
			int ready = 0;
			for(ready = 0; ready<NUM_CPU; ready++){
				if(cpu_available[ready] == 0){
					cpu_available[ready] = 1;					
					break;
				}
					
			} 
			task->cpu_num = ready;
			pthread_create(&(thread_id[ready]), NULL, &CPU_start, task);
			cpu_generated++;
		}
	}
	if(cpu_generated == 2)
		not_last_node = 0;
	sleep(0.01);
	


    }*/

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
    //printf("CPU num: %d\n" , thread_id[0].cpu_num);
    //char* selected_task = select_task();
    /***********************/
    /**** Simulation end ***/
    /***********************/
    for(int i = 0; i<NUM_CPU; i++){
	if(cpu_available[i] != 0){
    		pthread_join(thread_id[i], NULL);
	}
    }
    pthread_mutex_destroy(&mem_lock);

    printf("\n\n***SIMULATION COMPLETE***\n\n");

    //for(int i = 0; i<25; i++){
//	printf("code[%d]: %x\n", i ,code[i]); 
    //}
    return 0;
}


































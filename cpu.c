#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <pthread.h>
#include <string.h>
#include "cpu.h"
#include "2by2sim.h"



void *CPU_start(struct cpu *CPU){
	
	printf("CPU %d 	START!!\n",CPU->assigned_cpu);
	
	//instantiate cache
	int entry_pos; //this kinda actas as a time tracker since this will show the most recent entry, making the next one the oldest 
	int cache[4][8]; //this can hold 8 values that have no destination yet so it can move on to new nodes
	entry_pos = 0;
	for(int i = 0; i<8; i++){
		cache[0][i] = -1;//node number (technically the variable name)
		cache[1][i] = -1;//node stack address for its dest node
		cache[2][i] = -1;//node value
		cache[3][i] = -1;
	}
	
	while(1){
		//everything should be in this loop		
	
	printf("CPU %d DOING NODE %d\n",CPU->assigned_cpu,CPU->node_num);
	//check if there are requests to make, and if yes make em
	struct depend *dep = CPU->dependables;
	while(dep != NULL){ //should be null if no requests to make 
		
		printf("CPU %d has dependants to request\n",CPU->assigned_cpu);

		if(dep->cpu_num == CPU->assigned_cpu){ //fetch from your own cache!!
			for(int i = 0; i<8; i++){
				if(cache[0][i] == dep->node_num){
					int addr = CPU->node_size - (cache[1][i]/4) - 1;
					CPU->code[addr] =  cache[2][i];
					CPU->code[1]--; //7. decrese the number of dependent
					break;
				}
			}
		}else{
			struct cpu_out *output = (struct cpu_out *)malloc(sizeof(struct cpu_out)); 
			
			output->value = CPU->assigned_cpu;
			output->dest = dep->cpu_num;
			output->addr = dep->node_num;
			output->node_num = CPU->node_num;
			output->message_type = REQUEST;
			printf("CPU %d has sent var request to CPU %d\n",CPU->assigned_cpu, output->dest);
			pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
			pthread_mutex_lock(&mem_lock); // locking the mutex for the purpose of reading and writing to the queues

			enQueue(CPU->look_up[dep->cpu_num-1], output);

			pthread_mutex_unlock(&mem_lock); // unlocking the mutex!
			//enable cancelation
			pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
		}
		
		dep = dep->next;
		if(dep->cpu_num==0)
			break;
	}
	
	int count = 0;
	
	do { //while waiting for dependent
		sleep(0.01);
		// -check port for dependent! (checking the queue) , reset the count and reducce the number of dependents 
		//setting up the queue and periodically checking them

		//dissable cancel since it will be using mutexes and we dont wanna cancel while a mutex is locked
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);


		if(CPU->look_up[CPU->assigned_cpu-1]->size > 0){
			//printf("CPU %d wawawiwa\n",CPU->assigned_cpu);
			struct cpu_out *temp;
			pthread_mutex_lock(&mem_lock);
			temp = deQueue(CPU->look_up[CPU->assigned_cpu-1]);
			if(CPU->assigned_cpu != temp->dest){
				enQueue(CPU->look_up[temp->dest-1], temp);
			}else{
				if(temp->message_type == REQUEST){
					if(temp->addr == CPU->node_num){//requesting currently being processed node
						enQueue(CPU->look_up[CPU->assigned_cpu-1], temp);
					}else{
						struct cpu_out *output = (struct cpu_out *)malloc(sizeof(struct cpu_out)); 
						//addr is the requested node num (node name)
						for(int i = 0; i<8; i++){
							if(cache[0][i] == temp->addr && cache[3][i] == temp->node_num){
								output->value = cache[2][i];
								output->dest = temp->value;
								output->addr = cache[1][i]; 
								output->node_num = temp->node_num;
								output->message_type = RESULT;
								cache[0][i] = -1; //remove val
								break;
							}
						}
						enQueue(CPU->look_up[temp->value-1], output);
					}
					
				}else{
					if(temp->node_num != CPU->node_num){//requesting currently being processed node
						enQueue(CPU->look_up[CPU->assigned_cpu-1], temp);
					}else{
						int addr = CPU->node_size - (temp->addr/4) - 1;
						CPU->code[addr] = temp->value;
						count = 0; //6.reset the count
						CPU->code[1]--; //7. decrese the number of dependents
						printf("CPU %d dependents: %d\n",CPU->assigned_cpu, CPU->code[1]); //printing the updated dependents number
					}
				}
			}
			pthread_mutex_unlock(&mem_lock); // unlocking the mutex!
		}
		
		
		
		//enable cancelation
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
		
		if(count > 500){
			printf("CPU %d TIMER TIMEOUT!\n", CPU->assigned_cpu );
			return 0; //might be changed
		}
			
		count++;
	} while(CPU->code[1] > 0);
	

	int operation = CPU->code[4];
	//printf("OPERATION: %d\n",operation);	
	switch(operation)
	{
		//case code_input: 		printf("\t<< "); scanf("%d", CPU->code[2]); break;
		case code_plus: 		CPU->code[2] = CPU->code[6]+ CPU->code[7]; break;
		case code_minus:		CPU->code[2] = CPU->code[6] - CPU->code[7]; break;
		case code_times:		CPU->code[2] = CPU->code[6] * CPU->code[7]; break;
		case code_is_equal:		(CPU->code[6] == CPU->code[7]) ? (CPU->code[2]=1): (CPU->code[2]=0); break;
		case code_is_less:		(CPU->code[6] < CPU->code[7]) ? (CPU->code[2]=1): (CPU->code[2]=0); break;
		case code_is_greater:	(CPU->code[6] > CPU->code[7]) ? (CPU->code[2]=1): (CPU->code[2]=0); break;
		case code_if:			if((CPU->code[6] != 0))
								{ 
									(CPU->code[2] = CPU->code[7]);
								}
								else
								{ 
									
									//propagate_death(CPU->code[0]); 
									CPU->code[1] = DEAD; 
								} break; 
		case code_else:			if(CPU->code[6] == 0)
								{
									(CPU->code[2] = CPU->code[7]);
								}
								else
								{ 
									
									//propagate_death(CPU->code[0]);
									CPU->code[1] = DEAD; 
								} break;

		//TODO: Fix merge so it has a single argument
		case code_merge:		CPU->code[2] = (CPU->code[6] | CPU->code[7]); break;
		case code_identity:		break; //do nothing since its an identity
		default: 
		{ 
			printf("Error: CPU %d has unknown code found during interpretation\n	Operation: %d\n",CPU->assigned_cpu,operation);
			sleep(10);
		}
	}
		
	struct DEST *dest = CPU->dest;
	for(int i = 1; i<=CPU->num_dest;i++){

		struct cpu_out *output;
		output = (struct cpu_out *)malloc(sizeof(struct cpu_out));
		output->value = CPU->code[2];
		output->dest = dest->cpu_dest;//cpu number, it is either a cpu number or -99 for writing back to memory
		output->addr = CPU->code[CPU->node_size-i];//stack destination address, it is either a positive offset or -1 for "writing back to memory" state
		output->node_num = dest->node_dest;
		output->message_type = RESULT;

		//dissable cancel since it will be using mutexes and we dont wanna cancel while a mutex is locked
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		
		//not has dependent and should write its value in the queue
		if(dest->cpu_dest == -99){
			printf("CPU %d Writing to Main MEM!!\n",CPU->assigned_cpu);
			//updating CPU code (local memory)
			
			//writing back to memory (code array)
			pthread_mutex_lock(&mem_lock);
			writeMem(CPU->code_address+2, output->value);
			pthread_mutex_unlock(&mem_lock);

			for(int i = 0; i< CPU->node_size; i++){
			printf("    code[%d]: %d\n",i,CPU->code[i]);
			}
		
		}else if(dest->cpu_dest == 0){
			//save the value and node name (node number) for when its called upon
			printf("CPU %d STORING MEM!!\n",CPU->assigned_cpu);
			if(entry_pos == 7){
				cache[0][0] = CPU->node_num;
				cache[1][0] = output->addr;
				cache[2][0] = output->value;
				cache[3][0] = output->node_num;
				entry_pos = 0;
			}else{
				//entry pos + 1 should be technically the oldest entry 
				cache[0][entry_pos+1] = CPU->node_num;
				cache[1][entry_pos+1] = output->addr;
				cache[2][entry_pos+1] = output->value;
				cache[3][entry_pos+1] = output->node_num;
				entry_pos++;
			}
			
		}else{ // we need to calculate stuff here after everything has poped up in the queue
			enQueue(CPU->look_up[output->dest-1], output);
		}
		
		//enable cancelation
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

		dest = dest->next;
	}

		//request a new task
		printf("CPU %d has requested a new task\n",CPU->assigned_cpu);
		CPU = schedule_me(CPU->assigned_cpu);
		//make thread cancelable 
	}
	
	
}


// function to create a new linked list node.
struct cpu_out* newNode(struct cpu_out *out)
{
    struct cpu_out* temp = (struct cpu_out*)malloc(sizeof(struct cpu_out));
    temp->value = out->value;
    temp->dest = out->dest;
    temp->addr = out->addr;
    temp->node_num = out->node_num;
    temp->message_type = out->message_type;
    temp->next = NULL;
    return temp;
}

// function to create an empty queue
struct Queue* createQueue()
{
    struct Queue* q = (struct Queue*)malloc(sizeof(struct Queue));
    q->front = q->rear = NULL;
    q->size = 0;
    return q;
}


// function to add a value to queue
void enQueue(struct Queue* q, struct cpu_out *out)
{
    // Create a new LL node
    struct cpu_out* temp = newNode(out);
  
    // If queue is empty, then new node is front and rear both
    if (q->rear == NULL) {
        q->front = q->rear = temp;
        q->size += 1;
        return;
    }
  
    // Add the new node at the end of queue and change rear
    q->rear->next = temp;
    q->rear = temp;
    q->size += 1;
}

// Function to remove a value from queue
struct cpu_out* deQueue(struct Queue* q)
{
    struct cpu_out* result = (struct cpu_out*)malloc(sizeof(struct cpu_out));;
    // If queue is empty, return NULL.
    if (q->front == NULL)
        return NULL;
  
    // Store previous front and move front one node ahead
    struct cpu_out* temp = q->front;
    result->value = temp->value;
    result->dest = temp->dest;
    result->addr = temp->addr;
    result->node_num = temp->node_num;
    result->message_type = temp->message_type;
  
    q->front = q->front->next;
    q->size -= 1;
  
    // If front becomes NULL, then change rear also as NULL
    if (q->front == NULL)
        q->rear = NULL;
  
    free(temp);
    return result;
}


  

















#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <pthread.h>
#include <string.h>
#include "cpu.h"
#include "2by2sim.h"



void *CPU_start(struct CPU *cpu){
	
	printf("CPU %d 	START!!\n",cpu->cpu_num);

	for(int i = 0; i<LOCAL_STORAGE_SIZE; i++){
		cpu->local_mem[0][i] = UNDEFINED;
	}

	
	while(1){	
	
	struct AGP_node *NTE = cpu->node_to_execute;

	printf("CPU %d DOING NODE %d\n",cpu->cpu_num,NTE->node_num);

	//check if there are requests to make, and if yes make em
	struct Dependables *dep = NTE->depend;
	while(dep != NULL){ //should be null if no requests to make 
		
		if(dep->cpu_num==0){
			break;
		}else if(dep->cpu_num == cpu->cpu_num){ //fetch from your own cache!!
			printf("CPU %d fetching variable from local mem\n",cpu->cpu_num);
			for(int i = 0; i<8; i++){
				if(cpu->local_mem[0][i] == dep->node_needed){
					int addr = NTE->node_size - (cpu->local_mem[1][i]/4) - 1;
					NTE->code[addr] =  cpu->local_mem[2][i];
					cpu->local_mem[0][i] = UNDEFINED;
					NTE->code[1]--; //7. decrese the number of dependent
					break;
				}
			}
		}else{
			printf("CPU %d sending variable request to CPU %d\n",cpu->cpu_num, dep->cpu_num); 

			struct Message_capsul *request = (struct Message_capsul *)malloc(sizeof(struct Message_capsul)); 
			request->value = cpu->cpu_num; //cpu num to send reult back to (its self)
			request->dest = dep->cpu_num;  //request destination
			request->addr = dep->node_needed; //request variable name
			request->node_num = NTE->node_num; //node number thats requesting (to get correct version of var)
			request->message_type = REQUEST;
			pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

			pthread_mutex_lock(&mem_lock);
			enQueue(cpu->look_up[dep->cpu_num-1], request);
			pthread_mutex_unlock(&mem_lock);
			//enable cancelation
			pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
		}
		
		dep = dep->next;
		
	}
	
	do { //while waiting for dependent
		sleep(0.01);
		// -check port for dependent! (checking the queue) , reset the count and reducce the number of dependents 
		//setting up the queue and periodically checking them

		//dissable cancel since it will be using mutexes and we dont wanna cancel while a mutex is locked
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		pthread_mutex_lock(&mem_lock);
		
		if(cpu->look_up[cpu->cpu_num-1]->size > 0){
			struct Message_capsul *message;
			message = deQueue(cpu->look_up[cpu->cpu_num-1]);
			if(cpu->cpu_num != message->dest){
				enQueue(cpu->look_up[message->dest-1], message);
			}else{
				if(message->message_type == REQUEST){
					if(message->addr == NTE->node_num){//requesting currently being processed node
						enQueue(cpu->look_up[cpu->cpu_num-1], message);
					}else{
						struct Message_capsul *output = (struct Message_capsul *)malloc(sizeof(struct Message_capsul)); 
						int found = 0;
						for(int i = 0; i<8; i++){
							if(cpu->local_mem[0][i] == message->addr && cpu->local_mem[3][i] == message->node_num){
								output->value = cpu->local_mem[2][i];
								output->dest = message->value;
								output->addr = cpu->local_mem[1][i]; 
								output->node_num = message->node_num;
								output->message_type = RESULT;
								cpu->local_mem[0][i] = UNDEFINED; //remove val
								found = 1;
								break;
							}
						}
						if(found==1)
							enQueue(cpu->look_up[message->value-1], output);
						else
							enQueue(cpu->look_up[cpu->cpu_num-1], message);
					}
					
				}else{
					if(message->node_num != NTE->node_num){//requesting currently being processed node
						enQueue(cpu->look_up[cpu->cpu_num-1], message);
					}else{
						int addr = NTE->node_size - (message->addr/4) - 1;
						NTE->code[addr] = message->value;
						NTE->code[1]--; //7. decrese the number of dependents
						printf("CPU %d dependents: %d\n",cpu->cpu_num, NTE->code[1]); //printing the updated dependents number
					}
				}
			}
		}
		pthread_mutex_unlock(&mem_lock);
		//enable cancelation
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

	} while(NTE->code[1] > 0);
	

	int operation = NTE->code[4];
	//printf("OPERATION: %d\n",operation);	
	switch(operation)
	{
		//case code_input: 		printf("\t<< "); scanf("%d",NTE->code[2]); break;
		case code_plus: 		NTE->code[2] = NTE->code[6]+ NTE->code[7]; break;
		case code_minus:		NTE->code[2] = NTE->code[6] - NTE->code[7]; break;
		case code_times:		NTE->code[2] = NTE->code[6] * NTE->code[7]; break;
		case code_is_equal:		(NTE->code[6] == NTE->code[7]) ? (NTE->code[2]=1): (NTE->code[2]=0); break;
		case code_is_less:		(NTE->code[6] < NTE->code[7]) ? (NTE->code[2]=1): (NTE->code[2]=0); break;
		case code_is_greater:	(NTE->code[6] > NTE->code[7]) ? (NTE->code[2]=1): (NTE->code[2]=0); break;
		case code_if:			if((NTE->code[6] != 0))
								{ 
									(NTE->code[2] = NTE->code[7]);
								}
								else
								{ 
									
									//propagate_death(NTE->code[0]); 
									NTE->code[1] = DEAD; 
								} break; 
		case code_else:			if(NTE->code[6] == 0)
								{
									(NTE->code[2] = NTE->code[7]);
								}
								else
								{ 
									
									//propagate_death(NTE->code[0]);
									NTE->code[1] = DEAD; 
								} break;

		//TODO: Fix merge so it has a single argument
		case code_merge:		NTE->code[2] = (NTE->code[6] | NTE->code[7]); break;
		case code_identity:		break; //do nothing since its an identity
		default: 
		{ 
			printf("Error: CPU %d has unknown code found during interpretation\n	Operation: %d\n",cpu->cpu_num,operation);
			sleep(10);
		}
	}
		
	struct Destination *dest = NTE->dest;
	for(int i = 1; i<=NTE->num_dest;i++){

		struct Message_capsul *output;
		output = (struct Message_capsul *)malloc(sizeof(struct Message_capsul));
		output->value = NTE->code[2];
		output->dest = dest->cpu_dest;//cpu number, it is either a cpu number or -99 for writing back to memory
		output->addr = NTE->code[NTE->node_size-i];//stack destination address, it is either a positive offset or -1 for "writing back to memory" state
		output->node_num = dest->node_dest;
		output->message_type = RESULT;

		//dissable cancel since it will be using mutexes and we dont wanna cancel while a mutex is locked
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		
		//not has dependent and should write its value in the queue
		if(dest->cpu_dest == -99){
			printf("CPU %d Writing to Main MEM!!\n",cpu->cpu_num);
			//writing back to memory (code array)
			pthread_mutex_lock(&mem_lock);
			writeMem(NTE->code_address+2, output->value);
			pthread_mutex_unlock(&mem_lock);
		
		}else if(dest->cpu_dest == 0){
			//save the value and node name (node number) for when its called upon
			int stored = 0;
			for(int i = 0; i<LOCAL_STORAGE_SIZE; i++){
				if(cpu->local_mem[0][i] == UNDEFINED){
					printf("CPU %d STORING MEM!!\n",cpu->cpu_num);
					cpu->local_mem[0][i] = NTE->node_num;
					cpu->local_mem[1][i] = output->addr;
					cpu->local_mem[2][i] = output->value;
					cpu->local_mem[3][i] = output->node_num;  //add entry if its been used so we know theres a problem if it was evicted before use 
					cpu->local_mem[4][i] = 0; //unused
					stored = 1;
					break;
				}
			}
			if(stored == 0)
				printf("CPU %d STORING UNSUCCSESFULL\n",cpu->cpu_num);
		}else{ // we need to calculate stuff here after everything has poped up in the queue
			printf("CPU %d sending result to CPU %d\n",cpu->cpu_num,output->dest);
			pthread_mutex_lock(&mem_lock);
			enQueue(cpu->look_up[output->dest-1], output);
			pthread_mutex_unlock(&mem_lock);
		}
		
		//enable cancelation
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

		dest = dest->next;
	}

		//request a new task
		printf("CPU %d has requested a new task\n",cpu->cpu_num);
		cpu->node_to_execute = schedule_me(cpu->cpu_num);
	}
	
	
}


// function to create a new linked list node.
struct Message_capsul* newNode(struct Message_capsul *out)
{
    struct Message_capsul* temp = (struct Message_capsul*)malloc(sizeof(struct Message_capsul));
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
void enQueue(struct Queue* q, struct Message_capsul *out)
{
    // Create a new LL node
    struct Message_capsul* temp = newNode(out);
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
struct Message_capsul* deQueue(struct Queue* q)
{
    struct Message_capsul* result = (struct Message_capsul*)malloc(sizeof(struct Message_capsul));
    // If queue is empty, return NULL.
    if (q->front == NULL)
        return NULL;
  
    // Store previous front and move front one node ahead
    struct Message_capsul* temp = q->front;
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


  

















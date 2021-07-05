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
	
	int var_access_key = UNDEFINED; 
	

	
	START:
	while(1){	
	
	struct AGP_node *NTE = cpu->node_to_execute;

	printf("CPU %d DOING NODE %d\n",cpu->cpu_num,NTE->node_num);

	//check if there are requests to make, and if yes make em
	struct Dependables *dep = NTE->depend;
	while(dep != NULL){ //should be null if no requests to make 
		
		if(dep->cpu_num==0){
			goto NEXT;
		}else if(dep->cpu_num == cpu->cpu_num){ //fetch from your own cache!!
			printf("CPU %d fetching variable %d from local mem\n",cpu->cpu_num,dep->node_needed);
			for(int i = 0; i<8; i++){
				if(cpu->local_mem[0][i] == dep->node_needed){
					if(cpu->local_mem[3][i] == NTE->node_num || cpu->local_mem[3][i] == dep->key){
						if(NTE->code[4] == code_input){
							NTE->code[2] = cpu->local_mem[2][i];
							NTE->code[1]--;
							NTE->code[4] = code_identity;
							goto NEXT;
						}else{
							int addr = NTE->node_size - (cpu->local_mem[1][i]/4) - 1;
							NTE->code[addr] =  cpu->local_mem[2][i];
							cpu->local_mem[0][i] = UNDEFINED;
							NTE->code[1]--; //7. decrese the number of dependent
							goto NEXT;
						}
					}
				}
			}
		}else{
			printf("CPU %d sending variable request to CPU %d",cpu->cpu_num, dep->cpu_num); 
			printf(" FOR NODE %d\n",dep->node_needed); 
			struct Message_capsul *request = (struct Message_capsul *)malloc(sizeof(struct Message_capsul));

			request->value = cpu->cpu_num; //cpu num to send reult back to (its self)
			request->dest = dep->cpu_num;  //request destination
			request->addr = dep->node_needed; //request variable name
			request->node_num = NTE->node_num;
			request->message_type = REQUEST;
			

			if(dep->key == UNDEFINED){ 
				request->node_num = NTE->node_num;
				
			}else{
				request->node_num = dep->key;
				var_access_key = dep->key;
			}
			
			
			pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
			pthread_mutex_lock(&mem_lock);
			enQueue(cpu->look_up[dep->cpu_num-1], request);
			pthread_mutex_unlock(&mem_lock);

			//enable cancelation
			pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
		}
		NEXT:
		dep = dep->next;
		
	}
	//message passing loop
	int count = 0;
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
						printf("CPU %d LOOKING FOR NODE %d FOR NODE %d REQUEST\n",cpu->cpu_num,message->addr,message->node_num);
						//printf(" - value: %d\n - addr: %d\n - dest: %d\n - node_num: %d\n",message->value,message->addr,message->dest,message->node_num);
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
						if(found==1){
							enQueue(cpu->look_up[message->value-1], output);
							printf("CPU %d seding requested var to CPU %d\n",cpu->cpu_num,output->dest);
						}else{
							enQueue(cpu->look_up[cpu->cpu_num-1], message);
							printf("CPU %d didn't find requested var from CPU %d\n",cpu->cpu_num,message->value);
						}
					}
					
				}else{
					if(message->node_num != NTE->node_num && message->node_num != var_access_key){//requesting currently being processed node
						enQueue(cpu->look_up[cpu->cpu_num-1], message);
					}else if(NTE->code[4] == code_input){
						NTE->code[2] = message->value;
						NTE->code[4] = code_identity;
						NTE->code[1]--; //7. decrese the number of dependents
						printf("CPU %d dependents: %d\n",cpu->cpu_num, NTE->code[1]); //printing the updated dependents number
					}else{
						int addr = NTE->node_size - (message->addr/4) - 1;
						printf("CPU %d writing to addr %d\n",cpu->cpu_num, message->addr);
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
		
		count++;
		if(count == 750 && NTE->code[4] == -1){
			printf("CPU %d has requested a new task\n",cpu->cpu_num);
			pthread_mutex_lock(&mem_lock);
			cpu->node_to_execute = schedule_me(cpu->cpu_num);
			pthread_mutex_unlock(&mem_lock);
			goto START;
		}

	} while(NTE->code[1] > 0);
	var_access_key = UNDEFINED;


	  /********************/
	 /** INTERPRETATION **/
	/********************/
	//expansion is different
	if(NTE->code[4] == code_expansion)
	{
		printf("\n\nCPU %d EXPASION NOT IMPLEMENTED YET\n\n",cpu->cpu_num);

		


	}
	//all other operators
	else
	{
		int operation = NTE->code[4];
		//printf("OPERATION: %d\n",operation);	
		switch(operation)
		{
			case code_input: 		printf("\t<< "); scanf("%d",NTE->code[2]); break;
			case code_output:		printf("CPU %d printing: %d\n",cpu->cpu_num,NTE->code[2]); break;
			case code_plus: 		NTE->code[2] = NTE->code[6]+ NTE->code[7]; break;
			case code_minus:		NTE->code[2] = NTE->code[6] - NTE->code[7]; break;
			case code_times:		NTE->code[2] = NTE->code[6] * NTE->code[7]; break;
			case code_is_equal:		(NTE->code[6] == NTE->code[7]) ? (NTE->code[2]=1): (NTE->code[2]=0); break;
			case code_is_less:		(NTE->code[6] < NTE->code[7]) ? (NTE->code[2]=1): (NTE->code[2]=0); break;
			case code_is_greater:	(NTE->code[6] > NTE->code[7]) ? (NTE->code[2]=1): (NTE->code[2]=0); break;
			case code_if:			if((NTE->code[6] != 0))
									{ 
										(NTE->code[2] = NTE->code[7]);
										//pthread_mutex_lock(&mem_lock);
										mark_as_dead(NTE->node_num);
										//pthread_mutex_unlock(&mem_lock);
									}
									else
									{ 
										//pthread_mutex_lock(&mem_lock);
										propagate_death(NTE->node_num); 
										//pthread_mutex_unlock(&mem_lock);
									} break; 
			case code_else:			if(NTE->code[6] == 0)
									{
										(NTE->code[2] = NTE->code[7]);
										//pthread_mutex_lock(&mem_lock);
										mark_as_dead(NTE->node_num);
										//pthread_mutex_unlock(&mem_lock);
									}
									else
									{ 
										//pthread_mutex_lock(&mem_lock);
										propagate_death(NTE->node_num);
										//pthread_mutex_unlock(&mem_lock);
									} break;

			//TODO: Fix merge so it has a single argument
			case code_merge:		
					NTE->code[2] = (NTE->code[6] | NTE->code[7]); 
					printf("CPU %d MERGE %d & %d = %d\n",cpu->cpu_num,NTE->code[6],NTE->code[7],NTE->code[2]);
					break;
			case code_identity:		if(NTE->code[2] == NAV){NTE->code[2] = NTE->code[6];}break; 
			default: 
			{ 
				printf("Error: CPU %d has unknown code found during interpretation\n	Operation: %d\n",cpu->cpu_num,operation);
				sleep(10);
			}
		}
	}

		  /*******************/
		 /***** OUTPUT ******/
		/*******************/
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
				writeMem(NTE->code_address+2, output->value);
				
			
			}else if(dest->cpu_dest == 0){
				//save the value and node name (node number) for when its called upon
				int stored = 0;
				for(int i = 0; i<LOCAL_STORAGE_SIZE; i++){
					if(cpu->local_mem[0][i] == UNDEFINED){
						printf("CPU %d STORING NODE %d FOR NODE %d TO MEM!!\n",cpu->cpu_num,NTE->node_num,output->node_num);
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
		
		int prev = NTE->node_num;
		pthread_mutex_lock(&mem_lock);
		cpu->node_to_execute = schedule_me(cpu->cpu_num);
		pthread_mutex_unlock(&mem_lock);
		while(cpu->node_to_execute->node_num == prev){
			pthread_mutex_lock(&mem_lock);
			cpu->node_to_execute = schedule_me(cpu->cpu_num);
			pthread_mutex_unlock(&mem_lock);
		}
		
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


  

















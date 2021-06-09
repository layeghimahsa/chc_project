#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <pthread.h>
#include <string.h>
#include "cpu.h"
#include "2by2sim.h"


void *CPU_start(struct cpu *CPU){
	
	printf("CPU %d 	START!!\n",CPU->assigned_cpu);
	
	
	
	/* ************ returning cpu_output value ********************** */
	
	int count = 0;
	
	while(CPU->code[1] > 0){ //while waiting for dependent
		sleep(0.01);
		// -check port for dependent! (checking the queue) , reset the count and reducce the number of dependents 
		//setting up the queue and periodically checking them
		
		switch(CPU->assigned_cpu){
		
			struct cpu_out *temp;
			
			case 1: // 1. cpu1 is checking their queue
				pthread_mutex_lock(&mem_lock); // locking the mutex for the purpose of reading and writing to the queues
					//check your queue
					if(cpu_queue1->size > 0){ //2. if there is something in queue1,
						temp = deQueue(cpu_queue1); // 3. dequeue it
						printf("CPU %d dequeded element: %d\n",CPU->assigned_cpu, temp->value); //print it 
						//, 3. is its destination for cpu1
						if(temp->dest == 2){  //no its for 2
						 	enQueue(cpu_queue2, temp); // 5. enqueue it to cpu2 queue
							count = 0; //6.reset the count
						}else if(temp->dest == 3){ //no its for 3
							enQueue(cpu_queue3, temp); // 5. enqueue it to cpu3 queue
							count = 0; //6.reset the count
						}else if(temp->dest == 4){ //not its for 4
							enQueue(cpu_queue2, temp); // 5. enqueue it to cpu2 queue since not connected to 4
							count = 0; //6.reset the count
						}else{
							int addr = CPU->node_size - (temp->addr/4) - 1;
							CPU->code[addr] = temp->value;
							count = 0; //6.reset the count
							CPU->code[1]--; //7. decrese the number of dependents
							printf("CPU %d dependents: %d\n",CPU->assigned_cpu, CPU->code[1]); //printing the updated dependents number
						}
					}
				pthread_mutex_unlock(&mem_lock); // unlocking the mutex!
				break;
				
			case 2:
				pthread_mutex_lock(&mem_lock); // locking the mutex for the purpose of reading and writing to the queues
					//check your queue
					if(cpu_queue2->size > 0){ //2. if there is something in queue1,
						temp = deQueue(cpu_queue2); // 3. dequeue it
						printf("CPU %d dequeded element: %d\n",CPU->assigned_cpu, temp->value); //print it 
						//, 3. is its destination for cpu1
						if(temp->dest == 1){  //no its for 2
						 	enQueue(cpu_queue1, temp); // 5. enqueue it to cpu2 queue
							count = 0; //6.reset the count
						}else if(temp->dest == 4){ //no its for 3
							enQueue(cpu_queue4, temp); // 5. enqueue it to cpu3 queue
							count = 0; //6.reset the count
						}else if(temp->dest == 3){ //not its for 4
							enQueue(cpu_queue4, temp); // 5. enqueue it to cpu2 queue since not connected to 4
							count = 0; //6.reset the count
						}else{
							int addr = CPU->node_size - (temp->addr/4) - 1;
							CPU->code[addr] = temp->value;
							count = 0; //6.reset the count
							CPU->code[1]--; //7. decrese the number of dependents
							printf("CPU %d dependents: %d\n",CPU->assigned_cpu, CPU->code[1]); //printing the updated dependents number
						}
					}
				pthread_mutex_unlock(&mem_lock); // unlocking the mutex!
				break;
			case 3:
				pthread_mutex_lock(&mem_lock); // locking the mutex for the purpose of reading and writing to the queues
					//check your queue
					if(cpu_queue3->size > 0){ //2. if there is something in queue1,
						temp = deQueue(cpu_queue3); // 3. dequeue it
						printf("CPU %d dequeded element: %d\n",CPU->assigned_cpu, temp->value); //print it 
						//, 3. is its destination for cpu1
						if(temp->dest == 1){  //no its for 2
						 	enQueue(cpu_queue1, temp); // 5. enqueue it to cpu2 queue
							count = 0; //6.reset the count
						}else if(temp->dest == 4){ //no its for 3
							enQueue(cpu_queue4, temp); // 5. enqueue it to cpu3 queue
							count = 0; //6.reset the count
						}else if(temp->dest == 2){ //not its for 4
							enQueue(cpu_queue1, temp); // 5. enqueue it to cpu2 queue since not connected to 4
							count = 0; //6.reset the count
						}else{
							int addr = CPU->node_size - (temp->addr/4) - 1;
							CPU->code[addr] = temp->value;
							count = 0; //6.reset the count
							CPU->code[1]--; //7. decrese the number of dependents
							printf("CPU %d dependents: %d\n",CPU->assigned_cpu, CPU->code[1]); //printing the updated dependents number
						}
					}
				pthread_mutex_unlock(&mem_lock); // unlocking the mutex!
				
				break;
			case 4:
			
				pthread_mutex_lock(&mem_lock); // locking the mutex for the purpose of reading and writing to the queues
					//check your queue
					if(cpu_queue4->size > 0){ //2. if there is something in queue1,
						temp = deQueue(cpu_queue4); // 3. dequeue it
						printf("CPU %d dequeded element: %d\n",CPU->assigned_cpu, temp->value); //print it 
						//, 3. is its destination for cpu1
						if(temp->dest == 2){  //no its for 2
						 	enQueue(cpu_queue2, temp); // 5. enqueue it to cpu2 queue
							count = 0; //6.reset the count
						}else if(temp->dest == 3){ //no its for 3
							enQueue(cpu_queue3, temp); // 5. enqueue it to cpu3 queue
							count = 0; //6.reset the count
						}else if(temp->dest == 1){ //not its for 4
							enQueue(cpu_queue3, temp); // 5. enqueue it to cpu2 queue since not connected to 4
							count = 0; //6.reset the count
						}else{
							int addr = CPU->node_size - (temp->addr/4) - 1;
							CPU->code[addr] = temp->value;
							count = 0; //6.reset the count
							CPU->code[1]--; //7. decrese the number of dependents
							printf("CPU %d dependents: %d\n",CPU->assigned_cpu, CPU->code[1]); //printing the updated dependents number
						}
					}
				pthread_mutex_unlock(&mem_lock); // unlocking the mutex!
				
				break;
				
			default:
				puts("SHOULD NOT HAPPEN!\n");
				break;
				
		
		}
		
		
		if(count > 500){
			printf("CPU %d TIMER TIMEOUT!\n", CPU->assigned_cpu );
			return 0; //might be changed
		}
			
		count++;
	}
	

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
		default: printf("Error: unknown code found during interpretation\n");
	}
		
	struct cpu_out *output;
	output = (struct cpu_out *)malloc(sizeof(struct cpu_out));
	
	printf("CPU %d RESULT: %d\n", CPU->assigned_cpu, CPU->code[2]);
	output->value = CPU->code[2];
	
	
	/* ************ returning cpu_output destination and address ********************** */
	output->dest = CPU->cpu_dest;//cpu number, it is either a cpu number or -99 for writing back to memory
	
	/* ************ address translation  ********************** */

	output->addr = CPU->code[CPU->node_size-1];//stack destination address, it is either a positive offset or -1 for "writing back to memory" state



	//if statement that fills in the queue or writing to the memory!
	
	//not has dependent and should write its value in the queue
	if(CPU->dest_node != -99){
		
		switch(CPU->assigned_cpu){
		
			case 1:
				pthread_mutex_lock(&mem_lock);
					if(output->dest == 2)
						enQueue(cpu_queue2, output);
					else if(output->dest == 3)
						enQueue(cpu_queue3, output);
					else
						enQueue(cpu_queue2, output);
					
				printf("CPU %d sent to CPU queue %d\n",CPU->assigned_cpu ,output->dest);
				pthread_mutex_unlock(&mem_lock);
				break;
			case 2:
				pthread_mutex_lock(&mem_lock);
					if(output->dest == 1)
						enQueue(cpu_queue1, output);
					else if(output->dest == 4)
						enQueue(cpu_queue4, output);
					else
						enQueue(cpu_queue4, output);
					
				printf("CPU %d sent to CPU queue %d\n",CPU->assigned_cpu ,output->dest);
				pthread_mutex_unlock(&mem_lock);
				break;
			case 3:
				pthread_mutex_lock(&mem_lock);
					if(output->dest == 1)
						enQueue(cpu_queue1, output);
					else if(output->dest == 4)
						enQueue(cpu_queue4, output);
					else
						enQueue(cpu_queue1, output);
					
				printf("CPU %d sent to CPU queue %d\n",CPU->assigned_cpu ,output->dest);
				pthread_mutex_unlock(&mem_lock);
				break;
			case 4:
				pthread_mutex_lock(&mem_lock);
					if(output->dest == 2)
						enQueue(cpu_queue2, output);
					else if(output->dest == 3)
						enQueue(cpu_queue3, output);
					else
						enQueue(cpu_queue3, output);
					
				printf("CPU %d sent to CPU queue %d\n",CPU->assigned_cpu ,output->dest);
				pthread_mutex_unlock(&mem_lock);
				break;
			default:
				puts("SHOULD NEVER HAPPEN! \n");
				break; 
		
		}		
	}else{ // we need to calculate stuff here after everything has poped up in the queue
	
		printf("CPU %d Writing to Main MEM!!\n",CPU->assigned_cpu);
		//updating CPU code (local memory)
		CPU->code[2] = output->value;
		
		//writing back to memory (code array)
		pthread_mutex_lock(&mem_lock);
		writeMem(CPU->code_address+2, output->value);
		pthread_mutex_unlock(&mem_lock);
	}

	//printf("\n\nTESTING OUTPUT RESULT\n\n"); 
	printf("\t CPU %d 's VALUE: %d\n", CPU->assigned_cpu, output->value);
	printf("\t CPU %d 's DEST: %d\n", CPU->assigned_cpu, output->dest);
	printf("\t CPU %d 's ADDR: %d\n", CPU->assigned_cpu, output->addr);
		
	
	return output;
	
	
}


// function to create a new linked list node.
struct cpu_out* newNode(struct cpu_out *out)
{
    struct cpu_out* temp = (struct cpu_out*)malloc(sizeof(struct cpu_out));
    temp->value = out->value;
    temp->dest = out->dest;
    temp->addr = out->addr;
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
  
    q->front = q->front->next;
    q->size -= 1;
  
    // If front becomes NULL, then change rear also as NULL
    if (q->front == NULL)
        q->rear = NULL;
  
    free(temp);
    return result;
}


  

















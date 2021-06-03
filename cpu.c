#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <pthread.h>
#include <string.h>
#include "cpu.h"
#include "2by2sim.h"


void *CPU_start(struct cpu *CPU){
	
	printf("CPU %d 	START!!\n",CPU->assigned_cpu);
	struct cpu_out *output;
	output = (struct cpu_out *)malloc(sizeof(struct cpu_out));
	
	
	/* ************ returning cpu_output value ********************** */
	
	int count = 0;
	
	while(CPU->code[1] > 0){ //while waiting for dependent
		sleep(0.01);
		// -check port for dependent! (checking the queue) , reset the count and reducce the number of dependents 
		
		switch(CPU->assigned_cpu){
		
			struct cpu_out *temp;
			
			case 1: // 1. cpu1 is checking the queues
				pthread_mutex_lock(&mem_lock); // locking the mutex for the purpose of reading and writing to the queues
				
					//direct connections
					if(cpu_queue2->size > 0){ //2. if there is something in queue2, 
						if(cpu_queue2->front->dest == 1){ //, 3. and its destination is cpu1
							temp = deQueue(cpu_queue2); // 4. dequeue it 
							printf("dequeued element from cpu_queue2: %d\n", temp->value); //print it
							enQueue(cpu_queue1, temp); // 5. enqueue it to cpu1 queue
							count = 0; //6.reset the count
							CPU->code[1]--; //7. decrese the number of dependents
							printf("CPU %d dependents: %d\n",CPU->assigned_cpu, CPU->code[1]); //printing the updated dependents number
						}
						
					} else if(cpu_queue3->size > 0){ //same steps for the next connected cpu
					
						if(cpu_queue3->front->dest == 1){
							temp = deQueue(cpu_queue3);
							printf("dequeued element from cpu_queue3: %d\n", temp->value);
							enQueue(cpu_queue1, temp);
							count = 0;
							CPU->code[1]--;
							printf("CPU %d dependents: %d\n",CPU->assigned_cpu, CPU->code[1]);
						}
					}
					
					//indirect connections
					else if(cpu_queue4->size > 0){ //this is for indirect connections where one cpu needs to pass the result to an intermediate cpu
					
						if(cpu_queue4->front->dest == 1){
							temp = deQueue(cpu_queue4);
							printf("dequeued element from cpu_queue4: %d\n", temp->value);
							enQueue(cpu_queue3, temp); //dequeue it to an intermediate cpu, but we are not decreasing the number of dependents yet!
							count = 0;
						}
					}
				
				pthread_mutex_unlock(&mem_lock); // unlocking the mutex!
				
				break;
				
			case 2:
				pthread_mutex_lock(&mem_lock);
				
					//direct connections
					if(cpu_queue1->size > 0){
						if(cpu_queue1->front->dest == 2){
							temp = deQueue(cpu_queue1);
							printf("dequeued element from cpu_queue1: %d\n", temp->value);
							enQueue(cpu_queue2, temp);
							count = 0;
							CPU->code[1]--;
							printf("CPU %d dependents: %d\n",CPU->assigned_cpu, CPU->code[1]);
						}
					} else if(cpu_queue4->size > 0){
					
						if(cpu_queue4->front->dest == 2){
							temp = deQueue(cpu_queue4);
							printf("dequeued element from cpu_queue4: %d\n", temp->value);
							enQueue(cpu_queue2, temp);
							count = 0;
							CPU->code[1]--;
							printf("CPU %d dependents: %d\n",CPU->assigned_cpu, CPU->code[1]);
						}
					}
					
					//indirect connections
					else if(cpu_queue3->size > 0){ 
					
						if(cpu_queue3->front->dest == 2){
							temp = deQueue(cpu_queue3);
							printf("dequeued element from cpu_queue3: %d\n", temp->value);
							enQueue(cpu_queue1, temp); 
							count = 0;
						}
					}
				
				pthread_mutex_unlock(&mem_lock);
				
				break;
			case 3:
				pthread_mutex_lock(&mem_lock);
			
					//direct connections
					if(cpu_queue1->size > 0){
						if(cpu_queue1->front->dest == 3){
							temp = deQueue(cpu_queue1);
							printf("dequeued element from cpu_queue1: %d\n", temp->value);
							enQueue(cpu_queue3, temp);
							count = 0;
							CPU->code[1]--;
							printf("CPU %d dependents: %d\n",CPU->assigned_cpu, CPU->code[1]);
						}
					} else if(cpu_queue4->size > 0){
					
						if(cpu_queue4->front->dest == 3){
							temp = deQueue(cpu_queue4);
							printf("dequeued element from cpu_queue4: %d\n", temp->value);
							enQueue(cpu_queue3, temp);
							count = 0;
							CPU->code[1]--;
							printf("CPU %d dependents: %d\n",CPU->assigned_cpu, CPU->code[1]);
						}
					}
					
					
					//indirect connections
					else if(cpu_queue2->size > 0){
					
						if(cpu_queue2->front->dest == 3){
							temp = deQueue(cpu_queue2);
							printf("dequeued element from cpu_queue2: %d\n", temp->value);
							enQueue(cpu_queue4, temp);
							count = 0;
						}
					}
					
				
				pthread_mutex_unlock(&mem_lock);
				break;
			case 4:
			
				pthread_mutex_lock(&mem_lock);
				
					//direct connections
					if(cpu_queue2->size > 0){
						if(cpu_queue2->front->dest == 4){
							temp = deQueue(cpu_queue2);
							printf("dequeued element from cpu_queue2: %d\n", temp->value);
							enQueue(cpu_queue4, temp);
							count = 0;
							CPU->code[1]--;
							printf("CPU %d dependents: %d\n",CPU->assigned_cpu, CPU->code[1]);
						}
					} else if(cpu_queue3->size > 0){
					
						if(cpu_queue3->front->dest == 4){
							temp = deQueue(cpu_queue3);
							printf("dequeued element from cpu_queue3: %d\n", temp->value);
							enQueue(cpu_queue4, temp);
							count = 0;
							CPU->code[1]--;
							printf("CPU %d dependents: %d\n",CPU->assigned_cpu, CPU->code[1]);
						}
					}
					
					//indirect connections
					else if(cpu_queue1->size > 0){
					
						if(cpu_queue1->front->dest == 4){
							temp = deQueue(cpu_queue1);
							printf("dequeued element from cpu_queue1: %d\n", temp->value);
							enQueue(cpu_queue3, temp);
							count = 0;
						}
					}
				
				pthread_mutex_unlock(&mem_lock);
				
				break;
				
			default:
				puts("SHOULD NOT HAPPEN!\n");
				break;
				
		
		}
		
		/*if(cpu_queue->size > 0){
			//int val = deQueue(cpu_queue);
			count = 0;
			CPU->code[1]--;
		}*/
		
		if(count > 500){
			printf("CPU %d TIMER TIMEOUT!\n", CPU->assigned_cpu );
			return 0; //might be changed
		}
			
		count++;
	}
	
	
	/*if(CPU->dest_node != -99){ // if the node has no dependent, its value is ready and in hand. (I checked it by dest_node because eventually, number of dependents are reaching to zero),
					// , so it is not a good indicator
		output->value = CPU->code[2]; //if there is no dependendent, just assign the value.	
	}*/
	
	
	/* ************ returning cpu_output destination and address ********************** */
	output->dest = CPU->cpu_dest;//cpu number, it is either a cpu number or -99 for writing back to memory
	
	/* ************ address translation  ********************** */

	output->addr = CPU->code[CPU->node_size-1];//stack destination address, it is either a positive offset or -1 for "writing back to memory" state
	
	
	
	//if statement that fills in the queue or writing to the memory!
	
	//not has dependent and should write its value in the queue
	if(CPU->code[1] == 0 && CPU->dest_node != -99){
		//output->cpu_queue = createQueue();
		//enQueue(output->cpu_queue, CPU->code[2]);
		output->value = CPU->code[2];
		
		switch(CPU->assigned_cpu){
		
			case 1:
				pthread_mutex_lock(&mem_lock);
				
					enQueue(cpu_queue1, output);
					printf("CPU %d queue size: %d\n",CPU->assigned_cpu ,cpu_queue1->size);
				
				pthread_mutex_unlock(&mem_lock);
				
				break;
				
			case 2:
				pthread_mutex_lock(&mem_lock);
				
					enQueue(cpu_queue2, output);
					printf("CPU %d queue size: %d\n",CPU->assigned_cpu ,cpu_queue1->size);
					
				pthread_mutex_unlock(&mem_lock);
				
				break;
			
			case 3:
				pthread_mutex_lock(&mem_lock);
				
					enQueue(cpu_queue3, output);
					printf("CPU %d queue size: %d\n",CPU->assigned_cpu ,cpu_queue1->size);
				
				pthread_mutex_unlock(&mem_lock);
				
				break;
			
			case 4:
				pthread_mutex_lock(&mem_lock);
				
					enQueue(cpu_queue4, output);
					printf("CPU %d queue size: %d\n",CPU->assigned_cpu ,cpu_queue1->size);
				
				pthread_mutex_unlock(&mem_lock);
				
				break;
				
			default:
				puts("SHOULD NEVER HAPPEN! \n");
				break; 
		
		}		
	} else if(CPU->code[1] == 0 && CPU->dest_node == -99){ // we need to calculate stuff here after everything has poped up in the queue
	
		int operation = CPU->code[4];
		
		//dequeuing from the queue
		switch(CPU->assigned_cpu){
			struct cpu_out *temp;
			int array_index;
			
			case 1:
				pthread_mutex_lock(&mem_lock);
				
				while(cpu_queue1->size > 0){	
					temp = deQueue(cpu_queue1);
					array_index = CPU->node_size - (temp->addr/4) - 1;
					CPU->code[array_index] = temp->value;
					printf("code[%d] is : %d\n",array_index ,temp->value);
				}
					
				pthread_mutex_unlock(&mem_lock);
				
				break;
			case 2:
				pthread_mutex_lock(&mem_lock);
				
				while(cpu_queue2->size > 0){	
					temp = deQueue(cpu_queue2);
					array_index = CPU->node_size - (temp->addr/4) - 1;
					CPU->code[array_index] = temp->value;
					printf("code[%d] is : %d\n",array_index ,temp->value);
				}
					
				pthread_mutex_unlock(&mem_lock);
				break;
			case 3:
				pthread_mutex_lock(&mem_lock);
				
				while(cpu_queue3->size > 0){	
					temp = deQueue(cpu_queue3);
					array_index = CPU->node_size - (temp->addr/4) - 1;
					CPU->code[array_index] = temp->value;
					printf("code[%d] is : %d\n",array_index ,temp->value);
				}
					
				pthread_mutex_unlock(&mem_lock);
				break;
			case 4:
				pthread_mutex_lock(&mem_lock);
				
				while(cpu_queue4->size > 0){	
					temp = deQueue(cpu_queue4);
					array_index = CPU->node_size - (temp->addr/4) - 1;
					CPU->code[array_index] = temp->value;
					printf("code[%d] is : %d\n",array_index ,temp->value);
				}
					
				pthread_mutex_unlock(&mem_lock);
				break;
				
			default:
				puts("SHOULD NEVER HAPPEN! \n");
				break;
		
		}
		
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
			//case code_identity:		CPU->code[2] = CPU->code[6]; break;
			default: printf("Error: unknown code found during interpretation\n");
		}
		
		printf("RESULT: %d\n", CPU->code[2]);
		output->value = CPU->code[2];
	}
	
	
	//setting up the queue and periodically checking them
	
	
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


  

















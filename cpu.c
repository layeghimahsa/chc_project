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
	struct Queue* cpu_queue = createQueue();
	
	
	/* ************ returning cpu_output value ********************** */
	
	int count = 0;
	
	while(CPU->code[1] > 0){ //while waiting for dependent
		sleep(0.01);
		// -check port for dependent! (checking the queue) , reset the count and reducce the number of dependents 
		if(cpu_queue->size > 0){
			//int val = deQueue(cpu_queue);
			count = 0;
			CPU->code[1]--;
		}
		if(count > 500){
			printf("CPU %d TIMER TIMEOUT!\n", CPU->assigned_cpu );
			return 0; //might be changed
		}
			
		count++;
	}
	
	
		int operation = CPU->code[4];
		
		
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
		
		output->value = CPU->code[2]; //if there is no dependendent, just assign the value.
		
	
	
	
	/* ************ returning cpu_output destination and address ********************** */
	output->dest = CPU->cpu_dest;//cpu number, it is either a cpu number or -99 for writing back to memory
	
	/* ************ address translation  ********************** */

	output->addr = CPU->code[CPU->node_size-1];//stack destination address, it is either a positive offset or -1 for "writing back to memory" state
	
	//if statement that fills in the queue or writing to the memory!
	
	//not has dependent and should write its value in the queue
	if(CPU->code[1] == 0 && CPU->dest_node != -99){
		//output->cpu_queue = createQueue();
		//enQueue(output->cpu_queue, CPU->code[2]);
	} else if(CPU->code[1] == 0 && CPU->dest_node == -99){
		printf("VALUE: %d\n", output->value);
	}
	
	
	//setting up the queue and periodically checking them
	
	
	//printf("\n\nTESTING OUTPUT RESULT\n\n"); 
	printf("\t CPU %d 's VALUE: %d\n", CPU->assigned_cpu, output->value);
	printf("\t CPU %d 's DEST: %d\n", CPU->assigned_cpu, output->dest);
	printf("\t CPU %d 's ADDR: %d\n", CPU->assigned_cpu, output->addr);
		
	
	return output;
	
	
}


// function to create a new linked list node.
struct QNode* newNode(int val)
{
    struct QNode* temp = (struct QNode*)malloc(sizeof(struct QNode));
    temp->value = val;
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
void enQueue(struct Queue* q, int val)
{
    // Create a new LL node
    struct QNode* temp = newNode(val);
  
    // If queue is empty, then new node is front and rear both
    if (q->rear == NULL) {
        q->front = q->rear = temp;
        q->size += 1;
        return;
    }
  
    // Add the new node at the end of queue and change rear
    q->rear->next = temp;
    q->rear = temp;
}

// Function to remove a value from queue
int deQueue(struct Queue* q)
{
    int value;
    // If queue is empty, return NULL.
    if (q->front == NULL)
        return -1;
  
    // Store previous front and move front one node ahead
    struct QNode* temp = q->front;
    value = temp->value;
  
    q->front = q->front->next;
    q->size -= 1;
  
    // If front becomes NULL, then change rear also as NULL
    if (q->front == NULL)
        q->rear = NULL;
  
    free(temp);
    return value;
}


  

















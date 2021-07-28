#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include "cpu.h"
#include "2by2sim.h"



void *CPU_start(struct CPU *cpu){

	int cpu_num = cpu->cpu_num;

	if(MESSAGE == 1)
		printf("CPU %d 	START!!\n",cpu_num);
	for(int i = 0; i<LS_SIZE; i++){
		cpu->local_mem[0][i] = UNDEFINED;
	}
	int var_access_key = UNDEFINED;

	struct AGP_node *NTE;
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	while(1){
		sleep(0.5);
	//NTE = cpu->node_to_execute;

	if(MESSAGE == 1)
		printf("CPU %d DOING NODE %d\n	DEP %d\n",cpu_num,NTE->node_num,NTE->code[1]);

	}

}

struct Message*  Message_packing(int cpu_num, int rw, int addr, int data ){

			struct Messasge* temp = (struct Message*)malloc(sizeof(struct Message));

			unsigned int address = ((cpu_num & 0x0000003F) << 26)
											 | ((rw & 0x00000001) << 25)
											 | (addr & 0x0001FFFF);

			temp->addr = address;
			temp->data = data;
			temp->next = NULL;

			return temp;

}


void Message_printing(str Message *message){

			/* address 32 bits
			[cpu number][R/W][addr]
			[6b][1b][25b]
			*/

			unsigned int addr_temp;
			int cpu_num, rw, addr;

			printf("Message's Address: %u\n", message->addr);
			printf("Message's Data: %d\n", message->data);

			addr_temp = message->addr;
			addr = (int) addr_temp & 0x0001FFFF; //fetching addr
			printf("addr temp: %d", addr_temp);
			rw = (int) ( addr_temp >> 25 ) & 0x00000001; //fetching read or write
			printf("addr temp: %d", addr_temp);
			cpu_num = (int) ( addr_temp >> 26 ) & 0x0000003F; //fetc cpu number


			printf("- cpu number: %d , binary representation: ", cpu_num);
			bin_representation(cpu_num);
			puts("\n");

			(rw == 1) ? printf("- write\n") : printf("- read\n");

			printf("- address: %d , binary representation: ", addr);
			bin_representation(addr);
			puts("\n");

			return;

}


void bin_representation(int n)
{
    if (n > 1)
        bin_representation(n / 2);

    printf("%d", n % 2);
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

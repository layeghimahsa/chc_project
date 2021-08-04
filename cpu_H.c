#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include "cpu.h"
#include "2by2sim.h"
#include "many_core.h"

pthread_mutex_t cpu_lock;

void *CPU_H_start(struct CPU_H *cpu){

	if (pthread_mutex_init(&cpu_lock, NULL) != 0){
			printf("\n mutex init failed\n");
			exit(0);
	}

	int cpu_num = cpu->cpu_num;

	if(MESSAGE == 1)
		printf("CPU %d 	START!!\n",cpu_num);


	for(int i = 0; i<LS_SIZE; i++){
		cpu->local_mem[0][i] = UNDEFINED;
	}

	for(int i = 0; i<ADDRASABLE_SPACE; i++){
		cpu->stack[i] = UNDEFINED;
	}

	//struct AGP_node *NTE = cpu->node_to_execute;

	//TODO: create RAM

	/*
	---code---

	node_num 				-- 0
	num_dependence	-- 1
	value
	node_size
	operation
	num_args
	arg1
	arg...
	num_dest
	dest1 (cpu_dest or save)
	dest1 node num dest
	dest1 addr
	dest... cpu
	dest... node num dest
	dest... addr

	*/
	cpu->bp = 0;
	cpu->pc = 0;
	cpu->sp = 0;

	while(1){
		sleep(0.01);
		pthread_mutex_lock(&cpu_lock);
		switch(cpu->pc){
			//request task
			case RT:
				pthread_mutex_lock(&mem_lock);
				sendMessage(buss_Min,Message_packing(cpu_num,1,OPR,REQ_TASK));
				pthread_mutex_unlock(&mem_lock);
				cpu->pc=IDLE;
				break;
			//decode operation
			case DEC:
				cpu->pc = cpu->stack[cpu->sp+4];
				break;
			//NO operation
			case IDLE:
				break;
			//for number of destinations
			//send or save result
			case FND:
				if(cpu->stack[6+cpu->stack[5]] == 0){
					if(cpu->sp == 0){cpu->pc=RT;}
					else{
						cpu->pc = cpu->stack[cpu->sp-1];
						cpu->sp = cpu->stack[cpu->sp-2];
					}
				}else{
					int todo = cpu->stack[6+cpu->stack[5]]*3;
					//todo = (code[5+code[5]] * 2)-1;
					if(cpu->stack[todo-2] == SAVE_VAL){cpu->pc=SAVE_RES;}
					else if(cpu->stack[todo-2] == OUTPUT){
						printf("OUTPUT: %d",cpu->stack[cpu->sp+2]);
						pthread_mutex_lock(&mem_lock);
						sendMessage(buss_Min,Message_packing(cpu->stack[todo-2],1,cpu->stack[todo],cpu->stack[cpu->sp+2])); //1 for writing
						pthread_mutex_unlock(&mem_lock);
					}
					else{
						   cpu->pc=SEND_RES;
					}
				}
				break;
			case SEND_RES:
				//TODO: send to a cpu

				cpu->pc = FND;
				cpu->stack[6+cpu->stack[5]] -= 1;
				break;
			case SAVE_RES:
			{
				//should save node_num, value and offset_address
				int todo = cpu->stack[6+cpu->stack[5]]*3;
				for(int i = 0; i<LS_SIZE; i++){
					if(cpu->local_mem[0][i] == UNDEFINED){
						cpu->local_mem[0][i] = cpu->stack[cpu->sp]; //node_num
						cpu->local_mem[1][i] = cpu->stack[todo]; //offset_address
						cpu->local_mem[2][i] = cpu->stack[cpu->sp+2]; //node value
						cpu->local_mem[3][i] = cpu->stack[todo-1]; //node dest
						cpu->local_mem[4][i] = 0; //unused
						break;
					}
				}
				cpu->pc = FND;
				cpu->stack[6+cpu->stack[5]] -= 1;
				break;
			}
			//op code add
			case code_plus:
			  //add
				cpu->stack[cpu->sp+2] = cpu->stack[cpu->sp+6]+cpu->stack[cpu->sp+7];
				cpu->pc = FND;
				break;
			case code_minus:
				cpu->stack[cpu->sp+2] = cpu->stack[cpu->sp+6]-cpu->stack[cpu->sp+7];
				cpu->pc = FND;
				break;
			case code_times:
				cpu->stack[cpu->sp+2] = cpu->stack[cpu->sp+6]*cpu->stack[cpu->sp+7];
				cpu->pc = FND;
				break;
			case code_is_equal:
				cpu->stack[cpu->sp+2] = (cpu->stack[cpu->sp+6] == cpu->stack[cpu->sp+7]) ? 1 : 0;
				cpu->pc = FND;
				break;
			case code_is_less:
				cpu->stack[cpu->sp+2] = (cpu->stack[cpu->sp+6] < cpu->stack[cpu->sp+7]) ? 1 : 0;
				cpu->pc = FND;
				break;
			case code_is_greater:
				cpu->stack[cpu->sp+2] = (cpu->stack[cpu->sp+6] > cpu->stack[cpu->sp+7]) ? 1 : 0;
				cpu->pc = FND;
				break;
			case NVA:
				{
					int node_needed = getData(popMessage(buss_Mout));
					int cpu_dest = getData(popMessage(buss_Mout));
					int node_dest = getData(popMessage(buss_Mout));
					if(node_needed == cpu->stack[cpu->sp]){
						//modify correst dest
						int num_dest = cpu->stack[6+cpu->stack[5]];
						for(int i =0; i<num_dest; i++){
							if(((cpu->stack[num_dest]+2)+(3*i)) == node_dest){
								cpu->stack[(cpu->stack[num_dest]+1)+(3*i)] = cpu_dest;
								break;
							}
						}

					}else{
						//should be in local mem
						for(int i = 0; i<LS_SIZE; i++){
							if(cpu->local_mem[0][i] == node_needed && cpu->local_mem[3][i] == node_dest){
								//todo send var to cpu that needs it
							}
						}
					}
					cpu->pc = cpu->stack[cpu->sp+cpu->stack[cpu->sp+3]];
					break;
				}
			case EOM:
				{
					cpu->stack[cpu->sp+cpu->stack[cpu->sp+3]] = UNDEFINED;
					cpu->pc = DEC;
					break;
				}

			//shouldnt happen
			default:
				printf("cpu->pc %d undefined operation\n",cpu->pc);
				exit(0);
				break;

		}
		pthread_mutex_unlock(&cpu_lock);


	}

}


void message_listening(struct CPU_H *cpu){
	int op = CB;
	while(1){

			if(buss_Mout->size > 0){
				pthread_mutex_lock(&mem_lock);
				struct Message *m = peekMessage(buss_Mout);
				if(m != NULL){
					int cpu_n = getCpuNum(m); //fetch cpu number
					if(cpu_n == cpu->cpu_num){
						removeMessage(buss_Min); //remove the message from the buss
						if(getAddr(m) == OPR){
							pthread_mutex_lock(&cpu_lock);
							cpu->stack[cpu->stack[cpu->sp+3]] = cpu->pc;
							cpu->pc = getData(m);
							pthread_mutex_unlock(&cpu_lock);
						}else{
							pthread_mutex_lock(&cpu_lock);
							cpu->stack[getAddr(m)] = getData(m);
							pthread_mutex_unlock(&cpu_lock);
						}

					}
				}
				free(m);
				pthread_mutex_unlock(&mem_lock);
			}
			//now check cpu fifo
		}

}



struct Message*  Message_packing(int cpu_num, int rw, int addr, int data ){

			struct Message* temp = (struct Message*)malloc(sizeof(struct Message));

			unsigned int address = ((cpu_num & 0x0000003F) << 26)
											 | ((rw & 0x00000001) << 25)
											 | (addr & 0x0001FFFF);

			//printf("addr: %d\n", addr & 0x0001FFFF);

			temp->addr = address;
			temp->data = data;
			temp->next = NULL;

			return temp;

}


void Message_printing(struct Message *message){

			/* address 32 bits
			[cpu number][R/W][addr]
			[6b][1b][25b]
			*/

			int cpu_num, rw, addr;

			addr = (int) message->addr & 0x0001FFFF; //fetching addr
			rw = (int) ( message->addr >> 25 ) & 0x00000001; //fetching read or write
			cpu_num = (int) ( message->addr >> 26 ) & 0x0000003F; //fetc cpu number

			printf("CPU %d:\n  R/W: %d\n  addr: %d\n  data: %d\n",cpu_num,rw,addr,message->data);
			//printf("- cpu number: %d , binary representation: ", cpu_num);
//			bin_representation(cpu_num);

	//		(rw == 1) ? printf("- write\n") : printf("- read\n");

			//printf("- address: %d , binary representation: ", addr);
//			bin_representation(addr);
			//puts("\n");

			return;
}

int getCpuNum(struct Message *message){
		return (int) ( message->addr >> 26 ) & 0x0000003F;
}

int getRW(struct Message *message){
		return (int) ( message->addr >> 25 ) & 0x00000001;
}

int getAddr(struct Message *message){
		return (int) message->addr & 0x0001FFFF;
}

int getData(struct Message *message){
		return message->data;
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

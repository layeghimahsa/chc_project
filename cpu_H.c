#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include "cpu.h"
#include "2by2sim.h"
#include "many_core.h"

extern FILE *f_min;
extern FILE *f_mout;
extern FILE **f_cpus;
extern int GRAPH;
extern clock_t BEGIN;


void *CPU_H_start(struct CPU_H *cpu){

	int cpu_num = cpu->cpu_num;

	if(MESSAGE == 1)
		printf("CPU %d 	START!!\n",cpu_num);

	for(int i = 0; i<LS_SIZE; i++){
		cpu->local_mem[0][i] = UNDEFINED;
	}

	for(int i = 0; i<ADDRASABLE_SPACE; i++){
		cpu->stack[i] = UNDEFINED;
	}
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	//create listening thread


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
	int count = 0;
	int next_op = 0;
	int add_to_buff = 0;
	double ct;//curre time
	double pt;// previous time
	struct Message *buff = cpu->buffer;
	cpu->buffer = (struct Message*)malloc(sizeof(struct Message));

	cpu->bp = 0;
	cpu->pc = RT; //first instruction is to request task
	cpu->sp = 0;

	while(1){

		sleep(0.01);

	//	printf("CPU %d op: %d\n",cpu->cpu_num, cpu->pc);
	//	printf("buss_Min size %d\n",getFifoSize(buss_Min));
	//	printf("buss_Mout size %d\n",getFifoSize(buss_Mout));
	/*	if(getFifoSize(buss_Mout) > 1){
			printf("buss_Mout first item %d\n",getCpuNum(peekMessage(buss_Mout)));

			printf("buss_Mout first item %d\n",getAddr(peekMessage(buss_Mout)));
			printf("buss_Mout first item %d\n",getData(peekMessage(buss_Mout)));
		}//*/

		if(GRAPH){
					char buss_mout_size[100]; //50 was low for buss_mout_size and lead to core dumped
					char buss_mout_time[100];
					char buss_cpu_size[100];
					char buss_cpu_time[100];
					sprintf(buss_mout_size, "%d", getFifoSize(buss_Mout));
					sprintf(buss_cpu_size, "%d", getFifoSize(cpu->look_up[cpu->cpu_num-1]));
					clock_t t = clock();
					ct = ((double)(t - BEGIN)/CLOCKS_PER_SEC);

					if(ct - pt > 0.1){ //to avoid so many similar data measurement.
						if(cpu->cpu_num == 1){
							sprintf(buss_mout_time, "%f", ct);
							fputs(buss_mout_time,f_mout);
							fputs(" ",f_mout);
							fputs(buss_mout_size, f_mout);
							fputs("\n",f_mout);
						}
						//personal queue size
						//printf(" %s \n",buss_cpu_size);
						sprintf(buss_cpu_time, "%f", ct);
						fputs(buss_cpu_time,f_cpus[cpu->cpu_num-1]);
						fputs(" ",f_cpus[cpu->cpu_num-1]);
						fputs(buss_cpu_size, f_cpus[cpu->cpu_num-1]);
						fputs("\n",f_cpus[cpu->cpu_num-1]);

						pt = ct;
					}
			}


		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		if(getFifoSize(buss_Mout) > 0){
			pthread_mutex_lock(&mem_lock);
			struct Message *m = peekMessage(buss_Mout);
			pthread_mutex_unlock(&mem_lock);
			if(m != NULL){
				int cpu_n = getCpuNum(m); //fetch cpu number
				if(cpu_n == cpu->cpu_num){
					//printf("CPU %d received message from buss\n",cpu->cpu_num);
					pthread_mutex_lock(&mem_lock);
					removeMessage(buss_Mout); //remove the message from the buss
					pthread_mutex_unlock(&mem_lock);

					if(add_to_buff==1){
						if(getAddr(m) == OPR && getData(m)==EOM){
							cpu->stack[cpu->stack[cpu->sp+3]] = cpu->pc;
							cpu->pc = next_op;
							add_to_buff = 0;
						}else{
							*buff = *m;
							buff->next = (struct Message*)malloc(sizeof(struct Message));
							buff = buff->next;
						}
					}else if(getAddr(m) == OPR){
						add_to_buff = 1;
						next_op = getData(m);
						buff = cpu->buffer;
					}else{
						cpu->stack[getAddr(m)] = getData(m);
					}

				}
			}
			free(m);
		}
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		//now check cpu fifo
		if(getFifoSize(cpu->look_up[cpu->cpu_num-1]) > 0){
			if(MESSAGE == 1)
				printf("CPU %d received message from cpu com\n",cpu->cpu_num);
			pthread_mutex_lock(&mem_lock);
			struct Message *m = popMessage(cpu->look_up[cpu->cpu_num-1]);
			pthread_mutex_unlock(&mem_lock);
			if(m!=NULL){
					int cpu_n = getCpuNum(m); //fetch cpu number
					if(cpu_n != cpu->cpu_num){ //the message is not for you
						pthread_mutex_lock(&mem_lock);
						sendMessage(cpu->look_up[cpu_n-1],Message_packing(cpu_n,getRW(m),getAddr(m),getData(m)));
						pthread_mutex_unlock(&mem_lock);
					}else{ //this should only be operation for now
						//writing var in
						if(MESSAGE == 1)
							printf("CPU %d Writing %d to address %d\n",cpu->cpu_num, getData(m), getAddr(m));
						if(cpu->stack[cpu->sp+4] == code_input){
							cpu->stack[cpu->sp+2] = getData(m);
							cpu->stack[cpu->sp+4] = code_identity;
						}else{
							cpu->stack[getAddr(m)] = getData(m);
						}
						cpu->stack[cpu->sp+1]-=1;
					}
			}
			free(m);
		}
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
		switch(cpu->pc){
			//request task
			case RT:
			{
				pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
				pthread_mutex_lock(&mem_lock);
				sendMessage(buss_Min,Message_packing(cpu->cpu_num,1,OPR,REQ_TASK));
				pthread_mutex_unlock(&mem_lock);
				cpu->pc=WFC;
				pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
				break;
			}
			//wait for dependables to start
			case WTS:
			{
			/*	printf("CPU %d \n",cpu->cpu_num);
				for(int i = 0; i<64; i++){
					printf(" [%d]\n",i,cpu->stack[i]);
				}*/
				if(cpu->stack[cpu->sp+1]==0){
					cpu->pc = DEC;
				}else{
					//printf("CPU %d has %d dependables left\n",cpu->cpu_num,cpu->stack[cpu->sp+1]);
					sleep(0.01);
				}
				break;
			}
			case WFC:
			{
				sleep(0.05);
				break;
			}
				//decode operation
			case DEC:
			{
				cpu->pc = cpu->stack[cpu->sp+4];
				pthread_mutex_lock(&mem_lock);
				sendMessage(buss_Min,Message_packing(cpu->cpu_num,1,OPR,MD));
				sendMessage(buss_Min,Message_packing(cpu->cpu_num,1,0,cpu->stack[cpu->sp]));
				pthread_mutex_unlock(&mem_lock);
				break;
			}
			//NO operation
			case IDLE:
			{
				if(count == 500){
					count = 0;
					cpu->pc = RT;
				}else{
					count++;
					sleep(0.05);
				}

				break;
			}
			//for number of destinations
			//send or save result
			case FND:
				{
					pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
					if(cpu->stack[6+cpu->stack[5]] == 0){
						cpu->pc = CS;
					}else{
						int todo = 6+cpu->stack[5]+(cpu->stack[6+cpu->stack[5]]*3);
						if(MESSAGE == 1)
							printf("CPU %d Dest offset address is %d\n",cpu->cpu_num,todo);

						if(cpu->stack[todo-2] == UNKNOWN){cpu->pc=SAVE_RES;
							if(MESSAGE == 1)
								printf("CPU %d saving val %d for node %d\n",cpu->cpu_num,cpu->stack[cpu->sp],cpu->stack[todo-1]);
						}
						else if(cpu->stack[todo-2] == IGNORE){
							cpu->stack[6+cpu->stack[5]] -= 1;
						}
						else if(cpu->stack[todo-2] == OUTPUT){
							printf("\n\nCPU %d OUTPUT: %d\n\n",cpu->cpu_num,cpu->stack[cpu->sp+2]);
							/*for(int i = 0; i<cpu->stack[3]; i++){
								printf("stack [%d] [%d]\n",i,cpu->stack[i]);
							}*/
							//printf("CPU %d outputing address [%d] val [%d]\n",cpu->cpu_num,cpu->stack[todo],cpu->stack[cpu->sp+2]);
							pthread_mutex_lock(&mem_lock);
							sendMessage(buss_Min,Message_packing(cpu->cpu_num,1,cpu->stack[todo],cpu->stack[cpu->sp+2])); //1 for writing
							pthread_mutex_unlock(&mem_lock);
							cpu->stack[6+cpu->stack[5]] -= 1;
						}
						else{
							   cpu->pc=SEND_RES;
								 if(MESSAGE == 1)
								 		printf("CPU %d sending result %d to cpu %d address %d\n",cpu->cpu_num,cpu->stack[cpu->sp+2],cpu->stack[todo-2],cpu->stack[todo]);
						}
					}
					pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
					break;
				}
			case SEND_RES:
				{
					//TODO: send to a cpu
					pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
					//int todo = cpu->stack[6+cpu->stack[5]]*3;
					int todo = 6+cpu->stack[5]+(cpu->stack[6+cpu->stack[5]]*3);
					/*for(int i = 0; i<cpu->stack[3]; i++){
						printf("stack [%d] [%d]\n",i,cpu->stack[i]);
					}*/
					if(MESSAGE == 1)
						printf("CPU %d sending result %d to cpu %d address %d\n",cpu->cpu_num,cpu->stack[cpu->sp+2],cpu->stack[todo-2],cpu->stack[todo]);
					sleep(0.01);
					pthread_mutex_lock(&mem_lock);
					sendMessage(cpu->look_up[cpu->stack[todo-2]-1],Message_packing(cpu->stack[todo-2],1,cpu->stack[todo],cpu->stack[cpu->sp+2])); //1 for writing
					pthread_mutex_unlock(&mem_lock);

					cpu->pc = FND;
					cpu->stack[6+cpu->stack[5]] -= 1;
					pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
					break;
				}
			case SAVE_RES:
			{
				//should save node_num, value and offset_address
				//int todo = cpu->stack[6+cpu->stack[5]]*3;
				int todo = 6+cpu->stack[5]+(cpu->stack[6+cpu->stack[5]]*3);
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
			case code_input:
			{
				printf("\t<< "); scanf("%d",cpu->stack[2]);
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
			case code_if:
			{
				pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
				if((cpu->stack[6] != 0))
				{
					(cpu->stack[2] = cpu->stack[7]);
					pthread_mutex_lock(&mem_lock);
					//mark_as_dead(cpu->stack[sp]);
					sendMessage(buss_Min,Message_packing(cpu->cpu_num,1,OPR,MD));
					sendMessage(buss_Min,Message_packing(cpu->cpu_num,1,0,cpu->stack[cpu->sp]));
					pthread_mutex_unlock(&mem_lock);
				}
				else
				{
					cpu->stack[2] = 0;
					pthread_mutex_lock(&mem_lock);
					sendMessage(buss_Min,Message_packing(cpu->cpu_num,1,OPR,PD));
					sendMessage(buss_Min,Message_packing(cpu->cpu_num,1,0,cpu->stack[cpu->sp]));
					pthread_mutex_unlock(&mem_lock);
				}
				cpu->pc = FND;
				pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
				break;
			}
			case code_else:
			{
				pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
				if(cpu->stack[6] == 0)
				{
					(cpu->stack[2] = cpu->stack[7]);
					pthread_mutex_lock(&mem_lock);
					sendMessage(buss_Min,Message_packing(cpu->cpu_num,1,OPR,MD));
					sendMessage(buss_Min,Message_packing(cpu->cpu_num,1,0,cpu->stack[cpu->sp]));
					pthread_mutex_unlock(&mem_lock);
				}
				else
				{
					cpu->stack[2] = 0;
					pthread_mutex_lock(&mem_lock);
					sendMessage(buss_Min,Message_packing(cpu->cpu_num,1,OPR,PD));
					sendMessage(buss_Min,Message_packing(cpu->cpu_num,1,0,cpu->stack[cpu->sp]));
					pthread_mutex_unlock(&mem_lock);
				}
				cpu->pc = FND;
				pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
				break;
			}
			case code_merge:
			{
				cpu->stack[cpu->sp+2] = (cpu->stack[cpu->sp+ 6] | cpu->stack[cpu->sp+7]);
				//printf("CPU %d MERGE %d & %d = %d\n",cpu_num,cpu->stack[6],cpu->stack[7],cpu->stack[2]);
				cpu->pc = FND;
				break;
			}
			case code_identity:
			{
				if(cpu->stack[2] == NAV){cpu->stack[2] = cpu->stack[6];}
				cpu->pc = FND;
				break;
			}
			//changing a destination address or sending a var needed to another cpu
			case NVA:
				{
						pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
						if(MESSAGE == 1)
							printf("CPU %d got var request\n",cpu->cpu_num);
						int node_needed = getData(cpu->buffer);
						int cpu_dest = getData(cpu->buffer->next);
						int node_dest = getData(cpu->buffer->next->next);

						if(node_needed == cpu->stack[cpu->sp]){
							if(cpu->stack[cpu->sp+cpu->stack[cpu->sp+3]] < -5){
								if(MESSAGE == 1)
									printf("CPU %d sending var\n",cpu_num);
								for(int i = 6+cpu->stack[cpu->sp+5]+2; i < cpu->stack[cpu->sp+3];i+=3){
									if(cpu->stack[cpu->sp+i]==node_dest){
										pthread_mutex_lock(&mem_lock);
										sendMessage(cpu->look_up[cpu_dest-1],Message_packing(cpu_dest,1,cpu->stack[i+1],cpu->stack[cpu->sp+2])); //1 for writing
										pthread_mutex_unlock(&mem_lock);
										break;
									}
								}

							}else{
								if(MESSAGE == 1)
									printf("CPU %d changing dest address\n",cpu_num);
								int num_dest = cpu->stack[6+cpu->stack[5]];
								//modify correst dest
								for(int i =0; i<num_dest; i++){
									if(cpu->stack[(6+cpu->stack[5]+2)+(3*i)] == node_dest){
										//printf("FOUND\n");
										cpu->stack[(6+cpu->stack[5]+1)+(3*i)] = cpu_dest;
										break;
									}
								}
							}//*/

						}else{
							if(MESSAGE == 1)
								printf("CPU %d fetching from mem\n",cpu->cpu_num);
							//should be in local mem
							int found = 0;
							for(int i = 0; i<LS_SIZE; i++){
								if(cpu->local_mem[0][i] == node_needed && cpu->local_mem[3][i] == node_dest){
										pthread_mutex_lock(&mem_lock);
										sendMessage(cpu->look_up[cpu_dest-1],Message_packing(cpu_dest,1,cpu->local_mem[1][i],cpu->local_mem[2][i])); //1 for writing
										pthread_mutex_unlock(&mem_lock);
										found =1;
										break;
								}
							}
							if(found == 0){
								printf("FAILED TO FIND MEM %d for %d\n",node_needed,node_dest);
							}
						}

					cpu->pc = cpu->stack[cpu->sp+cpu->stack[cpu->sp+3]];
					if(MESSAGE == 1)
						printf("CPU %d returning to task %d\n",cpu->cpu_num,cpu->pc);
					pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
					break;
				}
			case CS:
			{
				//clear stack
				for(int i = 0; i<ADDRASABLE_SPACE; i++){
					cpu->stack[i] = UNDEFINED;
				}
				cpu->pc = RT;
				break;
			}
			//shouldnt happen
			default:
				printf("CPU %d cpu->pc %d undefined operation\n",cpu->cpu_num,cpu->pc);
				exit(0);
				break;
		}
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include "cpu.h"
#include "2by2sim.h"
#include "many_core.h"


void *CPU_SA_start(struct CPU_SA *cpu){

	int cpu_num = cpu->cpu_num;

	//if(MESSAGE == 1)
		printf("CPU %d 	START!!\n",cpu_num);


		int stack[ADDRASABLE_SPACE];
		for(int i = 0; i<ADDRASABLE_SPACE; i++){
			stack[i] = STACK_UNDEFINED;
		}

		//BEGIN: fill stack with all nodes from main
		int sp_top = ADDRASABLE_SPACE-1; //always point to the top of the stack
		int lp = 0;
		int pc = LFN;
		//get main dictionary entries
		int main_size;
		for(int i = 0; i<cpu->num_dict_entries; i++){
			if(cpu->dictionary[i][0] == cpu->main_addr){
				main_size = cpu->dictionary[i][1];
				break;
			}
		}

		int j;
		for(int i=0; i<cpu->code_size; i++){//traverse through all the nodes in a cpu
			//printf("CPU %d if(%d >= %d && %d <= %d)\n",cpu_num,cpu->PM[i+1],cpu->main_addr,cpu->PM[i+1],(cpu->main_addr+main_size));
			//if (lp_offset<r_offset<lp_offset+lp_size)
			if(cpu->PM[i+1] >= cpu->main_addr && cpu->PM[i+1] <= (cpu->main_addr+main_size)){ //check if the node is in main
				//add to stack
				sp_top--;
				j=i+cpu->PM[i+4]-1; // node size
				while(cpu->PM[j-1] != NODE_BEGIN_FLAG){
					stack[sp_top]=cpu->PM[j];
					sp_top--;j--;
				}
				stack[lp] = sp_top; //pointer to node... maybe we can embedded this into the 32 bit if mem size can be more restricted
				stack[lp+1] = lp_entry(cpu->PM[j+3],cpu->PM[j]); //needs to be changed to pack size and offset it same
				lp+=2;j--;
				stack[sp_top] = cpu->PM[j];
			}
			i = i + cpu->PM[i+4];
		}
		lp--;
	/*	for(int i = sp_top; i<ADDRASABLE_SPACE; i++){
			printf("CPU %d [%d][%d]\n",cpu_num,i,stack[i]);
		}*/
		/*END: stack is filled*/

		int ftfn = 0; //failed to find node count
		int idle_count = 0;

		int sp = sp_top;

		printf("CPU %d entering loop\n",cpu_num);

		while(1){

			//com part
			if(getFifoSize(buss)>0){
				struct Message *m = peekMessage(buss);
				if(getCpuNum(m)==cpu_num){
					//ignore messages from self
				}else if(getAddr(m)==OPR){

				}else{ //this would be just a write
					//check if any entries in lp holds the receiving node
					int lp_t = lp;
					int size;
					int offset;
					int m_addr = getAddr(m);
					while(lp_t>=0){
							size = getSize(stack[lp_t]);
							offset = getOffset(stack[lp_t]);
							//if true the val is for it
							if(m_addr > offset && m_addr < offset+size){
								stack[stack[lp_t-1]+(m_addr-offset)] = getData(m);
								stack[stack[lp_t-1]+1] -= 1; //reduce number of dependants by one
								pthread_mutex_lock(&buss->fifo_lock);
								removeMessage(buss);
								pthread_mutex_unlock(&buss->fifo_lock);
								break;
							}
						lp_t-=2;
					}
				}
				free(m);
			}

			//printf("CPU %d pc %d\n",cpu_num, pc);
			switch(pc){
				case code_input:
				{
					printf("\t<< "); scanf("%d",stack[sp+2]);//stack[2] or stack[sp_top+2]
					break;
				}
				//op code add
				case code_plus:
				  //add
					stack[sp+2] = stack[sp+6]+stack[sp+7];
					pc = FND;
					break;
				case code_minus:
					stack[sp+2] = stack[sp+6]-stack[sp+7];
					pc = FND;
					break;
				case code_times:
					stack[sp+2] = stack[sp+6]*stack[sp+7];
					pc = FND;
					break;
				case code_is_equal:
					stack[sp+2] = (stack[sp+6] == stack[sp+7]) ? 1 : 0;
					pc = FND;
					break;
				case code_is_less:
					stack[sp+2] = (stack[sp+6] < stack[sp+7]) ? 1 : 0;
					pc = FND;
					break;
				case code_is_greater:
					stack[sp+2] = (stack[sp+6] > stack[sp+7]) ? 1 : 0;
					pc = FND;
					break;
				case code_if:
				{
					//pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
					if((stack[6] != 0))
					{
						(stack[2] = stack[7]);
					//	pthread_mutex_lock(&mem_lock);
						//mark_as_dead(stack[sp]);
					//	sendMessage(buss_Min,Message_packing(cpu->cpu_num,1,OPR,MD));
					//	sendMessage(buss_Min,Message_packing(cpu->cpu_num,1,0,stack[sp]));
					//	pthread_mutex_unlock(&mem_lock);
					}
					else
					{
						stack[2] = 0;
					//	pthread_mutex_lock(&mem_lock);
					//	sendMessage(buss_Min,Message_packing(cpu->cpu_num,1,OPR,PD));
					//	sendMessage(buss_Min,Message_packing(cpu->cpu_num,1,0,stack[sp]));
					//	pthread_mutex_unlock(&mem_lock);
					}
					pc = FND;
					//pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
					break;
				}
				case code_else:
				{
					//pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
					if(stack[6] == 0)
					{
						(stack[2] = stack[7]);
					//	pthread_mutex_lock(&mem_lock);
						//sendMessage(buss_Min,Message_packing(cpu->cpu_num,1,OPR,MD));
					//	sendMessage(buss_Min,Message_packing(cpu->cpu_num,1,0,stack[sp]));
					//	pthread_mutex_unlock(&mem_lock);
					}
					else
					{
						stack[2] = 0;
					//	pthread_mutex_lock(&mem_lock);
					//	sendMessage(buss_Min,Message_packing(cpu->cpu_num,1,OPR,PD));
					//	sendMessage(buss_Min,Message_packing(cpu->cpu_num,1,0,stack[sp]));
					//	pthread_mutex_unlock(&mem_lock);
					}
					pc = FND;
					//pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
					break;
				}
				case code_merge:
				{
					stack[sp+2] = (stack[sp+ 6] | stack[sp+7]);
					//printf("CPU %d MERGE %d & %d = %d\n",cpu_num,stack[6],stack[7],stack[2]);
					pc = FND;
					break;
				}
				case code_identity:
				{
					if(stack[2] == NAV){stack[2] = stack[6];}
					pc = FND;
					break;
				}
				case LFN: //look for node to run
				{
					//find runnable node
					//if found
					// - pc = stack[sp+4];
					//else
					// - see if its failed to find a node too many times
					//	- if no then pc = idle
					//	- if yes then send node request on broadcast
					//printf("%d\n",stack[sp_top]);
					sp=sp_top;
					int found = 0;
					while(sp<ADDRASABLE_SPACE-2 && found == 0){
						if(stack[sp+1]==0){
							found = 1;
							break;
						}else{
							sp = sp + stack[sp+3];
						}
					}
					//printf("CPU %d looking for node\n",cpu_num);
					if(found == 1){
						printf("CPU %d found a node!!\n",cpu_num);
						//yay we found a node and can begin
						pc = stack[sp+4];
						//reset ftfn
						ftfn = 0;
					}else{
						//no node found. go idle for (blah) amount of time then try again
						//if failed attempts to find node = REQ_TIME then send bradcast request message for a node
						if(ftfn == FTFN_MAX){
							//send node request broadcast
							pc = IDLE;
						}else{
							pc = IDLE;
						}
						ftfn++;
					}

					break;
				}
				case FND:
				{
					printf("CPU %d sending results %d\n",cpu_num,stack[sp+2]);
					int num_dest = stack[sp+6+stack[sp+5]];
					int doffset = sp+6+stack[sp+5]+1;
					for(int i =0; i<num_dest;i++){
						if(stack[doffset]== IGNORE){//may not be needed anymore
						}else if(stack[doffset] == OUTPUT){
							printf("CPU %d OUTPUT %d\n",cpu_num,stack[sp+2]);
						}else{

							//is it in this cpu or must it be sent output
							int lp_t = lp;
							int size;
							int offset;
							int m_addr;
							int found = 0;
							//now must find own offset from lp
							while(lp_t>=0){
								if(stack[lp_t-1] == sp){
									break;
								}
								lp_t-=2;
							}
							m_addr = getOffset(stack[lp_t])+stack[doffset];
							lp_t = lp;
							//now lets see if its dest is in its own stack
							while(lp_t>=0){
									size = getSize(stack[lp_t]);
									offset = getOffset(stack[lp_t]);
									//if true the val is for it
									if(m_addr > offset && m_addr < offset+size){
										found = 1;
										break;
									}
								lp_t-=2;
							}
							//send or write result
							if(found){
								stack[stack[lp_t-1]+(m_addr-offset)] = stack[sp+2];
								stack[stack[lp_t-1]+1] -= 1;
							}else{
								pthread_mutex_lock(&buss->fifo_lock);
								sendMessage(buss,Message_packing(cpu_num,1,m_addr,stack[sp+2]));
								pthread_mutex_unlock(&buss->fifo_lock);
							}

						}
						doffset++;
					}
					pc = SDOWN;
					break;
				}
				case MAD: //mark as dead
				{
					//send out MAD op on bus for all dest
					//remove lp and sp enty of node
					//shift down
					//recover pc
					break;
				}
				case SDOWN: //shift down **cant be interupted**
				{
					//remove lp and sp entry of node
          //shift down
          //pc = LFN;

          int lp_tmp = lp;
          while(stack[lp_tmp-1] != sp){
              lp_tmp -= 2;
          }
          //lp_t now point to node_size and offset of a node and it's in a correct position
          int lp_below = lp_tmp+2;


          int tbd_start = sp; //the start of a node to be deleted
          int tbd_end = sp+stack[sp+4]; //the end of a node to be deleted
          int tn_start = sp_top; //the beggining of the toppest node in stack
          int tn_end = tbd_start-1; //the end of the node before the tbd node


          while(tn_end >= sp_top){
              stack[tbd_end] = stack[tn_end];
							if(stack[tbd_end] == NODE_BEGIN_FLAG){
								stack[lp_tmp] = stack[lp_below];
	              stack[lp_tmp-1] = tbd_end; //set the pointer to the new position
	              lp_below+=2;
	              lp_tmp+=2;
							}
              tn_end--;
              tbd_end--;
          }
          int new_sp = tbd_end+1;
          while(tbd_end >= sp_top){
              stack[tbd_end] = STACK_UNDEFINED;
          }
          //updating stack top
          sp_top = new_sp;

		     lp_tmp-=2; //refactor pointer
          int new_lp = lp_tmp;
          while(lp_tmp!=lp){
              stack[lp_tmp+1] = STACK_UNDEFINED;
              stack[lp_tmp+2] = STACK_UNDEFINED;
              lp_tmp+=2;
          }
          //updating lp
          lp = new_lp;

        	break;
				}
				case IDLE:
				{
					if(idle_count == 150){
						pc=LFN;
					}else{
						idle_count++;
						sleep(0.05);
					}
					break;
				}
				default:
				{
					printf("CPU %d unrecognized operation %d\n",cpu_num,pc);
					exit(0);
				}

			}

		}


}

int lp_entry(int size, int offset){
	unsigned int entry = ((size & 0x000000FF) << 24) | (offset & 0x00FFFFFF);
	return entry;
}

int getSize(int entry){
	return (int) ( entry >> 24 ) & 0x000000FF;
}

int getOffset(int entry){
	return (int) (entry & 0x00FFFFFF);
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

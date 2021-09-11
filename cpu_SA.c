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

  struct FIFO *buffer = buss[cpu_num-1];

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

	int j =cpu->code_size-1;
	while(j>=0){
		if(cpu->PM[j] == cpu->main_addr){ //check if the node is in main
			//add to stack
			sp_top--;
			while(cpu->PM[j-1] != NODE_BEGIN_FLAG){
				stack[sp_top]=cpu->PM[j];
				sp_top--;j--;
			}
			stack[lp] = sp_top; //pointer to node... maybe we can embedded this into the 32 bit if mem size can be more restricted
			stack[lp+1] = lp_entry(cpu->PM[j+3],cpu->PM[j]); //needs to be changed to pack size and offset it same
			lp+=2;j--;
			stack[sp_top] = cpu->PM[j];
			j--;
		}else{
			while(cpu->PM[j] != NODE_BEGIN_FLAG){j--;}
			j--;
		}
	}
	lp--;
//	printf("CPU %d code size %d\n",cpu_num,cpu->code_size);
/*	for(int i = 0; i<cpu->code_size; i++){
		printf("CPU %d PM [%d][%d]\n",cpu_num, i, cpu->PM[i]);
	}//*/
/*		for(int i = 0; i<=lp;i+=2){
	printf("CPU %d [%d][%d]\n",cpu_num,i,stack[i]);
	printf("CPU %d [%d][%d][%d]\n",cpu_num,i,getSize(stack[i+1]),getOffset(stack[i+1]));
}
for(int i = sp_top; i<ADDRASABLE_SPACE; i++){
	printf("CPU %d [%d][%d]\n",cpu_num,i,stack[i]);
}//*/
//exit(0);
	/*END: stack is filled*/

	int largest_offset = cpu->dictionary[0][0]+cpu->dictionary[0][1]+100;
 	int ftfn = 0; //failed to find node count
	int idle_count = 0;
	int oper=0;
	int next_op;
	int sp = sp_top;
	int sp_oper;
	struct FIFO *expand_buffer = create_FIFO();
	int eom_count = 0;
	//printf("CPU %d entering loop\n",cpu_num);

	while(1){

		//com part
		//check own buffer
		pthread_mutex_lock(&buss[cpu_num-1]->fifo_lock);
		int buff_size = getFifoSize(buss[cpu_num-1]);
		pthread_mutex_unlock(&buss[cpu_num-1]->fifo_lock);
		//printf("cpu %d buff size %d\n",cpu_num,buff_size);
		if(buff_size>0){
			pthread_mutex_lock(&buss[cpu_num-1]->fifo_lock);
			struct Message *m = popMessage(buss[cpu_num-1]);
			pthread_mutex_unlock(&buss[cpu_num-1]->fifo_lock);
			if(m==NULL){exit(0);}//should never happen
			if(oper==1){
				if(next_op==MAD){
					//look if node is in stack
					int found = 0;
					int lp_t = lp;
					int size;
					int offset;
					int m_addr = getAddr(m);
					while(lp_t>=0){
							size = getSize(stack[lp_t]);
							offset = getOffset(stack[lp_t]);
							//if true the val is for it
							if(m_addr < offset && m_addr > offset-size){
								sp_oper = stack[lp_t-1];
								if(stack[sp_oper+4]==code_merge){
								}else{
									stack[ADDRASABLE_SPACE-1] = pc;
									pc = next_op;
								}
								break;
							}
						lp_t-=2;
					}
					oper=0;
				}else if(next_op == EXP){
					if(getAddr(m)==OPR && getData(m)==EOM){
						sendMessage(expand_buffer,m);
						eom_count++;
						if(eom_count == 2){
							eom_count = 0;
							oper = 0;
							stack[ADDRASABLE_SPACE-1] = pc;
							pc = next_op;
						}
					}else{
						sendMessage(expand_buffer,m);
					}

				}
			}else if(getAddr(m)==OPR){    //there are
				oper = 1;
				next_op = getData(m);
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
						if(m_addr < offset && m_addr > offset-size){
							printf("CPU %d writing result %d to pos %d of node type %d\n",cpu_num, getData(m),(offset-m_addr-1),stack[stack[lp_t-1]+4]);
							stack[stack[lp_t-1]+(offset-m_addr-1)] = getData(m);
							stack[stack[lp_t-1]+1] -= 1; //reduce number of dependants by one
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
				if(stack[sp+2] == NAV){
					printf("\t<< "); scanf("%d",stack[sp+2]);//stack[2] or stack[sp_top+2]
				}
				pc=FND;
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
				if((stack[sp+6] != 0))
				{
					(stack[sp+2] = stack[sp+7]);
					pc = FND;
				//	pthread_mutex_lock(&mem_lock);
					//mark_as_dead(stack[sp]);
				//	sendMessage(buss_Min,Message_packing(cpu->cpu_num,1,OPR,MD));
				//	sendMessage(buss_Min,Message_packing(cpu->cpu_num,1,0,stack[sp]));
				//	pthread_mutex_unlock(&mem_lock);
				}
				else
				{
					stack[sp+2] = 0;
				//	pthread_mutex_lock(&mem_lock);
				//	sendMessage(buss_Min,Message_packing(cpu->cpu_num,1,OPR,PD));
				//	sendMessage(buss_Min,Message_packing(cpu->cpu_num,1,0,stack[sp]));
				//	pthread_mutex_unlock(&mem_lock);
					int	num_dest = stack[sp+6+stack[sp+5]];
					int	doffset = sp+6+stack[sp+5]+1;
					int m_addr;
					for(int i =0; i<num_dest;i++){
						if(stack[doffset] == OUTPUT){
						}else{
								m_addr = stack[sp+stack[sp+3]-1] + stack[doffset]/4;
								pthread_mutex_lock(&buss_lock);
								sendMessageOnBuss(0,Message_packing(cpu_num,0,OPR,MAD));
								sendMessageOnBuss(0,Message_packing(cpu_num,0,m_addr,MAD));
								pthread_mutex_unlock(&buss_lock);
						}
						doffset++;
					}
					pc = SDOWN;
					//pc = FND;
				}
				//pc = FND;
				//pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
				break;
			}
			case code_else:
			{
				//pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
				if(stack[sp+6] == 0)
				{
					(stack[sp+2] = stack[sp+7]);
					pc = FND;
				//	pthread_mutex_lock(&mem_lock);
					//mark_as_dead(stack[sp]);
				//	sendMessage(buss_Min,Message_packing(cpu->cpu_num,1,OPR,MD));
				//	sendMessage(buss_Min,Message_packing(cpu->cpu_num,1,0,stack[sp]));
				//	pthread_mutex_unlock(&mem_lock);
				}
				else
				{
					stack[sp+2] = 0;
				//	pthread_mutex_lock(&mem_lock);
				//	sendMessage(buss_Min,Message_packing(cpu->cpu_num,1,OPR,PD));
				//	sendMessage(buss_Min,Message_packing(cpu->cpu_num,1,0,stack[sp]));
				//	pthread_mutex_unlock(&mem_lock);
					int	num_dest = stack[sp+6+stack[sp+5]];
					int	doffset = sp+6+stack[sp+5]+1;
					int m_addr;
					for(int i =0; i<num_dest;i++){
						if(stack[doffset] == OUTPUT){
						}else{
								m_addr = stack[sp+stack[sp+3]-1] + stack[doffset]/4;
								pthread_mutex_lock(&buss_lock);
								sendMessageOnBuss(0,Message_packing(cpu_num,0,OPR,MAD));
								sendMessageOnBuss(0,Message_packing(cpu_num,0,m_addr,MAD));
								pthread_mutex_unlock(&buss_lock);
						}
						doffset++;
					}
					pc = SDOWN;
					//pc = FND;
				}
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
				if(stack[sp+2] == NAV){stack[sp+2] = stack[sp+6];}
				pc = FND;
				break;
			}
			case code_expansion:
			{

				//add nodes to stack
				//printf("entering expansion\n");

				struct FIFO *broadcast = create_FIFO();
				sendMessage(broadcast,Message_packing(cpu_num,1,OPR,EXP));

				int func_offset = stack[sp+6+(stack[sp+5]*2)+1];
				int func_size;
				for(int i=0;i<cpu->num_dict_entries;i++){
					if(func_offset == cpu->dictionary[i][0]){
						func_size = cpu->dictionary[i][1];
						break;
					}
				}

				sendMessage(broadcast,Message_packing(cpu_num,0,func_offset,func_size));

				int j;
				lp++;
				int new_func_offset = largest_offset + 1;
				//printf("CPU %d new_offset %d\n",cpu_num,new_func_offset);
				j =cpu->code_size-1;
				while(j>=0){
					if(cpu->PM[j] == func_offset){ //check if the node is in main
						//add to stack
						sp_top--;
						while(cpu->PM[j-1] != NODE_BEGIN_FLAG){
							stack[sp_top]=cpu->PM[j];
							sp_top--;j--;
						}
						stack[lp] = sp_top; //pointer to node... maybe we can embedded this into the 32 bit if mem size can be more restricted
						stack[lp+1] = lp_entry(cpu->PM[j+3],(new_func_offset+cpu->PM[j]-func_offset)); //needs to be changed to pack size and offset it same
						//printf("MM offset %d\n",new_func_offset+cpu->PM[j]-func_offset);
						lp+=2;j--;
						stack[sp_top] = cpu->PM[j];
						stack[sp_top+stack[sp_top+3]-1] = new_func_offset;
						if(stack[sp_top+4]==code_input){stack[sp_top+1]+=1;}
						j--;
					}else{
						while(cpu->PM[j] != NODE_BEGIN_FLAG){j--;}
						j--;
					}
				}
				lp--;
				largest_offset += func_size+100;

				int num_dest = stack[sp+6+(stack[sp+5]*2)];
				int doffset = sp+6+(stack[sp+5]*2)+1;
				int node_to_remap;
				int remap_to_this;
				for(int i=0; i<num_dest;i++){
					doffset += (2*i);
					node_to_remap = new_func_offset+stack[doffset+2]/4;
					//must remove its scope offset and add the scope offset that it wants to send too.
					//however when a result is sent, the dest offset is /4 so here we multiply by 4 to compensate
					remap_to_this = (stack[sp+stack[sp+3]-1] + stack[doffset+1]/4 - new_func_offset)*4;

					int lp_t = lp;
					int size;
					int offset;
					int found = 0;
					//now lets see if its dest is in its own stack
					while(lp_t>=0){
							size = getSize(stack[lp_t]);
							offset = getOffset(stack[lp_t]);
							//if true the val is for it
							if(node_to_remap < offset && node_to_remap > offset-size){
								found = 1;
								break;
							}
						lp_t-=2;
					}
					//send or write result
					if(found){
						//printf("CPU %d found an output node\n",cpu_num);
						int ntr_num_dest = stack[stack[lp_t-1]+6+stack[stack[lp_t-1]+5]];
						for(int k=0;k<ntr_num_dest;k++){
							int off = stack[lp_t-1]+7+stack[stack[lp_t-1]+5]+k;
							if( stack[off] == OUTPUT){
								 stack[off] = remap_to_this;
								 break;
							}
						}
					}else{
						sendMessage(broadcast,Message_packing(cpu_num,1,node_to_remap,remap_to_this));
					}
				}
				sendMessage(broadcast,Message_packing(cpu_num,1,OPR,EOM));

				int num_args = stack[sp+5];
				int aoffset = sp+6;
				int input_node_offset;
				for(int i = 0; i<num_args; i++){
					aoffset = aoffset + (2*i);
					input_node_offset = new_func_offset+stack[aoffset+1]/4;
					int lp_t = lp;
					int size;
					int offset;
					int found = 0;
					//now lets see if its dest is in its own stack
					while(lp_t>=0){
							size = getSize(stack[lp_t]);
							offset = getOffset(stack[lp_t]);
							//if true the val is for it
							if(input_node_offset  < offset && input_node_offset  > offset-size){
								found = 1;
								break;
							}
						lp_t-=2;
					}
					//send or write result
					if(found){
						//printf("CPU %d found an input node\n",cpu_num);
						//change output dest
						stack[stack[lp_t-1]+2] = stack[aoffset];
						stack[stack[lp_t-1]+1]-=1;
					}else{
						sendMessage(broadcast,Message_packing(cpu_num,1,input_node_offset-2,stack[aoffset]));
					}
				}
				sendMessage(broadcast,Message_packing(cpu_num,1,OPR,EOM));

				struct Message *t = popMessage(broadcast);
				pthread_mutex_lock(&buss_lock);
				while(t!=NULL){
					sendMessageOnBuss(cpu_num,t);
					t = popMessage(broadcast);
				}
				pthread_mutex_unlock(&buss_lock);
				free(broadcast);
				free(t);
				/*
				for(int i = 0; i<=lp;i++){
						printf("CPU %d [%d][%d]\n",cpu_num,i,stack[i]);
					}
					for(int i = sp_top; i<ADDRASABLE_SPACE; i++){
						printf("CPU %d [%d][%d]\n",cpu_num,i,stack[i]);
					}

				exit(0);//*/
				pc = SDOWN;
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
				while(sp<ADDRASABLE_SPACE-1){
					if(stack[sp+1]==0){
						found = 1;
						break;
					}else if(stack[sp+1]<0){
						for(int i = 0; i<=lp;i+=2){
							printf("CPU %d [%d][%d]\n",cpu_num,i,stack[i]);
							printf("CPU %d [%d][%d][%d]\n",cpu_num,i,getSize(stack[i+1]),getOffset(stack[i+1]));
						}
						for(int i = sp_top; i<ADDRASABLE_SPACE; i++){
							printf("CPU %d [%d][%d]\n",cpu_num,i,stack[i]);
						}
						exit(0);
					}else{
						sp = sp + stack[sp+3];
					}
				}
				//printf("CPU %d looking for node\n",cpu_num);
				if(found == 1){
					//printf("CPU %d found a node of type %d!!\n",cpu_num,stack[sp+4]);
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
						ftfn = 0;
					//	printf("\n\nCPU %d FTFN\n",cpu_num);
				/*		for(int i = 0; i<=lp;i+=2){
								printf("CPU %d [%d][%d]\n",cpu_num,i,stack[i]);
								printf("CPU %d [%d][%d][%d]\n",cpu_num,i,getSize(stack[i+1]),getOffset(stack[i+1]));
							}
							for(int i = sp_top; i<ADDRASABLE_SPACE; i++){
								printf("CPU %d [%d][%d]\n",cpu_num,i,stack[i]);
							}
							return NULL;*/
					}else{
						pc = IDLE;
					}
					ftfn++;
				}

				break;
			}
			case FND:
			{
				//printf("CPU %d sending results %d\n",cpu_num,stack[sp+2]);
				int num_dest = stack[sp+6+stack[sp+5]];
				int doffset = sp+6+stack[sp+5]+1;
				for(int i =0; i<num_dest;i++){
					if(stack[doffset]== IGNORE){//may not be needed anymore
					}else if(stack[doffset] == OUTPUT){
						printf("CPU %d OUTPUT %d\n",cpu_num,stack[sp+2]);

						for(int i = 0; i<=lp;i+=2){
							printf("CPU %d [%d][%d]\n",cpu_num,i,stack[i]);
							printf("CPU %d [%d][%d][%d]\n",cpu_num,i,getSize(stack[i+1]),getOffset(stack[i+1]));
						}
						for(int i = sp_top; i<ADDRASABLE_SPACE; i++){
							printf("CPU %d [%d][%d]\n",cpu_num,i,stack[i]);
						}//*/
							//exit(0);
					}else{

						//is it in this cpu or must it be sent output
						int lp_t = lp;
						int size;
						int offset;
						int m_addr;
						int found = 0;

						//now must find own dest offset
						m_addr = stack[sp+stack[sp+3]-1] + stack[doffset]/4;
						//printf("M_addr %d\n",m_addr);
						lp_t = lp;
						//now lets see if its dest is in its own stack
						while(lp_t>=0){
								size = getSize(stack[lp_t]);
								offset = getOffset(stack[lp_t]);
								//if true the val is for it
								if(m_addr < offset && m_addr > offset-size){
									found = 1;
									break;
								}
							lp_t-=2;
						}
						//send or write result
						if(found){
							//printf("CPU %d found dest in stack\n",cpu_num);
							//printf("addr %d\n",m_addr);
							stack[stack[lp_t-1]+(offset-m_addr-1)] = stack[sp+2];
							stack[stack[lp_t-1]+1] -= 1;
						}else{
							//printf("addr %d\n",m_addr);
							pthread_mutex_lock(&buss_lock);
							sendMessageOnBuss(cpu_num,Message_packing(cpu_num,1,m_addr,stack[sp+2]));
							pthread_mutex_unlock(&buss_lock);
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

				//for destinations send operation MAD
				int num_dest;
				int doffset;
				if(stack[sp_oper+4]==code_expansion){
					num_dest = stack[sp_oper+6+(stack[sp_oper+5]*2)];
					doffset = sp_oper+6+(stack[sp_oper+5]*2)+2;
				}else{
					num_dest = stack[sp_oper+6+stack[sp_oper+5]];
					doffset = sp_oper+6+stack[sp_oper+5]+1;
				}
				int m_addr;
<<<<<<< Updated upstream
				/*for(int i =0; i<num_dest;i++){
					if(stack[doffset]== IGNORE){//may not be needed anymore
					}else if(stack[doffset] == OUTPUT){
=======
				for(int i =0; i<num_dest;i++){
					if(stack[doffset] == OUTPUT){
>>>>>>> Stashed changes
					}else{
							m_addr = stack[sp_oper+stack[sp_oper+3]-1] + stack[doffset]/4;
							pthread_mutex_lock(&buss_lock);
							sendMessageOnBuss(0,Message_packing(cpu_num,0,OPR,MAD));
							sendMessageOnBuss(0,Message_packing(cpu_num,0,m_addr,MAD));
							pthread_mutex_unlock(&buss_lock);
					}
					if(stack[sp_oper+4]==code_expansion){doffset+=3;}
					else{doffset++;}
<<<<<<< Updated upstream
				}*/
=======
				}//*/

				if(sp_oper == sp){
					printf("NODE TO REMOVE IS POINTED BY SP\n");
					pc = LFN;
				}else{
					pc = stack[ADDRASABLE_SPACE-1];
				}


>>>>>>> Stashed changes
					//delete the node by removing it and shifting down the stack if needed
				if(sp_oper == sp_top){
					sp_top = sp_oper+stack[sp_oper+3];
					int to = sp_top-1;
					while(to+1 != sp_oper){
						stack[to] = STACK_UNDEFINED;
						to--;
					}
					stack[lp] = STACK_UNDEFINED;
					stack[lp-1] = STACK_UNDEFINED;
					lp-=2;

				}else{
					int lp_tmp = lp;
          while(stack[lp_tmp-1] != sp_oper){
              lp_tmp -= 2;
          }
					int to = sp_oper+stack[sp_oper+3]-1;
					int from = sp_oper-1;
					while(from != sp_top-1){
						stack[to] = stack[from];
						if(stack[to] == NODE_BEGIN_FLAG){//fix lp entry
							stack[lp_tmp-1] = to;
							stack[lp_tmp] = stack[lp_tmp+2];
							lp_tmp += 2;
						}
						if(sp==from){sp=to;}
						to--;from--;
					}
					sp_top = to+1;
					stack[lp] = STACK_UNDEFINED;
					stack[lp-1] = STACK_UNDEFINED;
					lp-=2;
					while(to!=from){
						stack[to] = STACK_UNDEFINED;
						to--;
					}
				}
				break;
			}
			case SDOWN: //shift down **cant be interupted**
			{
				//remove lp and sp entry of node
        //shift down
        //pc = LFN;

				if(sp == sp_top){
					sp_top = sp+stack[sp+3];
					int to = sp_top-1;
					while(to+1 != sp){
						stack[to] = STACK_UNDEFINED;
						to--;
					}
					stack[lp] = STACK_UNDEFINED;
					stack[lp-1] = STACK_UNDEFINED;
					lp-=2;

				}else{
					int lp_tmp = lp;
          while(stack[lp_tmp-1] != sp){
              lp_tmp -= 2;
          }
					int to = sp+stack[sp+3]-1;
					int from = sp-1;
					while(from != sp_top-1){
						stack[to] = stack[from];
						if(stack[to] == NODE_BEGIN_FLAG){//fix lp entry
							stack[lp_tmp-1] = to;
							stack[lp_tmp] = stack[lp_tmp+2];
							lp_tmp += 2;
						}
						to--;from--;
					}
					sp_top = to+1;
					stack[lp] = STACK_UNDEFINED;
					stack[lp-1] = STACK_UNDEFINED;
					lp-=2;
					while(to!=from){
						stack[to] = STACK_UNDEFINED;
						to--;
					}
				}

				pc = LFN;
      	break;
			}
			case EXP:
			{


				struct Message *m = popMessage(expand_buffer);

				int func_offset = getAddr(m);
				int func_size = getData(m);
				int j;
				lp++;
				int new_func_offset = largest_offset + 1;
				//printf("CPU %d new_offset %d\n",cpu_num,new_func_offset);
				j =cpu->code_size-1;
				while(j>=0){
					if(cpu->PM[j] == func_offset){ //check if the node is in main
						//add to stack
						sp_top--;
						while(cpu->PM[j-1] != NODE_BEGIN_FLAG){
							stack[sp_top]=cpu->PM[j];
							sp_top--;j--;
						}
						stack[lp] = sp_top; //pointer to node... maybe we can embedded this into the 32 bit if mem size can be more restricted
						stack[lp+1] = lp_entry(cpu->PM[j+3],(new_func_offset+cpu->PM[j]-func_offset)); //needs to be changed to pack size and offset it same
						//printf("MM offset %d\n",new_func_offset+cpu->PM[j]-func_offset);
						lp+=2;j--;
						stack[sp_top] = cpu->PM[j];
						stack[sp_top+stack[sp_top+3]-1] = new_func_offset;
						if(stack[sp_top+4]==code_input){stack[sp_top+1]+=1;}
						j--;
					}else{
						while(cpu->PM[j] != NODE_BEGIN_FLAG){j--;}
						j--;
					}
				}
				lp--;
				largest_offset += func_size+100;

				m = popMessage(expand_buffer);
				int node_to_remap;
				int remap_to_this;
				while(getAddr(m) != OPR && getData(m) != EOM){
					node_to_remap = getAddr(m);

					int lp_t = lp;
					int size;
					int offset;
					int found = 0;
					//now lets see if its dest is in its own stack
					while(lp_t>=0){
							size = getSize(stack[lp_t]);
							offset = getOffset(stack[lp_t]);
							//if true the val is for it
							if(node_to_remap < offset && node_to_remap > offset-size){
								found = 1;
								break;
							}
						lp_t-=2;
					}
					//send or write result
					if(found){
						//printf("CPU %d found an output node\n",cpu_num);
						//printf("offset %d size %d\n",offset,size);
						int ntr_num_dest = stack[stack[lp_t-1]+6+stack[stack[lp_t-1]+5]];
						for(int k=0;k<ntr_num_dest;k++){
							int off = stack[lp_t-1]+7+stack[stack[lp_t-1]+5]+k;
							if( stack[off] == OUTPUT){
								 stack[off] = getData(m);
								 break;
							}
						}
					}
					m = popMessage(expand_buffer);
				}


				m = popMessage(expand_buffer);
				int input_node_offset;
				while(getAddr(m) != OPR && getData(m) != EOM){
					input_node_offset = getAddr(m);

					int lp_t = lp;
					int size;
					int offset;
					int found = 0;
					//now lets see if its dest is in its own stack
					while(lp_t>=0){
							size = getSize(stack[lp_t]);
							offset = getOffset(stack[lp_t]);
							//if true the val is for it
							if(input_node_offset  < offset && input_node_offset  > offset-size){
								found = 1;
								break;
							}
						lp_t-=2;
					}
					//send or write result
					if(found){
						//printf("CPU %d found an input node\n",cpu_num);
						stack[stack[lp_t-1]+2] = getData(m);
						stack[stack[lp_t-1]+1]-=1;
					}
					m = popMessage(expand_buffer);
				}
				pc = stack[ADDRASABLE_SPACE-1];
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

/*void *buffer_func(){
	//add all messages from nuss to a buffer since cpu's are asynchronis
	while(1){
		pthread_mutex_lock(&buss->fifo_lock);
		int buss_size =getFifoSize(buss);
		pthread_mutex_unlock(&buss->fifo_lock);
		if(buss_size>0){
			pthread_mutex_lock(&buss->fifo_lock);
			struct Message *m = peekMessage(buss);
			pthread_mutex_unlock(&buss->fifo_lock);
			pthread_mutex_lock(&buffer->fifo_lock);
			sendMessage(buffer,m);
			pthread_mutex_ulock(&buffer->fifo_lock);
		}
	}

}*/


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
			temp->seen = 0;

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

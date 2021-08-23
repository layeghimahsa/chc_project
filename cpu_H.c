#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include "cpu.h"
#include "2by2sim.h"
#include "many_core.h"


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
	int next_op_2 = 0;
	int add_to_buff = 0;

	cpu->bp = 0;
	cpu->pc = RT; //first instruction is to request task
	cpu->sp = 0;

	int req_made = 0;

	while(1){

		sleep(0.01);
		//printf("CPU %d node num %d pc %d sp %d dep %d req %d\n",cpu_num,cpu->stack[cpu->sp],cpu->pc,cpu->sp,cpu->stack[cpu->sp+1],req_made);


		if(getFifoSize(buss_Mout) > 0 ){
			pthread_mutex_lock(&buss_Mout->fifo_lock);
			struct Message *m = peekMessage(buss_Mout);
			pthread_mutex_unlock(&buss_Mout->fifo_lock);

			  //if the message is for the given cpu
				if(getCpuNum(m) == cpu_num){

					//remove from the buss
					pthread_mutex_lock(&buss_Mout->fifo_lock);
					removeMessage(buss_Mout); //remove the message from the buss
					pthread_mutex_unlock(&buss_Mout->fifo_lock);

					if(add_to_buff==1){
						if(getAddr(m) == OPR && getData(m)==EOM){
							cpu->stack[ADDRASABLE_SPACE-1] = cpu->pc;
							cpu->pc = next_op;
							//next_op = -1;
							add_to_buff = 0;
						}else{
							sendMessage(cpu->buff,Message_packing(0,0,0,getData(m)));
						}
					}else if(getAddr(m) == OPR){
						add_to_buff = 1;
						next_op = getData(m);
					}else{
							cpu->stack[getAddr(m)] = getData(m);
					}
				}
			free(m);
		}

		//now check cpu fifo
		//currently can only get messages when it hass all its code
		if(getFifoSize(cpu->look_up[cpu_num-1]) > 0 && (cpu->pc < -2 || cpu->pc >= 0) && cpu->pc != NVA_C){

			//pop since no one else should be using it
			pthread_mutex_lock(&cpu->look_up[cpu_num-1]->fifo_lock);
			struct Message *m = popMessage(cpu->look_up[cpu_num-1]);
			pthread_mutex_unlock(&cpu->look_up[cpu_num-1]->fifo_lock);

					int cpu_n = getCpuNum(m); //fetch cpu number
					if(cpu_n != cpu_num){ //the message is not for you

						pthread_mutex_lock(&cpu->look_up[cpu_n-1]->fifo_lock);
						if(getAddr(m) == OPR){ //its var request, so pop all five and send at once to maintain order
							//printf("Message transfer Q size %d\n",getFifoSize(cpu->look_up[cpu_num-1]));
							pthread_mutex_lock(&cpu->look_up[cpu_num-1]->fifo_lock);
							sendMessage(cpu->look_up[cpu_n-1],Message_packing(cpu_n,getRW(m),getAddr(m),getData(m)));
							sendMessage(cpu->look_up[cpu_n-1],popMessage(cpu->look_up[cpu_num-1]));
							sendMessage(cpu->look_up[cpu_n-1],popMessage(cpu->look_up[cpu_num-1]));
							sendMessage(cpu->look_up[cpu_n-1],popMessage(cpu->look_up[cpu_num-1]));
							sendMessage(cpu->look_up[cpu_n-1],popMessage(cpu->look_up[cpu_num-1]));
							pthread_mutex_unlock(&cpu->look_up[cpu_num-1]->fifo_lock);
						}else{
							sendMessage(cpu->look_up[cpu_n-1],Message_packing(cpu_n,getRW(m),getAddr(m),getData(m)));
						}
						pthread_mutex_unlock(&cpu->look_up[cpu_n-1]->fifo_lock);
					}else{

						//printf("CPU %d got message [%d][%d][%d][%d]\n",cpu_num,cpu_n,getRW(m),getAddr(m),getData(m));
						//printf("queue size %d\n",getFifoSize(cpu->look_up[cpu_num-1]));


						//this assumes data is sent in order and that only one cpu can write their full message to a queue at a time
						if(getAddr(m) == OPR_V){
								//printf("CPU %d adding %d to buff_2\n",cpu_num,getData(m));
								sendMessage(cpu->buff_2,Message_packing(cpu_n,getRW(m),getAddr(m),getData(m)));
						}else if(getAddr(m) == OPR){
							if(getAddr(m) == OPR && getData(m)==EOM){
								cpu->stack[ADDRASABLE_SPACE-2] = cpu->pc;
								cpu->pc = next_op_2;
							}else{
								next_op_2 = getData(m);
							}
						}else{
							//printf("CPU %d got message [%d][%d][%d][%d]\n",cpu_num,cpu_n,getRW(m),getAddr(m),getData(m));
							if(cpu->stack[cpu->sp+4] == code_input){
								cpu->stack[cpu->sp+2] = getData(m);
								cpu->stack[cpu->sp+4] = code_identity;
							}else{
								cpu->stack[cpu->sp+getAddr(m)] = getData(m);
							}
							if(cpu->stack[cpu->sp+1]<0){exit(0);}
							cpu->stack[cpu->sp+1]-=1;
							//printf("CPU %d has %d dep left\n",cpu_num,cpu->stack[cpu->sp+1]);
						}
					}
			free(m);
		}

		//now for the atual processing
		switch(cpu->pc){
			//request task
			case RT:
			{
				pthread_mutex_lock(&buss_Min->fifo_lock);
				sendMessage(buss_Min,Message_packing(cpu_num,1,OPR,REQ_TASK));
				pthread_mutex_unlock(&buss_Min->fifo_lock);
				cpu->pc=WFC;
				break;
			}
			case SP:
			{
				cpu->sp = getData(popMessage(cpu->buff));
				//printf("CPU %d SP is %d\n",cpu_num,cpu->sp);
				cpu->pc = WTS;
				break;
			}
			case SDR:
			{
					//this sends all the data requests needed
					for(int i = cpu->stack[cpu->sp+1]-1; i>=0; i--){
						//calculate the offset for the curent dependable request
						int offset = cpu->stack[cpu->sp+3]+(3*i);
						//gather data to send
						int cpu_dest = cpu->stack[cpu->sp+offset+1];
						int node_need = cpu->stack[cpu->sp+offset];
						int for_node = cpu->stack[cpu->sp+offset+2];
						//send request
						//printf("CPU %d sending request [%d][%d][%d] to %d\n",cpu_num,node_need,cpu_num,for_node,cpu_dest);
						pthread_mutex_lock(&cpu->look_up[cpu_dest-1]->fifo_lock);
						sendMessage(cpu->look_up[cpu_dest-1],Message_packing(cpu_dest,1,OPR,NVA_C));
						sendMessage(cpu->look_up[cpu_dest-1],Message_packing(cpu_dest,1,OPR_V,node_need));
						sendMessage(cpu->look_up[cpu_dest-1],Message_packing(cpu_dest,1,OPR_V,cpu_num));
						sendMessage(cpu->look_up[cpu_dest-1],Message_packing(cpu_dest,1,OPR_V,for_node));
						sendMessage(cpu->look_up[cpu_dest-1],Message_packing(cpu_dest,1,OPR,EOM));
						pthread_mutex_unlock(&cpu->look_up[cpu_dest-1]->fifo_lock);

						req_made++;
						//printf("queue %d size %d\n",cpu_dest,getFifoSize(cpu->look_up[cpu_dest-1]));
					}
				cpu->pc = WTS;
			}
			//wait for dependables to start
			case WTS:
			{
				//printf("CPU %d WTS\n",cpu_num);
				if(cpu->stack[cpu->sp+1]<0){
					for(int i = 0; i < 30; i++){
						printf("[%d] : [%d]\n",i,cpu->stack[i]);
					}
					exit(0);
				}
				if(cpu->stack[cpu->sp+1] == 0){
					//printf("CPU %d going to decode\n",cpu_num);
					cpu->pc = DEC;
				}else{
					//printf("CPU %d has %d dependables left\n",cpu_num,cpu->stack[cpu->sp+1]);
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
					if(cpu->stack[cpu->sp+6+cpu->stack[cpu->sp+5]] == 0){
						cpu->pc = CS;
					}else{
						int todo = 6+cpu->stack[cpu->sp+5]+(cpu->stack[cpu->sp+6+cpu->stack[cpu->sp+5]]*3);
						if(MESSAGE == 1)
							printf("CPU %d Dest offset address is %d\n",cpu_num,todo);

						if(cpu->stack[cpu->sp+todo-2] == UNKNOWN){
							cpu->pc=SAVE_RES;
							if(MESSAGE == 1)
								printf("CPU %d saving val %d for node %d\n",cpu_num,cpu->stack[cpu->sp],cpu->stack[cpu->sp+todo-1]);
						}else if(cpu->stack[cpu->sp+todo-2] == IGNORE){
							cpu->stack[cpu->sp+6+cpu->stack[cpu->sp+5]] -= 1;
						}
						else if(cpu->stack[cpu->sp+todo-2] == OUTPUT){
							printf("CPU %d OUTPUT: %d\n",cpu_num,cpu->stack[cpu->sp+2]);
							//printf("CPU %d outputing address [%d] val [%d]\n",cpu_num,cpu->stack[todo],cpu->stack[cpu->sp+2]);
							pthread_mutex_lock(&buss_Min->fifo_lock);
							sendMessage(buss_Min,Message_packing(cpu_num,1,cpu->stack[cpu->sp+todo],cpu->stack[cpu->sp+2])); //1 for writing
							pthread_mutex_unlock(&buss_Min->fifo_lock);
							cpu->stack[cpu->sp+6+cpu->stack[cpu->sp+5]] -= 1;
						}
						else{
							   cpu->pc=SEND_RES;
								 if(MESSAGE == 1)
								 		printf("CPU %d sending result %d to cpu %d address %d\n",cpu_num,cpu->stack[cpu->sp+2],cpu->stack[cpu->sp+todo-2],cpu->stack[cpu->sp+todo]);
						}
					}
					break;
				}
			case SEND_RES:
				{
					int todo = 6+cpu->stack[cpu->sp+5]+(cpu->stack[cpu->sp+6+cpu->stack[cpu->sp+5]]*3);
					if(MESSAGE == 1)
						printf("CPU %d sending result %d to cpu %d for node %d\n",cpu_num,cpu->stack[cpu->sp+2],cpu->stack[cpu->sp+todo-2],cpu->stack[cpu->sp+todo-1]);
					pthread_mutex_lock(&cpu->look_up[cpu->stack[cpu->sp+todo-2]-1]->fifo_lock);
					sendMessage(cpu->look_up[cpu->stack[cpu->sp+todo-2]-1],Message_packing(cpu->stack[cpu->sp+todo-2],1,cpu->stack[cpu->sp+todo],cpu->stack[cpu->sp+2])); //1 for writing
					pthread_mutex_unlock(&cpu->look_up[cpu->stack[cpu->sp+todo-2]-1]->fifo_lock);

					cpu->pc = FND;
					cpu->stack[cpu->sp+6+cpu->stack[cpu->sp+5]] -= 1;
					break;
				}
			case SAVE_RES:
			{
				//should save node_num, value and offset_address
				//int todo = cpu->stack[6+cpu->stack[5]]*3;
				int todo = 6+cpu->stack[cpu->sp+5]+(cpu->stack[cpu->sp+6+cpu->stack[cpu->sp+5]]*3);
				for(int i = 0; i<LS_SIZE; i++){
					if(cpu->local_mem[0][i] == UNDEFINED){
						cpu->local_mem[0][i] = cpu->stack[cpu->sp]; //node_num
						cpu->local_mem[1][i] = cpu->stack[cpu->sp+todo]; //offset_address
						cpu->local_mem[2][i] = cpu->stack[cpu->sp+2]; //node value
						cpu->local_mem[3][i] = cpu->stack[cpu->sp+todo-1]; //node dest
						cpu->local_mem[4][i] = 0; //unused
						break;
					}
				}
				cpu->pc = FND;
				cpu->stack[cpu->sp+6+cpu->stack[cpu->sp+5]] -= 1;
				break;
			}
			case code_input:
			{
				printf("\t<< "); scanf("%d",cpu->stack[cpu->sp+2]);
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
				if((cpu->stack[cpu->sp+6] != 0))
				{
					(cpu->stack[cpu->sp+2] = cpu->stack[cpu->sp+7]);
					pthread_mutex_lock(&buss_Min->fifo_lock);
					//mark_as_dead(cpu->stack[sp]);
					sendMessage(buss_Min,Message_packing(cpu_num,1,OPR,MD));
					sendMessage(buss_Min,Message_packing(cpu_num,1,0,cpu->stack[cpu->sp]));
					pthread_mutex_unlock(&buss_Min->fifo_lock);
				}
				else
				{
					cpu->stack[cpu->sp+2] = 0;
					pthread_mutex_lock(&buss_Min->fifo_lock);
					sendMessage(buss_Min,Message_packing(cpu_num,1,OPR,PD));
					sendMessage(buss_Min,Message_packing(cpu_num,1,0,cpu->stack[cpu->sp]));
					pthread_mutex_unlock(&buss_Min->fifo_lock);
				}
				cpu->pc = FND;
				break;
			}
			case code_else:
			{
				if(cpu->stack[cpu->sp+6] == 0)
				{
					(cpu->stack[cpu->sp+2] = cpu->stack[cpu->sp+7]);
					pthread_mutex_lock(&buss_Min->fifo_lock);
					sendMessage(buss_Min,Message_packing(cpu_num,1,OPR,MD));
					sendMessage(buss_Min,Message_packing(cpu_num,1,0,cpu->stack[cpu->sp]));
					pthread_mutex_unlock(&buss_Min->fifo_lock);
				}
				else
				{
					cpu->stack[cpu->sp+2] = 0;
					pthread_mutex_lock(&buss_Min->fifo_lock);
					sendMessage(buss_Min,Message_packing(cpu_num,1,OPR,PD));
					sendMessage(buss_Min,Message_packing(cpu_num,1,0,cpu->stack[cpu->sp]));
					pthread_mutex_unlock(&buss_Min->fifo_lock);
				}
				cpu->pc = FND;
				break;
			}
			case code_merge:
			{
				cpu->stack[cpu->sp+2] = (cpu->stack[cpu->sp+ 6] | cpu->stack[cpu->sp+7]);
				cpu->pc = FND;
				break;
			}
			case code_identity:
			{
				if(cpu->stack[cpu->sp+2] == NAV){cpu->stack[cpu->sp+2] = cpu->stack[cpu->sp+6];}
				cpu->pc = FND;
				break;
			}
			//changing a destination address or sending a var needed to another cpu
			case NVA:
				{
						//if(MESSAGE == 1)


						int node_needed = getData(popMessage(cpu->buff));
						int cpu_dest = getData(popMessage(cpu->buff));
						int node_dest = getData(popMessage(cpu->buff));

						//printf("CPU %d got var request from master for cpu %d\n",cpu_num,cpu_dest);

						if(node_needed == cpu->stack[cpu->sp]){ //if current node
							if(cpu->stack[ADDRASABLE_SPACE-1] < -8){
								if(MESSAGE == 1)
									printf("CPU %d sending var\n",cpu_num);
								for(int i = 6+cpu->stack[cpu->sp+5]+2; i < cpu->stack[cpu->sp+3];i+=3){
									if(cpu->stack[cpu->sp+i]==node_dest){
										pthread_mutex_lock(&cpu->look_up[cpu_dest-1]->fifo_lock);
										sendMessage(cpu->look_up[cpu_dest-1],Message_packing(cpu_dest,1,cpu->stack[cpu->sp+i+1],cpu->stack[cpu->sp+2])); //1 for writing
										pthread_mutex_unlock(&cpu->look_up[cpu_dest-1]->fifo_lock);
										break;
									}
								}
							}else{
								if(MESSAGE == 1)
									printf("CPU %d changing dest address of node [%d][%d]\n",cpu_num,cpu->stack[cpu->sp],cpu_dest);
								int num_dest = cpu->stack[cpu->sp+6+cpu->stack[cpu->sp+5]];
								int changed = 0;
								//modify correst dest
								for(int i =0; i<num_dest; i++){
									if(cpu->stack[cpu->sp+(6+cpu->stack[cpu->sp+5]+2)+(3*i)] == node_dest){

										cpu->stack[cpu->sp+(6+cpu->stack[cpu->sp+5]+1)+(3*i)] = cpu_dest;
										//printf("\n\n cpu_dest %d\n node_dest %d\n addr %d\n\n",cpu->stack[cpu->sp+(6+cpu->stack[cpu->sp+5]+1)+(3*i)],cpu->stack[cpu->sp+(6+cpu->stack[cpu->sp+5]+2)+(3*i)],cpu->stack[cpu->sp+(6+cpu->stack[cpu->sp+5]+3)+(3*i)]);
										changed = 1;
										break;
									}
								}
								if(changed == 0){
									printf("CPU %d FAILED TO CHANGE VAR DEST\n",cpu_num);
									exit(0);
								}
							}//*/

						}else{

							//first must make sure its not a node on stack yet to be ran
							int t_sp = cpu->sp;
							int in_mem = 1;
						  for(int i = 0; i<=MNNC; i++){
								t_sp = cpu->stack[t_sp-1];
								if(cpu->stack[t_sp] == node_needed){
									in_mem = 0;
									break;
								}
								if(t_sp == 0){break;}
							}

							if(in_mem==1){
								if(MESSAGE == 1)
									printf("CPU %d not unran node... fetching from mem\n",cpu_num);
								//should be in local mem
								int found = 0;
								for(int i = 0; i<LS_SIZE; i++){
									if(cpu->local_mem[0][i] == node_needed && cpu->local_mem[3][i] == node_dest){
											pthread_mutex_lock(&cpu->look_up[cpu_dest-1]->fifo_lock);
											sendMessage(cpu->look_up[cpu_dest-1],Message_packing(cpu_dest,1,cpu->local_mem[1][i],cpu->local_mem[2][i])); //1 for writing
											pthread_mutex_unlock(&cpu->look_up[cpu_dest-1]->fifo_lock);
											cpu->local_mem[0][i] = UNDEFINED;
											found =1;
											break;
									}
								}
								if(found == 0){
									printf("CPU %d FAILED TO FIND MEM %d for %d for %d\n",cpu_num,node_needed,node_dest,cpu_dest);
									printf("PC: %d\n",cpu->stack[ADDRASABLE_SPACE-1]);
									for(int i = 0; i < 10; i++){
										printf("[%d] : [%d][-][%d][%d][%d]\n",i,cpu->local_mem[0][i],cpu->local_mem[2][i],cpu->local_mem[3][i],cpu->local_mem[4][i]);
									}
									exit(0);
								}
							}else{
								if(MESSAGE == 1)
									printf("\n\nCPU %d changing dest address of unran node [%d][%d]\n\n",cpu_num,cpu->stack[t_sp],cpu_dest);
								int num_dest = cpu->stack[t_sp+6+cpu->stack[t_sp+5]];
								int changed = 0;
								//modify correst dest
								for(int i =0; i<num_dest; i++){
									if(cpu->stack[t_sp+(6+cpu->stack[t_sp+5]+2)+(3*i)] == node_dest){
										//printf("FOUND\n");
										cpu->stack[t_sp+(6+cpu->stack[t_sp+5]+1)+(3*i)] = cpu_dest;
										changed = 1;
										break;
									}
								}
								if(changed == 0){
									printf("CPU %d FAILED TO CHANGE VAR DEST\n",cpu_num);
									exit(0);
								}
							}//*/

						}
					cpu->pc = cpu->stack[ADDRASABLE_SPACE-1];
					if(MESSAGE == 1)
						printf("CPU %d returning to task %d\n",cpu_num,cpu->pc);

					break;
				}
				case NVA_C:
					{

							if(MESSAGE == 1)
								printf("CPU %d got NVA_C var request\n",cpu_num);

							//printf("CPU %d buff_2 size %d\n",cpu_num,getFifoSize(cpu->buff_2));

							int node_needed = getData(popMessage(cpu->buff_2));
							int cpu_dest = getData(popMessage(cpu->buff_2));
							int node_dest = getData(popMessage(cpu->buff_2));

							//printf("CPU %d serving request [%d][%d][%d]\n",cpu_num,node_needed,cpu_dest,node_dest);

							if(node_needed == cpu->stack[cpu->sp]){ //if current node
								if(cpu->stack[ADDRASABLE_SPACE-2] < -8){
									if(MESSAGE == 1)
										printf("CPU %d sending var\n",cpu_num);
									for(int i = 6+cpu->stack[cpu->sp+5]+2; i < cpu->stack[cpu->sp+3];i+=3){
										if(cpu->stack[cpu->sp+i]==node_dest){
											pthread_mutex_lock(&cpu->look_up[cpu_dest-1]->fifo_lock);
											sendMessage(cpu->look_up[cpu_dest-1],Message_packing(cpu_dest,1,cpu->stack[cpu->sp+i+1],cpu->stack[cpu->sp+2])); //1 for writing
											pthread_mutex_unlock(&cpu->look_up[cpu_dest-1]->fifo_lock);
											break;
										}
									}

								}else{
									if(MESSAGE == 1)
										printf("CPU %d changing dest address of node [%d][%d]\n",cpu_num,cpu->stack[cpu->sp],cpu_dest);
										int num_dest = cpu->stack[cpu->sp+6+cpu->stack[cpu->sp+5]];
										int changed = 0;
										//modify correst dest
										for(int i =0; i<num_dest; i++){
											if(cpu->stack[cpu->sp+(6+cpu->stack[cpu->sp+5]+2)+(3*i)] == node_dest){
												cpu->stack[cpu->sp+(6+cpu->stack[cpu->sp+5]+1)+(3*i)] = cpu_dest;
												changed = 1;
												break;
											}
										}
										if(changed == 0){
											printf("CPU %d FAILED TO CHANGE VAR DEST\n",cpu_num);
											exit(0);
										}
									}//*/

							}else{
								//first must make sure its not a node on stack yet to be ran
								int t_sp = cpu->sp;
								int in_mem = 1;
							  for(int i = 0; i<=MNNC; i++){
									t_sp = cpu->stack[t_sp-1];
									if(cpu->stack[t_sp] == node_needed){
										in_mem = 0;
										break;
									}
									if(t_sp == 0){break;}
								}

								if(in_mem==1){
									if(MESSAGE == 1)
										printf("CPU %d fetching from mem\n",cpu_num);
									//should be in local mem
									int found = 0;
									for(int i = 0; i<LS_SIZE; i++){
										if(cpu->local_mem[0][i] == node_needed && cpu->local_mem[3][i] == node_dest){
												pthread_mutex_lock(&cpu->look_up[cpu_dest-1]->fifo_lock);
												sendMessage(cpu->look_up[cpu_dest-1],Message_packing(cpu_dest,1,cpu->local_mem[1][i],cpu->local_mem[2][i])); //1 for writing
												pthread_mutex_unlock(&cpu->look_up[cpu_dest-1]->fifo_lock);
												cpu->local_mem[0][i] = UNDEFINED;
												found =1;
												break;
										}
									}
									if(found == 0){
										printf("NVA_C\n");
										printf("CPU %d FAILED TO FIND MEM %d for %d for %d\n",cpu_num,node_needed,node_dest,cpu_dest);
										printf("PC: %d\n",cpu->stack[ADDRASABLE_SPACE-2]);
										for(int i = 0; i < 10; i++){
											printf("[%d] : [%d][-][%d][%d][%d]\n",i,cpu->local_mem[0][i],cpu->local_mem[2][i],cpu->local_mem[3][i],cpu->local_mem[4][i]);
										}
										exit(0);
									}
								}else{
										//printf("\n\nCPU %d changing dest address of unran node [%d][%d]\n\n",cpu_num,cpu->stack[t_sp],cpu_dest);
									int num_dest = cpu->stack[t_sp+6+cpu->stack[t_sp+5]];
									int changed = 0;
									//modify correst dest
									for(int i =0; i<num_dest; i++){
										if(cpu->stack[t_sp+(6+cpu->stack[t_sp+5]+2)+(3*i)] == node_dest){
											//printf("FOUND\n");
											cpu->stack[t_sp+(6+cpu->stack[t_sp+5]+1)+(3*i)] = cpu_dest;
											changed = 1;
											break;
										}
									}
									if(changed == 0){
										printf("CPU %d FAILED TO CHANGE VAR DEST\n",cpu_num);
										exit(0);
									}
								}//*/
							}

						cpu->pc = cpu->stack[ADDRASABLE_SPACE-2];
						if(MESSAGE == 1)
							printf("CPU %d returning to task %d\n",cpu_num,cpu->pc);
						break;
					}
			case CS:
			{
				pthread_mutex_lock(&buss_Min->fifo_lock);
				sendMessage(buss_Min,Message_packing(cpu_num,1,OPR,MD));
				sendMessage(buss_Min,Message_packing(cpu_num,1,0,cpu->stack[cpu->sp]));
				pthread_mutex_unlock(&buss_Min->fifo_lock);
				req_made = 0;
				if(cpu->sp == 0){
					//clear stack
					for(int i = 0; i<ADDRASABLE_SPACE; i++){
						cpu->stack[i] = UNDEFINED;
					}
					cpu->pc = RT;
				}else{ //still another node
					//printf("CPU %d SP-1 %d\n",cpu_num,cpu->stack[cpu->sp-1]);
					cpu->sp = cpu->stack[cpu->sp-1];
					cpu->pc = SDR;
				}
				break;
			}
			//shouldnt happen
			default:
				printf("CPU %d cpu->pc %d undefined operation\n",cpu_num,cpu->pc);
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

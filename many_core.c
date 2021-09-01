#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include <string.h>

#include "many_core.h"
#include "2by2sim.h"
#include "cpu.h"


//dynamic code array (starts as copy of original code)
int *runtime_code;
//cpu status array
int *cpu_status;
//used to keep track of node numbers
int list_index;
//This is the number of dead nodes (0 destinations) that were removed (needed for node_dest allignment
int nodes_removed;
//the list of all tasks (that have more than 0 destinations)
struct AGP_node *program_APG_node_list;
//main mutex
pthread_mutex_t mem_lock;
//buss master in
struct FIFO *buss_Min;
//buss master out
struct FIFO *buss_Mout;

struct FIFO **buss;

int NUM_CPU;
//FOR OUTPUT DISPLAY
int MESSAGE;
int GRAPH;
struct data_entry **data;
clock_t BEGIN;


//DO NOT REMOVE THE LINE BELLOW!! File may become corrupt if it is (used to write code array in)
//CODE BEGINE//
const int code[] = {//End main:
0x7fffffff,
0x0,
0x1,
0x20,
0xc,
0x0,
0x1,
0x8,
0x7fffffff,
0x0,
0x2,
0x20,
0xc,
0x0,
0x1,
0xc,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0xffffffff
//Start main @(0):
};
int code_size = 26;
int main_addr = 0;
int main_num_nodes = 3;
int dictionary[][3] = {{0,26,3}
};
int num_dict_entries = 1;
//CODE END//
//DO NOT REMOVE THE LINE ABOVE!!//

/**
 * @brief size function
 *
 * This function is called to calculate the size of each node. (how many entries does the node have)
 * @param [in] addr is the beggining of the node.
 * @param [out] size of the node.
 * @return int
 **/
 int size(int addr){
 	//find size
 	int i = addr + 1;
 	int size = 1;
 	while(code[i] != NODE_BEGIN_FLAG && i < code_size){
 		size++;
 		i++;
 	}
 	return size;
 }

/**
 * @brief find_num_node function
 *
 * This function is called to find the number of nodes between two addresses
 * @param [in] begin the offset from top of the stack to start search
 * @param [in] end the offset from top of the stack te end search
 * @param [out] number of nodes between the given offset
 * @return int
 *
int find_num_node(int begin, int end){

	int dest = code_size - (end/4);
	int count = 0;
	//int i = code_size - (begin/4)
	for(int i = begin; i <= dest; i++){
		if(code[i] == NODE_BEGIN_FLAG){
			count++;
		}
	}
	return count;
}

//creates liked list of given subgraph
struct AGP_node *create_list(int start_address){
	//find dict entry
	int num_nodes_to_make;
	int func_size;
	for(int i = 0; i<num_dict_entries; i++){
		if(dictionary[i][0] == start_address){
			num_nodes_to_make = dictionary[i][2];
			func_size = dictionary[i][1];
			break;
		}
	}

	//make that many nodes
	int i = code_size - start_address - func_size;
	int func_top = i;
	int pre_list_index = list_index;//index before adding nodes
	struct AGP_node *return_node = (struct AGP_node *)malloc(sizeof(struct AGP_node));
	struct AGP_node *making = return_node;
	while(num_nodes_to_make > 0){

		making->node_size = size(i);
		making->code_address = i;
		making->depend = NULL;
		making->node_num = list_index;
		list_index+=1;
		making->assigned_cpu = UNDEFINED;
		making->node_func = start_address;
		making->state = ALIVE; //alive
		for(int j=0; j<making->node_size; j++){
			making->code[j] = code[i];
			i++;
		}


		if(making->code[4] != code_expansion){
			making->num_dest = making->code[(6+making->code[5])];
			if(making->num_dest == 0){
				making->dest = NULL;
			}else{ //make destination list

				struct Destination *dest = (struct Destination *)malloc(sizeof(struct Destination));
				struct Destination *temp = dest;

				for(int j = 1; j <= making->num_dest; j++){
					if(making->code[making->node_size-j] == -1){
						temp->node_dest = OUTPUT; //write to mem
					}else{
						temp->node_dest = pre_list_index -1 + find_num_node(func_top,(start_address*4+making->code[making->node_size-j]));
					}
					temp->cpu_dest = UNDEFINED;
					temp->state = REFACTOR;
					if(j+1 > making->num_dest){
						temp->next = NULL;
					}else{
						temp->next = (struct Destination *)malloc(sizeof(struct Destination));
						temp = temp->next;
					}
				}

				//free(temp);
				making->dest = dest;
			}
		}else{
			making->num_dest = making->code[(6+(making->code[5]*2))];

			struct Destination *dest = (struct Destination *)malloc(sizeof(struct Destination));
			struct Destination *temp = dest;

			for(int j = 1; j <= making->num_dest; j++){
				if(making->code[making->node_size-j] == -1){
					temp->node_dest = OUTPUT; //write to mem
				}else{
					temp->node_dest = pre_list_index -1 + find_num_node(func_top,(start_address*4+making->code[(8+(making->code[5]*2)+((j-1)*3))]));
					//printf("NODE DEST %d",temp->node_dest);
				}
				temp->cpu_dest = UNDEFINED;
				temp->state = REFACTOR;
				if(j+1 > making->num_dest){
					temp->next = NULL;
				}else{
					temp->next = (struct Destination *)malloc(sizeof(struct Destination));
					temp = temp->next;
				}
			}

			//free(temp);
			making->dest = dest;

		}
		num_nodes_to_make--;
		if(num_nodes_to_make == 0){making->next = NULL;}
		else{
			making->next = (struct AGP_node *)malloc(sizeof(struct AGP_node));
			making = making->next;
		}
	}

	create_links(return_node);
	return return_node;
}

//this will create all destination and dependables links of the newest segment in the list
void create_links(struct AGP_node *in){
	if(MESSAGE)
		printf("\nCREATING LINKS\n\n");

	struct AGP_node *trav = in;

	//do all destination links
	while(trav != NULL){
		struct Destination *dest = trav->dest;
		while(dest != NULL){
				if(dest->node_dest != OUTPUT){
					struct AGP_node *find = in;
					while(find != NULL){
						if(find->node_num == dest->node_dest){
							dest->destination = find;
							goto NEX;
						}
						find = find->next;
					}
				}
			NEX:
			dest = dest->next;
		}
		trav = trav->next;
	}
	//create and do all dependent links
	trav = in;
	while(trav != NULL){
		struct Destination *dest = trav->dest;
		while(dest != NULL){
			if(dest->node_dest != OUTPUT){
				struct AGP_node *add_dep = dest->destination;
				struct Dependables *dep_t = add_dep->depend;
				struct Dependables *dep = (struct Dependables *)malloc(sizeof(struct Dependables));
				dep->node_needed = dest->destination->node_num;
				dep->key = 0;
				dep->dependencie = trav;
				add_dep->depend = dep;
				add_dep->depend->next = dep_t;
			}
			dest = dest->next;
		}
		trav = trav->next;
	}

}

void expansion(struct AGP_node *current){

	if(MESSAGE == 1)
		printf("\n\nEXPANDING\n");

	//1. get the address of subgraph to expand
	int sub_address = current->code[(7+(current->code[1]*2))];

	current->state = DEAD;
	//2. calling the generate_list function using this address
	struct AGP_node *expand_top = create_list(sub_address);
	if(expand_top == NULL){
		printf("\n\nFAILED TO EXPAND GRAPH\n\n");
		exit(0);
	}

	//3. connect to main list, make program_agp_node top!
	struct AGP_node *traverse = expand_top;
	while(traverse->next != NULL){traverse = traverse->next;}
	traverse->next = program_APG_node_list;
	program_APG_node_list = expand_top;

	//4. refactor the given expansion

	//get sub function info
	int sub_func_size;
	for(int i = 0; i<num_dict_entries; i++){   //get sub function dictionary info
		if(dictionary[i][0] == sub_address){
			sub_func_size = dictionary[i][1];
		}
	}
	int sub_code_pos = code_size - sub_address - sub_func_size;

	struct AGP_node *node_to_change;
	struct AGP_node *node_to_point;
	struct Destination *cur_dest = current->dest;
	for(int i = 0; i < current->num_dest; i++){
		node_to_change = program_APG_node_list;
		//calculate node offset from called function      //9 + number of inputs * 2 (input size) + output num * 3 (output size)
		int ntc = find_num_node(sub_code_pos, (sub_address*4+current->code[(9+(current->code[5]*2)+(i*3))]));
		for(ntc; ntc>1; ntc--){node_to_change = node_to_change->next;}
		node_to_point = cur_dest->destination;

		//create destination node
		struct Destination *dest_node = (struct Destination *)malloc(sizeof(struct Destination));
		dest_node->node_dest = node_to_point->node_num;
		dest_node->cpu_dest = UNDEFINED;
		dest_node->state = DONT_REFACTOR;
		dest_node->destination = node_to_point;

		//do refactor here
		int dest = code_size - 1 - node_to_point->node_func - (current->code[(8+(current->code[5]*2)+(i*3))]/4);
		int count = 0;
		while(code[dest] != NODE_BEGIN_FLAG){
			count++; dest--;
		}
		dest = (node_to_point->node_size - count - 1)*4;
		dest_node->offset = dest;
		if(node_to_change->dest == NULL){
			node_to_change->dest = dest_node;
		}
		else{
			struct Destination *temp = node_to_change->dest;
			node_to_change->dest = dest_node;
			dest_node->next = temp;
		}
		node_to_change->code[node_to_change->node_size - 1 - node_to_change->num_dest] +=1;
		node_to_change->num_dest++;
		node_to_change->node_size++;
		node_to_change->code[node_to_change->node_size-1] = dest;
		//remove write to mem since it was mapped
		struct Destination *temp = node_to_change->dest;
		for(int i = 0; i < node_to_change->num_dest; i++ ){
			if(temp->node_dest == OUTPUT){
				temp->cpu_dest = IGNORE;
				temp->node_dest = IGNORE;
				temp->state = DONT_REFACTOR;
			}
			temp = temp->next;
		}
		cur_dest = cur_dest->next;
	}

	struct Dependables *d = node_to_point->depend;
	while(d != NULL){
		if(d->dependencie->node_num == current->node_num){
			d->node_needed = node_to_change->node_num;
			d->key = 0;
			d->dependencie = node_to_change;
			break;
		}
		d = d->next;
	}

	//CREATING INPUT VARIABLE REQUEST MESSAGE

	int num_args = current->code[5] - 1;
	//for the numbr of arguments there are/are called
	struct AGP_node *inputed_node;
	struct Dependables *dep = current->depend;
	while(num_args >= 0){

		inputed_node = dep->dependencie;
		struct AGP_node *requ_node = program_APG_node_list; //inputed node
		//find node that needs to request
		int ntp  = find_num_node(sub_code_pos,sub_address*4+current->code[7+(2*num_args)]);
		for(ntp; ntp>1; ntp--){requ_node = requ_node->next;}

		//need to create request
		requ_node->depend = (struct Dependables *)malloc(sizeof(struct Dependables));
		requ_node->depend->dependencie = inputed_node;
		requ_node->depend->key = current->node_num;
		requ_node->depend->node_needed = inputed_node->node_num;
		requ_node->depend->cpu_num = inputed_node->assigned_cpu;

		if(MESSAGE == 1)
			printf("node %d will request %d with key %d\n\n", requ_node->node_num,inputed_node->node_num,requ_node->depend->key);

		inputed_node = inputed_node->next;
		num_args--;
		requ_node->code[1]++;
		//requ_node->code[4] = code_identity;
		dep = dep->next;
	}
}

int binary_routing(int row, int start, int end){

	/*
					j ->
	0000 0001 0010 0011	      i 0  1  2  3
	0100 0101 0110 0111	      | 4  5  6  7
	1000 1001 1010 1011	      v 8  9  10 11
	1100 1101 1110 1111		12 13 14 15

	*


	int first_cpu_dest = UNDEFINED;

	/* x = least significant (j)   y = most significant (i)*
	int x_start, y_start;
	int x_end, y_end;

	x_start = start%row; //j
	y_start = start/row; //i
	x_end = end%row; //j
	y_end = end/row; //i

	int routing_x, routing_y;
	routing_x = x_end - x_start;
	routing_y = y_end - y_start;

	struct path *routing = (struct path*)malloc(sizeof(struct path));

	routing->x = routing_x;
	routing->y = routing_y;

	//printf("x: %d ",routing_x);
	//printf("y: %d \n",routing_y);

	//one option could be returning the whole path struct
	//return routing;

	int sign_x, sign_y;
	sign_x = (routing_x >0) ? 1 : -1;
	sign_y = (routing_y >0) ? 1 : -1;

	// the other option would be returning the first cpu, the start cpu can send the result to
	if(routing_x == 0 && routing_y == 0) first_cpu_dest = start;
	else if(routing_x == 0) first_cpu_dest = start + (row * sign_y); //move one in y axis
	else if (routing_y == 0) first_cpu_dest = start + sign_x; //move one in x axis
	else first_cpu_dest = start + sign_x; //all other cases would start transfering the message towards the x axis first.

	if(first_cpu_dest == UNDEFINED){
		printf("ROUTING WAS UNSUCCESSFUL!\n");
		return -1;
	}

	//printf("first cpu dest: %d \n",first_cpu_dest);
	return first_cpu_dest;
}

/**
 * @brief refactor_destinations function
 *
 * This function is called to update or refactor destinations to be matched to node's stack rather than entire code stack
 * @param [in] current is the current node we are evaluating
 * @param [in] top is the top of the AGP nodes' list
 * @return void
 *
void refactor_destinations(struct AGP_node *current, struct AGP_node *top){
	if(current == NULL){
		printf("cant refactor a null node!!!\n");
	}else{
		struct Destination *dest_struct = current->dest; //getting the list of destinations
		for(int i = 1; i<=current->num_dest;i++){
			if(dest_struct->node_dest == OUTPUT){ //return to main mem since there are no dependants
				dest_struct->cpu_dest = OUTPUT; //main mem
			}else if(dest_struct->node_dest == IGNORE){
				dest_struct->cpu_dest = IGNORE;
			}else{
				struct AGP_node *temp = dest_struct->destination;
				//if the destination isnt assigned, the current node must hold the value
				if(temp->assigned_cpu == UNDEFINED || temp->code[4] == code_expansion)
					dest_struct->cpu_dest = UNKNOWN;
				else
					dest_struct->cpu_dest = temp->assigned_cpu;

				//now we must change the satck destination to match the node stack rather than the full code stack
				//this is done even if the cpu isnt assinged yet
				if(dest_struct->state == REFACTOR){

					int dest = code_size - 1 - temp->node_func - current->code[current->node_size-i]/4;
					int count = 0;
					while(code[dest] != NODE_BEGIN_FLAG){
						count++; dest--;
					}
					dest = (temp->node_size - count - 1)*4;
					current->code[current->node_size-i] = dest;
					dest_struct->state = DONT_REFACTOR;
					dest_struct->offset = dest;
				}
			}
			dest_struct = dest_struct->next;
		}
	}
}

int check_dep_unscheduled(struct AGP_node *current){

	struct Dependables *dep = current->depend;
	while(dep != NULL){
		struct AGP_node *trav = dep->dependencie;
		if(trav->assigned_cpu == UNDEFINED && trav->state != DEAD){return 0;}
		else if(trav->code[4] == code_if || trav->code[4] == code_else){
			if(trav->state == DEAD){}
			else{return 0;}
		}else{}
		dep = dep->next;
	}
	return 1;
}

/**
 * @brief schedule_me function
 *
 * This function is called to schedule a new task to be run on the current cpu. (on-demand scheduling)
 * @param [in] cpu_num specifies the cpu which requests for a new task to execute it.
 * @return struct AGP_node a new node that can be executed on the current cpu. it can be a dummy node in case there are no nodes left.
 *
struct AGP_node *schedule_me(int cpu_num){

	//initial while that we will traverse through and try to find the node we want to schedule
	//count number of possible to be scheduled nodes!
	//if count == 0, create dumy cpu (temp->code[1] = 1; seb) struct that has multiple dependencies, set the cpu status cpu_status [cpu_num-1] = IDLE
	//else (we can schedule), pick the first node we run into (later, nodes which SHOULD be ran, e.g. priority or deadlines) , right now FIFO;  check if -1 ->>> it measns it is still unassigned
	//runtime refactor -> change dest to either cpu numebr or temp (means hold the value)
	//check number of dependencies, if 0 return cpu. if has dependables, do they know their destination? if yes, you can return cpu. if No, make a list of those who have your dependables, return
	// structure cpu


	struct AGP_node *current = program_APG_node_list;
	int unode_num = 0; //number of unscheduled nodes
	int count=0;
	/*finding unscheduled nodes and store them into a new list*

	while(current != NULL){
		if(current->state != DEAD){
			if(current->assigned_cpu == UNDEFINED && (current->code[1] == 0 || current->code[4] == code_input)){
				unode_num++;
				goto FOUND;
			}else if(current->assigned_cpu == UNDEFINED){
				int result = check_dep_unscheduled(current);
				if(result == 1){
					unode_num++;
					goto FOUND;
				}
			}
		}
		current = current->next;
	}
	FOUND:


	//if there is no node to be left to be scheduled
	if(unode_num == 0){
		if(MESSAGE == 1)
			printf("no more nodes to assign!! sending CPU %d a dummy node\n",cpu_num);

		struct AGP_node *dummy = (struct AGP_node *)malloc(sizeof(struct AGP_node));
		dummy->assigned_cpu = cpu_num;
		dummy->code[1] = 1;
		dummy->code[4] = -1;
		cpu_status[cpu_num-1] = CPU_IDLE; //there are no nodes left! go to idle mode.

		if(GRAPH){
			struct data_entry *d = data[cpu_num-1];
			while(d->n != NULL){d = d->n;}
			d->n = (struct data_entry *)malloc(sizeof(struct data_entry));
			d = d->n;
			d->y = 0.0;
			clock_t t = clock();
			d->x = ((double)(t - BEGIN)/CLOCKS_PER_SEC);
		}

		return dummy;
	}else{ //there is some unassigned nodes
		if(current->code[4] == code_expansion){
			current->assigned_cpu = 1000;
			expansion(current);
			return schedule_me(cpu_num);

		} else{
			if(current->code[1] == 0){//if the node has no dependent
				current->assigned_cpu = cpu_num;
				refactor_destinations(current, program_APG_node_list);
				cpu_status [cpu_num-1] = CPU_UNAVAILABLE;
				struct AGP_node *return_node = (struct AGP_node *)malloc(sizeof(struct AGP_node));
				//*return_node = *current;
				return_node = current;

				if(GRAPH){
					struct data_entry *d = data[cpu_num-1];
					while(d->n != NULL){d = d->n;}
					d->n = (struct data_entry *)malloc(sizeof(struct data_entry));
					d = d->n;
					d->y = (double)current->node_num;
					clock_t t = clock();
					d->x = ((double)(t - BEGIN)/CLOCKS_PER_SEC);
				}
				if(MESSAGE == 1)
					printf("sending CPU %d node %d\n",cpu_num,return_node->node_num);
				return return_node;
			}else{ //if the node has dependables
				struct Dependables *dep = current->depend;
				while(dep!=NULL){
					struct AGP_node *temp = dep->dependencie;
					dep->cpu_num = temp->assigned_cpu; //cpu that has that variable
					dep->node_needed = temp->node_num; //variable name to be requested
					if(dep->key == 0){dep->key = UNDEFINED;}
					dep = dep->next;
				}
				//return the cpu.
				current->assigned_cpu = cpu_num;
				refactor_destinations(current, program_APG_node_list);
				cpu_status [cpu_num-1] = CPU_UNAVAILABLE;
				//return copy of node, not actual node
				struct AGP_node *return_node = (struct AGP_node *)malloc(sizeof(struct AGP_node));
				//*return_node = *current;
				return_node = current;

				if(GRAPH){
					struct data_entry *d = data[cpu_num-1];
					while(d->n != NULL){d = d->n;}
					d->n = (struct data_entry *)malloc(sizeof(struct data_entry));
					d = d->n;
					d->y = (double)current->node_num;
					clock_t t = clock();
					d->x = ((double)(t - BEGIN)/CLOCKS_PER_SEC);
				}
				if(MESSAGE == 1)
					printf("sending CPU %d node %d\n",cpu_num,return_node->node_num);
				return return_node;
			}
		}
	}
}

void prop_death(struct AGP_node *trav){
	if(trav->code[4] == code_merge){
		if(MESSAGE == 1)
			printf("\nCANT REMOV MERGE NODE %d\n",trav->node_num);
	}else{
			trav->state = DEAD;
			struct Destination *dest = trav->dest;
			while(dest!=NULL){
				prop_death(dest->destination);
				dest = dest->next;
			}
	}
}
void propagate_death(int node_num){

	struct AGP_node *trav = program_APG_node_list;
	while(trav->node_num != node_num){trav = trav->next;}

	if(trav->code[4] == code_merge){
		if(MESSAGE == 1)
			printf("\nCANT REMOV MERGE NODE %d\n",trav->node_num);
	}else{
			trav->state = DEAD;
			struct Destination *dest = trav->dest;
			while(dest!=NULL){
				prop_death(dest->destination);
				dest = dest->next;
			}
	}
}

//mark as dead makes the given node as dead
void mark_as_dead(int node_num){
	struct AGP_node *trav = program_APG_node_list;
	while(trav->node_num != node_num){trav = trav->next;}
	trav->state = DEAD;
	if(MESSAGE == 1)
		printf("\n\nNODE %d MARKED AS DEAD %d\n\n", trav->node_num, trav->state);

}

/**
 * @brief writeMem function
 *
 * This function is called to writing back the result to memory.
 * @param [in] ind the code address for writing back to memory array.
 * @param [in] val the result.
 * @return void
 *
void writeMem(int ind, int val){

	runtime_code[ind] = val;
	if(MESSAGE == 1){
		printf("WRITING BACK TO MEMORY...\n");
		printf("code[%d] = %d\n",ind, runtime_code[ind]);
	}
	printf("OUTPUT: %d\n",val);
}

void nodes_never_ran(){
	printf("\n\nList of nodes that never ran on a core:\n");
	printf("-----------------------------------------\n");
	printf("STATE | NODE # | OPERATION | DESTINATION \n");
	printf("-----------------------------------------\n");
	struct AGP_node *trav = program_APG_node_list;
	while(trav != NULL){
		if(trav->assigned_cpu == UNDEFINED){

			if(trav->state == DEAD)
				printf("DEAD  |");
			else
				printf("ALIVE |");

			int n = trav->node_num;
			if(n<10){printf("  %d     |",trav->node_num);}
			else if(n<100){printf("  %d    |",trav->node_num);}
			else if(n<1000){printf("  %d   |",trav->node_num);}
			else if(n<10000){printf("  %d  |",trav->node_num);}
			else if(n<100000){printf("  %d |",trav->node_num);}
			else{printf("  %d |",trav->node_num);}

			if(trav->code[4] == code_expansion){
				printf(" EXPANSION |");
			}else{
				if(trav->code[4]<10){printf("  OP: %d    |", trav->code[4]);}
				else{printf("  OP: %d   |", trav->code[4]);}

				if(trav->num_dest>0){
					struct Destination *dest = trav->dest;
					for(int i=trav->num_dest; i>0; i--){
							printf("  %d",dest->node_dest);
							dest = dest->next;
					}
				}
			}
			printf("\n");
		}
		trav = trav->next;
	}
}

void print_node_short(){

	printf("\n\nNode List Short From:\n");
	printf("-----------------------------------------\n");
	printf("STATE | NODE # | OPERATION | DESTINATION \n");
	printf("-----------------------------------------\n");
	struct AGP_node *trav = program_APG_node_list;
	while(trav != NULL){

		if(trav->state == DEAD)
			printf("DEAD  |");
		else
			printf("ALIVE |");

		int n = trav->node_num;
		if(n<10){printf("  %d     |",trav->node_num);}
		else if(n<100){printf("  %d    |",trav->node_num);}
		else if(n<1000){printf("  %d   |",trav->node_num);}
		else if(n<10000){printf("  %d  |",trav->node_num);}
		else if(n<100000){printf("  %d |",trav->node_num);}
		else{printf("  %d |",trav->node_num);}

		if(trav->code[4] == code_expansion){
			printf(" EXPANSION |");
		}else{
			if(trav->code[4]<10){printf("     %d     |", trav->code[4]);}
			else{printf("     %d    |", trav->code[4]);}

			if(trav->num_dest>0){
				struct Destination *dest = trav->dest;
				for(int i=trav->num_dest; i>0; i--){
						printf(" %d",dest->node_dest);
						dest = dest->next;
				}
			}
		}
		printf("\n");
		trav = trav->next;
	}
}

/**
 * @brief print_nodes function
 *
 * This function is just a pretty printer which prints all the AGP_nodes
 * @param [in] nodes the pointer to AGP nodes' list
 * @return void
 *
void print_nodes(struct AGP_node *nodes){
	if(nodes == NULL){

	}else{
		printf("\n\nNODE: \n");
		printf(" - CPU assigned: %d\n",nodes->assigned_cpu);
		printf(" - Node number: %d\n",nodes->node_num);
		printf(" - code main mem addr: %d\n", nodes->code_address);
		printf(" - node size: %d\n",nodes->node_size);
		printf(" - number of dest: %d\n",nodes->num_dest);
		printf(" - code:\n");
		for(int i = 0; i< nodes->node_size; i++){
			printf("    code[%d]: %d\n",i,nodes->code[i]);
		}
		if(nodes->dest != NULL){
			struct Destination *temp = nodes->dest;
			for(int i = 0; i < nodes->num_dest; i++){
				printf(" - Destination %d:\n    node dest: %d\n    cpu dest: %d\n",i,temp->node_dest, temp->cpu_dest);
				temp = temp->next;
			}
		}
		print_nodes(nodes->next);
	}
}



void run_sim(){

	//print_node_short();
	struct Message *buffer; //need malloc
	int op = CB; int serving_cpu = 0;
	int terminate_sim = 0;
	while(terminate_sim == 0){

				switch(op){
					case CB:  //check buss
					{
						if(getFifoSize(buss_Min) > 0){

							pthread_mutex_lock(&mem_lock);
							struct Message *m = popMessage(buss_Min);
							pthread_mutex_unlock(&mem_lock);

							if(MESSAGE == 1)
								printf("Buss_Min Message in: [%d][%d][%d]	  [%d]\n",getCpuNum(m),getRW(m),getAddr(m),getData(m));

							if(getAddr(m) == OPR){
								serving_cpu = getCpuNum(m); op = getData(m);
							}else{
								*buffer = *m;
								op = getRW(m);
							}
							free(m);
						}else{

						}
						break;
					}
					case REQ_TASK: //request new task
						{
							struct AGP_node *task = schedule_me(serving_cpu);
							if(MESSAGE == 1)
								printf("SENDING CPU %d A TASK\n",serving_cpu);

							if(serving_cpu <1 || serving_cpu >4 ){
								exit(0);
							}

							if(task->code[4] == -1){
								//printf("sending dummy node task to cpu %d\n",serving_cpu);
								pthread_mutex_lock(&mem_lock);
								sendMessage(buss_Mout,Message_packing(serving_cpu ,1,OPR,IDLE));
								sendMessage(buss_Mout,Message_packing(serving_cpu ,1,OPR,EOM));
								pthread_mutex_unlock(&mem_lock);

								//sending dummy node... should check if all are idle and sim over
								int idle_count = 0;
								for(int i = 0; i<NUM_CPU;i++){
									if(cpu_status[i] == CPU_IDLE)
										idle_count++;
								}
								if(idle_count == NUM_CPU)
									terminate_sim = 1;

							}else{
								if(MESSAGE == 1)
									printf("sending task %d to cpu %d\n",task->node_num,serving_cpu);

								if(task->code[4] == 11){
									task->code[1]++;
									//printf("\n\n dependants %d\n\n",task->code[1]);
								}
								//send requesting cpu their task
								int i;
								pthread_mutex_lock(&mem_lock);
								sendMessage(buss_Mout,Message_packing(serving_cpu ,1,0,task->node_num));
								pthread_mutex_unlock(&mem_lock);
								for( i=1; i<=(6+task->code[5]); i++){
										pthread_mutex_lock(&mem_lock);
										sendMessage(buss_Mout,Message_packing(serving_cpu ,1,i,task->code[i]));
										pthread_mutex_unlock(&mem_lock);

								}
								struct Destination *dest = task->dest;
								int addr;
								for(int j=0; j<task->num_dest; j++){
									if(dest->node_dest == IGNORE){
										addr = -1;
									}else if(dest->node_dest == OUTPUT){
										addr = task->code_address + 2;
									}else{
										addr = dest->destination->node_size - dest->offset/4 - 1;
									}
									if(MESSAGE == 1)
										printf("Dest cpu %d  Dest node %d  offset %d\n",dest->cpu_dest,dest->node_dest,addr);
									pthread_mutex_lock(&mem_lock);
									sendMessage(buss_Mout,Message_packing(serving_cpu ,1,i,dest->cpu_dest));
									sendMessage(buss_Mout,Message_packing(serving_cpu ,1,i+1,dest->node_dest));
									sendMessage(buss_Mout,Message_packing(serving_cpu ,1,i+2,addr));
									pthread_mutex_unlock(&mem_lock);
									i+=3;
									dest = dest->next;
								}
								pthread_mutex_lock(&mem_lock);
								sendMessage(buss_Mout,Message_packing(serving_cpu ,1,3,i));//code size
								pthread_mutex_unlock(&mem_lock);

								pthread_mutex_lock(&mem_lock);
								sendMessage(buss_Mout,Message_packing(serving_cpu ,1,OPR,WTS));
								sendMessage(buss_Mout,Message_packing(serving_cpu ,1,OPR,EOM));
								pthread_mutex_unlock(&mem_lock);

								//send dependent cpus the dest for their task or previous task
								struct Dependables *dep = task->depend;

								while(dep != NULL){
									pthread_mutex_lock(&mem_lock);
									if(dep->cpu_num < 1){
										sendMessage(buss_Mout,Message_packing(serving_cpu ,1,1,task->code[1]-1));
									}else{
										sendMessage(buss_Mout,Message_packing(dep->cpu_num,1,OPR,NVA));
										sendMessage(buss_Mout,Message_packing(dep->cpu_num,1,0,dep->node_needed));
										sendMessage(buss_Mout,Message_packing(dep->cpu_num,1,0,serving_cpu));
										if(dep->key == UNDEFINED){
											sendMessage(buss_Mout,Message_packing(dep->cpu_num,1,1,task->node_num));
										}else{
											sendMessage(buss_Mout,Message_packing(dep->cpu_num,1,1,dep->key));
										}
										sendMessage(buss_Mout,Message_packing(dep->cpu_num ,1,OPR,EOM));
									}
									pthread_mutex_unlock(&mem_lock);
									dep = dep->next;
								}
							}





							op = CB;
							serving_cpu = 0;
							//free(task);
							break;
						}
					case MD:  //mark as dead
						{
							pthread_mutex_lock(&mem_lock);
							struct Message *m = popMessage(buss_Min);
							pthread_mutex_unlock(&mem_lock);
							if(MESSAGE == 1)
								printf("NODE %d MARKED AS DEAD\n",getData(m));
							mark_as_dead(getData(m));
							op = CB;
							free(m);
							break;
						}
					case PD: //propogate death
						{
							pthread_mutex_lock(&mem_lock);
							struct Message *m = popMessage(buss_Min);
							pthread_mutex_unlock(&mem_lock);
							if(MESSAGE == 1)
								printf("NODE %d PROPOGATE DEATH\n",getData(m));
							propagate_death(getData(m));
							op = CB;
							free(m);
							break;
						}
					case READ:
						//runtime_code[getAddr(buffer)] = getData(buffer);
						printf("no reads needed yet\n");
						break;
					case WRITE:
					{
						runtime_code[getAddr(buffer)] = getData(buffer);
						op = CB;
						break;
					}
				}
		}
}
*/

int main(int argc, char **argv)
{
    printf("\n***SIMULATION START***\n\n");
		NUM_CPU;
		MESSAGE = 0;
		GRAPH=0;
		int KG=0;
		int h=0;
		int n = 0;
		int NODE_NUM_MAX = 2;

		int opt;
		while ((opt = getopt(argc, argv, "mhngK:")) != -1) {
             switch (opt) {
             case 'm':
                 MESSAGE = 1;
                 break;
						 case 'h':
 								 h=1;
 								 break;
						 case 'n':
						 		 n = 1;
								 break;
						 case 'g':
						 		 GRAPH = 1;
								 break;
             case 'K':
                 KG = 1;
                 break;
             default: /* '?' */
                 printf("Usage: %s [-m] [-g] [-K] [-h] [-n] num_cpu\n",argv[0]);
                 exit(EXIT_FAILURE);
             }
    }
		if(h==1){
			printf("Usage: ./sim [-m] [-n] [-g] [-K] [-h] num_cpu  (ex: ./sim 4 or ./sim -m 4)\n[-m]: Display all core messages\n[-n]: Display node details\n[-g]: Create graphs and save them to pdf (Requires gnuplot)\n[-K]: Create and display graphs directly in kitty terminal (Requires gnuplot)\n[-h]: Display all options\n\n");
			return 0;
		}
		if (optind >= argc) {
             fprintf(stderr, "Expected argument!\nhint: add -h to see all options\n\n");
             exit(EXIT_FAILURE);
  	}else{
			NUM_CPU = atoi(argv[optind]);
		}

    if(NUM_CPU < 1){
			printf("NUM CPU %d\n",NUM_CPU);
			printf("YOU MUST HAVE AT LEAST 1 CPU\n");
			return 1;
    }

    int row_col = UNDEFINED;

    for (int i = 1; i * i <= NUM_CPU; i++){
        // if (i * i = n)
        if ((NUM_CPU % i == 0) && (NUM_CPU / i == i)) {
            row_col = i;
        }
    }

    if(row_col == UNDEFINED){
    	printf("Only N*N cpu structure is supported! \n");
			return 1;
    }

    if(MESSAGE == 1){
    	printf("CREATING A %dx%d SIMULATION\n",row_col,row_col);
	    printf("CREATING A %dx%d SIMULATION\n",NUM_CPU/2,NUM_CPU/2);
	    printf("\n\nSETTING UP ENVIRONMENT\n\n");
		}

    //create array of thread id
    pthread_t thread_id[NUM_CPU];
    list_index = 1;
    nodes_removed = 0;

		//instantiate FIFOs for all CPUs
    /*struct FIFO *cpu_fifos[NUM_CPU];
    for(int i = 0; i<NUM_CPU; i++){
			cpu_fifos[i] = create_FIFO();
    }

		//initializing cpu queue connections
		int queue_index;
	  for(int i = 0; i<NUM_CPU; i++){
			for(int j = 0; j<NUM_CPU; j++){
				//printf("%d ",queue_index);
				queue_index = binary_routing(NUM_CPU, i, j);
				cpus[i]->look_up[j] = cpu_fifos[queue_index];
			}
	  }*/

		//program_APG_node_list = create_list(main_addr);


		//create cpu struct
    struct CPU_SA *cpus[NUM_CPU];
    for(int i = 0; i<NUM_CPU; i++){
        struct CPU_SA *cpu_t = (struct CPU_SA*) malloc(sizeof(struct CPU_SA));
        cpu_t->cpu_num = i+1;
				cpu_t->main_addr = main_addr;
				cpu_t->num_dict_entries = num_dict_entries;

				cpu_t->dictionary = dictionary;

				cpu_t->code_size = 0;
        cpus[i] = cpu_t;
    }

		/**********************************************************************************************************/
		int num_nodes_to_make = 0;
		for(int i = 0; i<num_dict_entries; i++){
				num_nodes_to_make += dictionary[i][2];
		}

		//using random coloruing for all nodes (including those in subgraph)
		srand(time(NULL)); //resetting the seed to avoid same result.
		printf("total number of nodes: %d\n", num_nodes_to_make);
		int colouring_random[num_nodes_to_make];//holds node allocations to cpus.
		int counter = 0;
		for(int i = 0; i< num_nodes_to_make; i++){
				colouring_random[i] = rand() % NUM_CPU; //random allocation
				//printf("cpu rand[%d]: %d\n", i, colouring_random[i]+1);
				counter++;
		}
		//printf("counter: %d\n", counter);

		int i = 0;
		int node_counter = 0;
		int node_size;
		int j;
		int rand_cpu;
		int s,si;

    int func_offset = dictionary[0][0];
    int dict_ent = 0;
		//num_nodes_to_make--;
		while(num_nodes_to_make != 0){

			rand_cpu = colouring_random[node_counter];
      printf("CPU %d getting node %d\n",rand_cpu+1,node_counter);
			node_size = size(i);
			j = cpus[rand_cpu]->code_size;
			cpus[rand_cpu]->code_size += node_size + 2;
			s=j+4;si=node_size+1;
			cpus[rand_cpu]->PM[j] = code[i]; //this is the new node flag
			cpus[rand_cpu]->PM[j+1] = code_size - i; //this is the MM offset
			j+=2;i++;
			node_size += i;
			for(i; i<node_size-1; i++){
				cpus[rand_cpu]->PM[j] = code[i];
				j++;
			}
      cpus[rand_cpu]->PM[j] = func_offset; //this is the function offset this node is placed at node_size-1
      if(i == (code_size-func_offset) && i>0){
        dict_ent++;
        func_offset = dictionary[dict_ent][0];
      }
			cpus[rand_cpu]->PM[s] = si; //this changes the orgiginal entry of next node to node size
			node_counter++;
			num_nodes_to_make--;
		}
		/**********************************************************************************************************/

		if(MESSAGE == 1)
    	printf("\n\nCREATING NODE LIST\n\n");

    //program_APG_node_list = create_list(main_addr);

/*   printf("\n\nSCHEDULING NODES\n\n");
    for(int i = 0; i<NUM_CPU; i++){
			cpus[i]->node_to_execute = schedule_me(cpus[i]->cpu_num);
    }*/


		if(MESSAGE == 1)
    	printf("\n\nLAUNCHING THREADS!!!\n\n");

			//buss_Min = create_FIFO();
			//buss_Mout = create_FIFO();
    buss = (struct FIFO**) malloc(NUM_CPU*sizeof(struct FIFO*));
    for(int i = 0; i<NUM_CPU;i++){
      buss[i] = create_FIFO();
    }


		BEGIN = clock();

    for(int i = 0; i<NUM_CPU; i++){
				pthread_create(&thread_id[i], NULL, &CPU_SA_start, cpus[i]);
    }//*/

    /***********************/
    /**** Simulation end ***/
    /***********************/

 /*   //wait for all active cpu threads to finish
    int num_cpu_idle = 0;
    while(num_cpu_idle < NUM_CPU){
			num_cpu_idle = 0;
			pthread_mutex_lock(&mem_lock);
			for(int i = 0; i<NUM_CPU; i++){
				if(cpu_status[i] == CPU_IDLE)
	    		num_cpu_idle++;
      }
			pthread_mutex_unlock(&mem_lock);
	//can do other busy work while sim continues '\/('_')\/'
}*/

		///////////////////////////////////////////////////////////
		////////////////// Send message test /////////////////////
		/////////////////////////////////////////////////////////

	//	run_sim();
	while(1){

	}
	/*	int count = 1;
		struct AGP_node *test;
		struct Message *m;
		while(count < 5){
		//	test = schedule_me(100);
			m = Message_packing(count,1,OPR,code_plus);
			sendMessage(buss_Mout,m);
			count++;
		}*/



		///////////////////////////////////////////////////////////
		//////////////// Send message test end////////////////////
		/////////////////////////////////////////////////////////



		//this should prob be in but it causes a seg fault
    for(int i = 0; i<NUM_CPU; i++){
				//pthread_cancel(thread_id[i]); //cancel all threads
				pthread_join(thread_id[i], NULL); //wait for all threads to clean and cancel safely
    }

		printf("\n***SIMULATION COMPLETE***\n\n");

		clock_t finish = clock();
		double elapsed = (double)(finish - BEGIN)/CLOCKS_PER_SEC;

    //pthread_mutex_destroy(&mem_lock);

		printf("TIME ELAPSED: %f\n\n", elapsed);

    //printf("%d AGP nodes created\n",list_index-1);

    return 0;
}

struct FIFO *create_FIFO(){
	struct FIFO *fifo = (struct FIFO*)malloc(sizeof(struct FIFO));
	pthread_mutex_init(&fifo->fifo_lock, NULL);
	fifo->front = fifo->back = NULL;
	fifo->size = 0;
	return fifo;
}
void sendMessageOnBuss(struct Message *m){
  for(int i=0;i<NUM_CPU;i++){
    pthread_mutex_lock(&buss[i]->fifo_lock);
    sendMessage(buss[i],m);
    pthread_mutex_unlock(&buss[i]->fifo_lock);
  }
}
void sendMessage(struct FIFO *fifo, struct Message *m){
	struct Message *new = (struct Message*)malloc(sizeof(struct Message));
	*new = *m;
	if(fifo->back == NULL){
		fifo->front = fifo->back = new;
		fifo->size+=1;
		fifo->message_counter+=1;
	}else{
		fifo->back->next = new;
		fifo->back = fifo->back->next;
		fifo->size+=1;
		fifo->message_counter+=1;
	}
	//printf("message added\n");
}
struct Message *peekMessage(struct FIFO *fifo){
	if(fifo->front == NULL){
		return NULL;
	}
	struct Message *new = (struct Message*)malloc(sizeof(struct Message));
	*new = *fifo->front;
	new->next = NULL;

  fifo->front->seen+=1;
  printf("M SEEN %d\n",fifo->front->seen);
  if(fifo->front->seen == NUM_CPU){
    printf("REMOVING MESSAGE FROM BUSS\n");
    removeMessage(fifo);
  }
	return new;
}
void removeMessage(struct FIFO *fifo){
	if(fifo->front == NULL){
		//return NULL;
		printf("NO MESSAGES TO REMOVE");
	}else{
		struct Message *m = fifo->front;
		fifo->front = fifo->front->next;
		m->next = NULL;

		if(fifo->front == NULL)
			fifo->back = NULL;

		fifo->size-=1;
		free(m);
	}
}
struct Message *popMessage(struct FIFO *fifo){
	if(fifo->front == NULL){
		return NULL;
	}
	struct Message *new = (struct Message*)malloc(sizeof(struct Message));
	*new = *fifo->front;
	new->next = NULL;

	struct Message *remove = fifo->front;
	fifo->front = fifo->front->next;
	remove->next = NULL;
	free(remove);

	if(fifo->front == NULL)
		fifo->back = NULL;

	fifo->size-=1;
	return new;
}

int getFifoSize(struct FIFO *fifo){
	return fifo->size;
}


void GNUPLOT(int NUM_CPU){

    FILE * gnuplotPipe = popen ("gnuplot -persistent", "w");
    int i;

		fprintf(gnuplotPipe, "set title '%s'\n", "NODES RAN ON CORES OVER TIME");
		char filename_format[] = "cpu_%d.txt";
		char filename[sizeof(filename_format) + 3];  // for up to 4 digit numbers

		for (int i = 0; i < NUM_CPU; i++) {
				snprintf(filename, sizeof(filename), filename_format, i);
				FILE * temp = fopen(filename, "w");

				if(!temp){
					printf("cpu_%d file cannot be oppened.\n",i);
					break;  // break the loop
				}

				struct data_entry *d = data[i];
				while(d!=NULL){
					fprintf(temp, "%f %f \n", d->x, d->y);
					d = d->n;
				}

		}

		fprintf(gnuplotPipe, "plot 'cpu_0.txt'");
		for (int i = 1; i < NUM_CPU; i++) {
				snprintf(filename, sizeof(filename), filename_format, i);
				fprintf(gnuplotPipe, ", '%s'",filename);
		}

		fprintf(gnuplotPipe, " \n");
}

/***********************/
    /*** task scheduling ***/
    /***********************/
    /*
	-64:	0x7fffffff      //new node label
	-60:	0x0		//number of dependencies
	-5c:	0x7		//value (const 7)
	-58:	0x20		//end address + 1 (next node in graph)
	-54:	0xc		//operation
	-50:	0x0             //number of arguments
	-4c:	0x1             //expansion or not flag
	-48:	0x8             //result destination
	-44:	0x7fffffff
	-40:	0x0
	-3c:	0x7
	-38:	0x20
	-34:	0xc
	-30:	0x0
	-2c:	0x1
	-28:	0x4
	-24:	0x7fffffff
	-20:	0x2
	-1c:	0xfffffffc
	-18:	0x24
	-14:	0x3
	-10:	0x2
	-c:	0x0
	-8:	0x0
	-4:	0x0
    */

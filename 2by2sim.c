#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include <string.h>

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
0xa4,
0x7fffffff,
0x0,
0x1,
0x20,
0xc,
0x0,
0x1,
0x74,
0x7fffffff,
0x0,
0x1,
0x20,
0xc,
0x0,
0x1,
0x44,
0x7fffffff,
0x0,
0x1,
0x20,
0xc,
0x0,
0x1,
0x14,
0x7fffffff,
0x1,
0xfffffffc,
0x24,
0xc,
0x1,
0x0,
0x1,
0xffffffff,
0x7fffffff,
0x1,
0xfffffffc,
0x24,
0xc,
0x1,
0x0,
0x1,
0xffffffff,
0x7fffffff,
0x1,
0xfffffffc,
0x24,
0xc,
0x1,
0x0,
0x1,
0xffffffff,
0x7fffffff,
0x1,
0xfffffffc,
0x24,
0xc,
0x1,
0x0,
0x1,
0xffffffff,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x850,
0x1,
0x78,
0x134,
0x4a4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x850,
0x1,
0x78,
0x110,
0x4a4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x850,
0x1,
0x78,
0xec,
0x4a4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x850,
0x1,
0x78,
0xc8,
0x4a4,
//Start main @(661):
//End single:
0x7fffffff,
0x0,
0x14,
0x20,
0xc,
0x0,
0x1,
0x810,
0x7fffffff,
0x0,
0xfffffffc,
0x24,
0x1,
0x0,
0x2,
0x434,
0x814,
0x7fffffff,
0x2,
0xfffffffc,
0x80,
0x3,
0x2,
0x0,
0x0,
0x17,
0x14,
0x44,
0x74,
0xa4,
0xd4,
0x104,
0x134,
0x164,
0x194,
0x1c4,
0x1f4,
0x224,
0x254,
0x284,
0x2b4,
0x2e4,
0x314,
0x344,
0x374,
0x3a4,
0x3d4,
0x404,
0x464,
0x7fffffff,
0x1,
0xfffffffc,
0x24,
0xc,
0x1,
0x0,
0x1,
0x488,
0x7fffffff,
0x1,
0xfffffffc,
0x24,
0xc,
0x1,
0x0,
0x1,
0x48c,
0x7fffffff,
0x1,
0xfffffffc,
0x20,
0xc,
0x1,
0x0,
0x0,
0x7fffffff,
0x1,
0xfffffffc,
0x20,
0xc,
0x1,
0x0,
0x0,
0x7fffffff,
0x1,
0xfffffffc,
0x20,
0xc,
0x1,
0x0,
0x0,
0x7fffffff,
0x1,
0xfffffffc,
0x20,
0xc,
0x1,
0x0,
0x0,
0x7fffffff,
0x1,
0xfffffffc,
0x20,
0xc,
0x1,
0x0,
0x0,
0x7fffffff,
0x1,
0xfffffffc,
0x20,
0xc,
0x1,
0x0,
0x0,
0x7fffffff,
0x1,
0xfffffffc,
0x20,
0xc,
0x1,
0x0,
0x0,
0x7fffffff,
0x1,
0xfffffffc,
0x20,
0xc,
0x1,
0x0,
0x0,
0x7fffffff,
0x1,
0xfffffffc,
0x20,
0xc,
0x1,
0x0,
0x0,
0x7fffffff,
0x1,
0xfffffffc,
0x20,
0xc,
0x1,
0x0,
0x0,
0x7fffffff,
0x1,
0xfffffffc,
0x20,
0xc,
0x1,
0x0,
0x0,
0x7fffffff,
0x1,
0xfffffffc,
0x20,
0xc,
0x1,
0x0,
0x0,
0x7fffffff,
0x1,
0xfffffffc,
0x20,
0xc,
0x1,
0x0,
0x0,
0x7fffffff,
0x1,
0xfffffffc,
0x20,
0xc,
0x1,
0x0,
0x0,
0x7fffffff,
0x1,
0xfffffffc,
0x20,
0xc,
0x1,
0x0,
0x0,
0x7fffffff,
0x1,
0xfffffffc,
0x20,
0xc,
0x1,
0x0,
0x0,
0x7fffffff,
0x1,
0xfffffffc,
0x20,
0xc,
0x1,
0x0,
0x0,
0x7fffffff,
0x1,
0xfffffffc,
0x20,
0xc,
0x1,
0x0,
0x0,
0x7fffffff,
0x1,
0xfffffffc,
0x20,
0xc,
0x1,
0x0,
0x0,
0x7fffffff,
0x1,
0xfffffffc,
0x20,
0xc,
0x1,
0x0,
0x0,
0x7fffffff,
0x1,
0xfffffffc,
0x20,
0xc,
0x1,
0x0,
0x0,
0x7fffffff,
0x1,
0xfffffffc,
0x20,
0xc,
0x1,
0x0,
0x0,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0xffffffff,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x794,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x770,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x74c,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x72c,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x70c,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x6ec,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x6cc,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x6ac,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x68c,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x66c,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x64c,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x62c,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x60c,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x5ec,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x5cc,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x5ac,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x58c,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x56c,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x54c,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x52c,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x50c,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x4ec,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x4cc,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x4ac,
0x1b4,
//Start single @(120):
//End fact:
0x7fffffff,
0x0,
0xfffffffc,
0x28,
0x1,
0x0,
0x3,
0xa8,
0xf4,
0x124,
0x7fffffff,
0x1,
0xfffffffc,
0x28,
0xb,
0x2,
0x0,
0x0,
0x1,
0xffffffff,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x8,
0x2,
0x0,
0x0,
0x1,
0x19c,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x9,
0x2,
0x0,
0x0,
0x1,
0x198,
0x7fffffff,
0x2,
0xfffffffc,
0x30,
0x5,
0x2,
0x0,
0x0,
0x3,
0x80,
0x14c,
0x174,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x148,
0x7fffffff,
0x1,
0xfffffffc,
0x24,
0xc,
0x1,
0x0,
0x1,
0xf0,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0xa,
0x2,
0x0,
0x0,
0x1,
0x7c,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x9,
0x2,
0x0,
0x0,
0x1,
0x14,
0x7fffffff,
0x0,
0x0,
0x20,
0xc,
0x0,
0x1,
0x120,
0x7fffffff,
0x0,
0x1,
0x24,
0xc,
0x0,
0x2,
0xa4,
0x170,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0xcc,
0x1b4
//Start fact @(0):
};
int code_size = 777;
int main_addr = 661;
int main_num_nodes = 12;
int dictionary[][3] = {{661,116,12},
{120,541,52},
{0,120,12}
};
int num_dict_entries = 3;
//CODE END//
//DO NOT REMOVE THE LINE ABOVE!!//


/**
 * @brief size function
 *
 * This function is called to calculate the size of each node. (how many entries does the node have)
 * @param [in] addr is the beggining of the node.
 * @param [out] size of the node.
 * @return int
 */
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
 */
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





/*
0x7fffffff, //expansion node start
0x1, //created count, missing 1 input
0xfffffffc,
0x30, //node size
0x0, //operation (zero means expansion)
0x1, // 1 argument
0x0, //value
0x1dc, //address of x in our example (in code array)
0x1, //1 destination
0x0, // address of subgraph to expand
0x38, // address of node in subgraph to remap (address of result in our example)
0x1b4, //remaping to this (address of b in our example)


//i think this was inversed
0x38, //remaping to this (address of b in our example)
0x1b4,// address of node in subgraph to remap (address of result in our example)
*/


//TODO

//5. check variable if unavailable -> we have to block and continiouslt check if it's been updated
//6. find an output node to ramap it (given address of node in subgraph to remap)
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
		dep = dep->next;
	}
}


/*int generate_lookup_table(int row, int start, int end){

	if(end<0) return -1;
	
	if(end == start+1 && end%row = row-1) return -1;
	
	if(end == start-1 && end%row = 0) return -1;
	
	if(end>=row*row) return -1;
	
	return 0;

int binary_routing(int row, int start, int end){
	
	/*
					j ->
	0000 0001 0010 0011	      i 0  1  2  3
	0100 0101 0110 0111	      | 4  5  6  7
	1000 1001 1010 1011	      v 8  9  10 11
	1100 1101 1110 1111		12 13 14 15
	
	*/
	
	
	int first_cpu_dest = UNDEFINED;
	
	/* x = least significant (j)   y = most significant (i)*/
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
 */
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
 */
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
	/*finding unscheduled nodes and store them into a new list*/

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
 */
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
 */
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


int main(int argc, char **argv)
{
    printf("\n***SIMULATION START***\n\n");
		int NUM_CPU;
		MESSAGE = 0;
		GRAPH=0;
		int KG=0;
		int h=0;
		int n = 0;

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

		//printf("FLAGS: m %d | g %d | K %d\n",M,g,KG);
    if(NUM_CPU < 1){
			printf("NUM CPU %d\n",NUM_CPU);
			printf("YOU MUST HAVE AT LEAST 1 CPU\n");
			return 1;
    }

    //create mutex
    if (pthread_mutex_init(&mem_lock, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }


    if(NUM_CPU < 1){
	printf("NODE NUM %d\n",NUM_CPU);
	printf("YOU MUST HAVE AT LEAST 1 CPU\n");
	return 1;
    }

    int row_col = UNDEFINED;

    for (int i = 1; i * i <= NUM_CPU; i++) {

        // if (i * i = n)
        if ((NUM_CPU % i == 0) && (NUM_CPU / i == i)) {
            row_col = i;
        }
    }

    if(row_col == UNDEFINED){
    	printf("Only N*N cpu structure is supported! \n");
	return 1;
    }


    //TODO specifi case for cpu_num == 1, should ask
    //This should happen for NUM_CPU > 1


    if(MESSAGE == 1){
    	printf("CREATING A %dx%d SIMULATION\n",row_col,row_col);
	    printf("CREATING A %dx%d SIMULATION\n",NUM_CPU/2,NUM_CPU/2);
	    printf("\n\nSETTING UP ENVIRONMENT\n\n");
		}

    //create array of thread id
    pthread_t thread_id[NUM_CPU];
    list_index = 1;
    nodes_removed = 0;

    //create status array
    cpu_status = (int *)malloc(sizeof(int) * NUM_CPU);
    for(int i = 0; i<NUM_CPU; i++){
			cpu_status[i] = CPU_AVAILABLE;
    }

    //instantiate queues for all CPUs
    struct Queue *cpu_queues[NUM_CPU];
    for(int i = 0; i<NUM_CPU; i++){
			cpu_queues[i] = createQueue();
    }

    //create cpu struct
    struct CPU *cpus[NUM_CPU];
    for(int i = 0; i<NUM_CPU; i++){
	struct CPU *cpu_t = (struct CPU*)malloc(sizeof(struct CPU));
        cpu_t->cpu_num = i+1;
        cpu_t->look_up = (struct Queue **) malloc(sizeof(struct Queue*) *NUM_CPU);
	cpus[i] = cpu_t;
    }


    int queue_index; 

    /*top right bottom left -> (0,1,2,3)*/
    /*for(int i = 0; i<NUM_CPU; i++){
	queue_index = generate_lookup_table(NUM_CPU, i, i-row_col); //top cpu
	cpus[i]->look_up[0] = (queue_index != -1) ? cpu_queues[i-row_col] : NULL;
	queue_index = generate_lookup_table(NUM_CPU, i, i+1); //right cpu
	cpus[i]->look_up[1] = (queue_index != -1) ? cpu_queues[i+1] : NULL;
	queue_index = generate_lookup_table(NUM_CPU, i, i+row_col); //bottom cpu
	cpus[i]->look_up[2] = (queue_index != -1) ? cpu_queues[i+row_col] : NULL;
	queue_index = generate_lookup_table(NUM_CPU, i, i-1); //left cpu
	cpus[i]->look_up[3] = (queue_index != -1) ? cpu_queues[i-1] : NULL;
    }*/
	

    //int queue_index; 	
    //initializing cpu queue connections
    for(int i = 0; i<NUM_CPU; i++){
	for(int j = 0; j<NUM_CPU; j++){
		//printf("%d ",queue_index);
		queue_index = binary_routing(NUM_CPU, i, j);
		cpus[i]->look_up[j] = cpu_queues[queue_index];
	}

	//puts("\n");
    }


	//data entry array
	if(GRAPH==1){
		data = (struct data_entry **)malloc(sizeof(struct data_entry*) *NUM_CPU);
		for(int i=0;i<NUM_CPU; i++){
			data[i] = (struct data_entry *)malloc(sizeof(struct data_entry));
			data[i]->x = 0.0;
			data[i]->y = 0.0;
		}
	}


    runtime_code = (int *)malloc(sizeof(int) *code_size);
    for(int i = 0; i<code_size; i++){
			if(MESSAGE == 1)
				printf("code[%d]: %d\n", i ,code[i]);
			runtime_code[i] = code[i];
    }

		if(MESSAGE == 1)
    	printf("\n\nCREATING NODE LIST\n\n");

    program_APG_node_list = create_list(main_addr);

///*   printf("\n\nSCHEDULING NODES\n\n");
    for(int i = 0; i<NUM_CPU; i++){
			cpus[i]->node_to_execute = schedule_me(cpus[i]->cpu_num);
    }
		if(MESSAGE == 1)
    	print_nodes(program_APG_node_list);

		if(MESSAGE == 1)
    	printf("\n\nLAUNCHING THREADS!!!\n\n");

		BEGIN = clock();

    for(int i = 0; i<NUM_CPU; i++){
				pthread_create(&(thread_id[cpus[i]->cpu_num-1]), NULL, &CPU_start, cpus[i]);
        if(cpus[i]->node_to_execute->node_num == DUMMY_NODE)
					cpu_status[cpus[i]->cpu_num-1] = CPU_IDLE;
				else
					cpu_status[cpus[i]->cpu_num-1] = CPU_UNAVAILABLE;
    }//*/

    /***********************/
    /**** Simulation end ***/
    /***********************/

 ///*   //wait for all active cpu threads to finish
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
    }

    for(int i = 0; i<NUM_CPU; i++){
				pthread_cancel(thread_id[i]); //cancel all threads
				pthread_join(thread_id[i], NULL); //wait for all threads to clean and cancel safely
    }

		clock_t finish = clock();
		double elapsed = (double)(finish - BEGIN)/CLOCKS_PER_SEC;

    pthread_mutex_destroy(&mem_lock);

		printf("\n***SIMULATION COMPLETE***\n\n");

		printf("TIME ELAPSED: %f\n\n", elapsed);

    printf("%d AGP nodes created\n",list_index-1);

		//print_nodes(program_APG_node_list);
		if(n == 1){
			print_node_short();
			nodes_never_ran();
		}

		if(KG==1){
		}else if(GRAPH==1){
			GNUPLOT(NUM_CPU);
			for(int i = 0; i< NUM_CPU; i++){
				int count = 0;
				struct data_entry *d = data[i];
				while(d != NULL){count++; d=d->n;}
				printf("CPU %d Ran %d nodes\n",i+1,count);
			}
		}

    return 0;
}

void GNUPLOT(int NUM_CPU){

    FILE * gnuplotPipe = popen ("gnuplot -persistent", "w");
    int i;

		fprintf(gnuplotPipe, "set title '%s'\n", "NODES RAN ON CORES OVER TIME");
		char name[32];

		for(int i = 0; i< NUM_CPU; i++){
				sprintf(name, "%d", i+1);
				FILE * temp = fopen(name, "w");
				struct data_entry *d = data[i];
				while(d!=NULL){
					fprintf(temp, "%f %f \n", d->x, d->y);
					d = d->n;
				}
		}
		fprintf(gnuplotPipe, "plot '1'");
		for(int i = 1; i< NUM_CPU; i++){
			sprintf(name, "%d", i+1);
			fprintf(gnuplotPipe, ", '%s'",name);
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

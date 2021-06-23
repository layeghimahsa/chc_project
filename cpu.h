#ifndef CPU_H
#define CPU_H

#define MAX_MEM 1024*64
//#include <stdbool.h>

#define UNDEFINED -1
#define UNKNOWN 0
#define OUTPUT -99

#define code_expansion 	0
#define code_input		1
//#define code_output		2 //output isn't really an operation, it's a destination
#define code_plus 		3
#define code_times		4
#define	code_is_equal	5
#define	code_is_less	6
#define code_is_greater	7
#define	code_if			8 
#define code_else		9 
#define code_minus		10
#define code_merge		11
#define code_identity	12

#define code_output		0xFFFFFFFF	//convenient to have it set to a special value that can be tested at runtime
#define NAV			0xFFFFFFFC
//Dead operator: remove it
#define DEAD 		0xFFFFFFFF
//Can process corresponding operation and send result to destinations (all arguments resolved)
#define READY 		0

#define LOCAL_STORAGE_SIZE 64

#define RESULT 0
#define REQUEST 1

struct Destination{ 

	int cpu_dest;
	int node_dest;

	struct Destination *next;
};

struct Dependables{ //this is a list of all variable a cpu must call upon to get their variable 
	int node_needed; //this is technically the variable name 
	int cpu_num;  //cpu the request must be sent to 
		
	struct Dependables *next;
};

struct CPU{
	int cpu_num; //the actual cpu number

	int local_mem[5][64]; //local variable storage

	struct Queue *look_up[4]; //lookup queue table. 

	struct AGP_node *node_to_execute; //the node that needs to be executed 

};

struct AGP_node{
	int assigned_cpu; //the cpu the node was or is run on. This is important for scheduling, destination refactoring, and keeping track of who has what node results

	int code[64]; //chunk of code
	int node_size; //actual the size of stack/code
	int code_address; //original code address
	int node_num; //current node number
	
	int num_dest; //number of node's destinations

	int node_func; //sub graph its from 

	struct Destination *dest; 	//destination cpu
	struct Dependables *depend; //list of all cpu that contain your dependables and need var request
	struct AGP_node *next;

};

struct Message_capsul{
	int value;
	int dest;
	int addr; 

	int node_num; //variable name!

	int message_type; //result, request...

	struct Message_capsul *next;
};



void *CPU_start();




#endif

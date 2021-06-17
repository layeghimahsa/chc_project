#ifndef CPU_H
#define CPU_H

#define MAX_MEM 1024*64
//#include <stdbool.h>



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
#define max_dependables 30


#define RESULT 0
#define REQUEST 1

struct DEST{ //destination

	int cpu_dest;
	int node_dest;

	struct DEST *next;
};

struct depend{ //this is a list of all variable a cpu must call upon to get their variable 
	int node_num; //this is technically the variable name 
	int cpu_num;  //cpu the request must be sent to 
		
	struct depend *next;
};
struct cpu{

	int code[64]; //chunk of code
	int node_size; //actual the size of stack/code
	int code_address;
	int assigned_cpu; //cpu the node is assinged to or currently being processed on 
	int node_num; //current node number
	
	int num_dest; //number of node's destinations

	struct Queue *look_up[4]; //lookup queue table
	
	struct DEST *dest; 	//destination cpu
	struct depend *dependables; //list of all cpu that contain your dependables and need var request
	struct cpu *next;
};



struct cpu_out{
	int value;
	int dest;
	int addr; 

	int node_num; //variable name!

	int message_type; //result, request...

	struct cpu_out *next;
};



void *CPU_start();
//void execute(struct memory *mem,struct cpu *CPU);
//void fetch_task();




#endif

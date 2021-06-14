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
#define max_expandable 30

struct DEST{ //destination

	int cpu_dest;
	int node_dest;

	struct DEST *next;
};

struct cpu{

	int code[64]; //chunk of code
	int node_size; //actual the size of stack/code
	int code_address;
	int assigned_cpu; //cpu the node is assinged to or currently being processed on 
	
	int num_dest;
	int expandables[max_expandable]; //list of all cpus that have current cpu's expandables
	
	struct DEST *dest; 	//destination cpu
	
	struct cpu *next;
};



struct cpu_out{
	int value;
	int dest;
	int addr; //destination could also be a tuppl, but we need cpu num and its stack destination address
	struct cpu_out *next;
};



void *CPU_start();
//void execute(struct memory *mem,struct cpu *CPU);
//void fetch_task();




#endif

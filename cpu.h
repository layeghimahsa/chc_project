#ifndef CPU_H
#define CPU_H

#define MAX_MEM 1024*64
#include <stdbool.h>



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



int cpu_num = 1; //starts from first cpu


struct cpu{

	int cpu_number;
	int code[64]; //chunk of code
	int node_size; //actual the size of stack/code
	bool has_dependent;
	int dependents_num; //specify the number of dependency if has any

	int assinged_cpu; //cpu the node is assinged to or currently being processed on 
	int cpu_dest; 	//destination cpu
	int dest_node;  //destination in node list (used in allocation)

	//struct cpu *cpu_source;
	//struct cpu *cpu_dest;
	struct cpu *next;
};

struct cpu_out{

	int value;
	int dest;
	int addr; //destination could also be a tuppl, but ulimately we need cpu num and its stack destination address
};

/*struct cpu{
	int assinged_cpu;
	int cpu_dest; 
	
	int dest_node;

	int code[64];
	int node_size;

	struct cpu *next;
};*/


void *CPU_start();
//void execute(struct memory *mem,struct cpu *CPU);
void fetch_task();

#endif

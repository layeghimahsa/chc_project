#ifndef CPU_HW
#define CPU_HW

#define MAX_MEM 1024*64
//#include <stdbool.h>

#define UNDEFINED -1
#define UNKNOWN 0
#define IGNORE -2
#define OUTPUT -99


#define code_expansion 	0
#define code_input		1
#define code_output		2 //output isn't really an operation, it's a destination
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

//#define code_output		0xFFFFFFFF	//convenient to have it set to a special value that can be tested at runtime
#define NAV			0xFFFFFFFC
//Dead operator: remove it
#define ALIVE		1
#define DEAD 		0xFFFFFFFF
#define DONE    2
#define REFACTOR	-3
#define DONT_REFACTOR   -2
//Can process corresponding operation and send result to destinations (all arguments resolved)
#define READY 		0

#define LS_SIZE 1024 //local storage size

#define RESULT 0
#define REQUEST 1
#define INPUT_REQUEST 2
#define ALIVE 1

#define ADDRASABLE_SPACE 256

struct Destination{
	int cpu_dest;
	int node_dest;
	int state;
	struct AGP_node *destination;
	struct Destination *next;
};

struct Dependables{ //this is a list of all variable a cpu must call upon to get their variable
	int node_needed; //this is technically the variable name
	int cpu_num;  //cpu the request must be sent to
	int key; //needed if its a node requesting for a node that isnt technically ment for it (used for inputs of expansions)
	struct AGP_node *dependencie; //direct link to depentent
	struct Dependables *next;
};

struct CPU{
	int cpu_num; //the actual cpu number

	int local_mem[5][LS_SIZE]; //local variable storage

	struct Queue **look_up; //lookup queue table.

	struct AGP_node *node_to_execute; //the node that needs to be executed

};

struct CPU_H{
	int cpu_num; //the actual cpu number
	int local_mem[5][LS_SIZE]; //local variable storage
	int stack[ADDRASABLE_SPACE];
	int bp; //base pointer
	int sp; //stack pointer
	int pc; //program counter
	struct FIFO **look_up; //lookup FIFO table.
	struct AGP_node *node_to_execute; //the node that needs to be executed
};

struct AGP_node{
	int assigned_cpu; //the cpu the node was or is run on. This is important for scheduling, destination refactoring, and keeping track of who has what node results

	int code[64]; //chunk of code
	int node_size; //actual the size of stack/code
	int code_address; //original code address
	int node_num; //current node number

	int num_dest; //number of node's destinations
	int state; //alive or dead if else statements
	int node_func; //sub graph its from

	struct Destination *dest; 	//destination cpu
	struct Dependables *depend; //list of all cpu that contain your dependables and need var request
	struct AGP_node *next;

};

struct Message_capsul{
	int value;
	int dest; // cpu destination
	int addr;
	int node_num; //variable name!
	int message_type; //result, request...

	struct Message_capsul *next;
};


struct Message{
		unsigned int addr;
		int data;

		struct Message *next;
};


void *CPU_start();
void *CPU_H_start();
struct Message*  Message_packing(int cpu_num, int rw, int addr, int data );
void Message_printing(struct Message *message);
void bin_representation(int n);

void *message_listening();

#endif

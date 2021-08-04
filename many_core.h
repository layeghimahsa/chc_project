#ifndef MANY_CORE_H
#define MANY_CORE_H

#define OPR 131071
#define REQ_TASK 3
#define RT 0
#define DEC 1
#define FND 2
#define SAVE_VAL 0
#define SAVE_RES 13
#define SEND_RES 14
#define IDLE 15

#define READ 0
#define WRITE 1
#define CB 2      //check buss
//#define RNT 3
#define MD 4      //mark as dead
#define PD 5      //propagate death
#define EOM 1010  //end of message
#define NVA 15    //new variable address
#define ADDRASABLE_SPACE 64




extern struct FIFO *buss_Min; //buss master input
extern struct FIFO *buss_Mout; //buss master output

struct FIFO{
    int size;
    struct Message *front, *back;
};

struct CPU_H{
	int cpu_num; //the actual cpu number

	int local_mem[5][LS_SIZE]; //local variable storage

	int stack[ADDRASABLE_SPACE];

	int bp; //base pointer
	int sp; //stack pointer
	int pc; //program counter

	struct Queue **look_up; //lookup queue table.

	struct AGP_node *node_to_execute; //the node that needs to be executed

};


struct FIFO *create_FIFO();
void sendMessage(struct FIFO *fifo, struct Message *m);
struct Message *popMessage(struct FIFO *fifo);
struct Message *peekMessage(struct FIFO *fifo);
void removeMessage(struct FIFO *fifo);
void bin_representation(int n);
void message_listening(struct CPU *cpu);
int getCpuNum(struct Message *message);
int getRW(struct Message *message);
int getAddr(struct Message *message);
int getData(struct Message *message);

#endif

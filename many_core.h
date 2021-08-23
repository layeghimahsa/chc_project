#ifndef MANY_CORE_H
#define MANY_CORE_H

#define OPR 131071
#define OPR_V 131070
#define REQ_TASK 3



#define SAVE_VAL 0





#define READ 0
#define WRITE 1
#define SP -1     //set sp
#define WFC -2    //wait for code
#define SDR -3    //send dependable requests
#define WTS -4    //wait to start / waiting for dependables
#define DEC -5    //decode
#define NVA -6    //new variable address
#define NVA_C -7
#define EOM -8  //end of message
#define IDLE -9
#define SEND_RES -10
#define SAVE_RES -11
#define RT -12
#define FND -13
#define CS -14
//#define RNT 3

#define CB -1     //check buss
#define MD 4      //mark as dead
#define PD 5      //propagate death






#define MNNC 3 //max node num per cpu



extern struct FIFO *buss_Min; //buss master input
extern struct FIFO *buss_Mout; //buss master output

struct FIFO{
    pthread_mutex_t fifo_lock;
    int size;
    int message_counter;
    struct Message *front, *back;
};




struct FIFO *create_FIFO();
void sendMessage(struct FIFO *fifo, struct Message *m);
struct Message *popMessage(struct FIFO *fifo);
struct Message *peekMessage(struct FIFO *fifo);
void removeMessage(struct FIFO *fifo);
int getFifoSize(struct FIFO *fifo);

void bin_representation(int n);
int getCpuNum(struct Message *message);
int getRW(struct Message *message);
int getAddr(struct Message *message);
int getData(struct Message *message);

#endif

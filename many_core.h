#ifndef MANY_CORE_H
#define MANY_CORE_H

#define OPR 131071
#define REQ_TASK 3
#define RT -10
#define DEC -4
#define FND -11
#define SAVE_VAL 0
#define SAVE_RES -9
#define SEND_RES -8
#define IDLE -7

#define READ 0
#define WRITE 1
#define CB -1     //check buss
//#define RNT 3
#define MD 4      //mark as dead
#define PD 5      //propagate death
#define EOM -6  //end of message
#define NVA -5    //new variable address
#define WTS -2
#define WFC -3
#define CS -12



extern struct FIFO *buss_Min; //buss master input
extern struct FIFO *buss_Mout; //buss master output

struct FIFO{
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

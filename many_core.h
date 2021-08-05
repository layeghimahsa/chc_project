#ifndef MANY_CORE_H
#define MANY_CORE_H

#define OPR 131071
#define REQ_TASK 3
#define RT -9
#define DEC -3
#define FND -10
#define SAVE_VAL 0
#define SAVE_RES -8
#define SEND_RES -7
#define IDLE -6

#define READ 0
#define WRITE 1
#define CB -1     //check buss
//#define RNT 3
#define MD 4      //mark as dead
#define PD 5      //propagate death
#define EOM -5  //end of message
#define NVA -4    //new variable address
#define WTS -2





extern struct FIFO *buss_Min; //buss master input
extern struct FIFO *buss_Mout; //buss master output

struct FIFO{
    int size;
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

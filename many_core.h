#ifndef MANY_CORE_H
#define MANY_CORE_H

#define OPR 131071
#define REQ_TASK 0
#define RT 0
#define DEC 1
#define FND 2
#define SAVE_VAL 0
#define SAVE_RES 13
#define SEND_RES 14
#define IDLE 15


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
void bin_representation(int n);
int getCpuNum(struct Message *message);
int getRW(struct Message *message);
int getAddr(struct Message *message);
int getData(struct Message *message);

#endif

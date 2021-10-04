#ifndef MANY_CORE_H
#define MANY_CORE_H


extern struct FIFO **buss;//buss master output
extern pthread_mutex_t buss_lock;
extern pthread_t *thread_id;

struct FIFO{
    pthread_mutex_t fifo_lock;
    int size;
    int message_counter;
    struct Message *front, *back;
};

struct Message{
		unsigned int addr;
		int data;
    int seen;//num cpus that have peeked the message
		struct Message *next;
};

struct FIFO *create_FIFO();
void sendMessageOnBuss(int cpu_num,struct Message *m);
void sendMessage(struct FIFO *fifo, struct Message *m);
struct Message *popMessage(struct FIFO *fifo);
struct Message *peekMessage(struct FIFO *fifo);
void removeMessage(struct FIFO *fifo);
int getFifoSize(struct FIFO *fifo);
int getCpuNum(struct Message *message);
int getRW(struct Message *message);
int getAddr(struct Message *message);
int getData(struct Message *message);

#endif

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

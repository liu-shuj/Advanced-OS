#ifndef CQUEUE_H
#define CQUEUE_H
#include<xinu.h>

typedef void* qElement;

struct Queue{
        void** arr;
        int head;
        int tail;
        int size;
        int full;
        int empty;
};

int init_queue(struct Queue* q,int maxsize);
int push_queue(struct Queue* q,void* element);
int pop_queue(struct Queue* q,void** element);
#endif

#include<cqueue.h>

int init_queue(struct Queue** q,int maxsize){
	if(maxsize<=0)
		return -1;
	(*q)->arr=(void**)getmem(maxsize*sizeof(void*));
	(*q)->size=maxsize;
	(*q)->head=0;
	(*q)->tail=0;
	(*q)->full=0;
	(*q)->empty=1;
	return 0;
}

int push_queue(struct Queue* q,void* element)
{
	//printf("%d %d %d %d %d\n",q->head,q->tail,q->full,q->empty,q->size);
	if(q->full)
		return -1;
	q->arr[q->tail]=element;
	q->empty=0;
	q->tail=(q->tail+1)%(q->size);
	if(q->tail==q->head)
		q->full=1;
	return 0;
}

int pop_queue(struct Queue* q,void** element)
{
	//printf("%d %d %d %d %d\n",q->head,q->tail,q->full,q->empty,q->size);
	if(q->empty)
		return -1;
	*element=q->arr[q->head];
	q->head=(q->head+1)%(q->size);
	q->full=0;
	if(q->head==q->tail)
		q->empty=1;
	return 0;
}

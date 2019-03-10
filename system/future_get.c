#include<xinu.h>
#include<future.h>
syscall future_get(future* f, char* value)
{
	struct procent *prptr;
	intmask mask;
	mask = disable();
	if(f==(future*)NULL || f->state==FUTURE_FREE){
		restore(mask);
		return SYSERR;
	}
	if(f->state==FUTURE_EMPTY){
		f->state=FUTURE_WAITING;
		f->pid=currpid;
		prptr=&proctab[currpid];
		prptr->prstate=PR_WAIT;
		resched();
	}
	else if(f->state==FUTURE_WAITING){
		restore(mask);
		return SYSERR;
	}
	if(f->state==FUTURE_FULL){
		memcpy(value,f->value,f->size);
		f->state=FUTURE_EMPTY;
	}
	restore(mask);
	return OK;
}

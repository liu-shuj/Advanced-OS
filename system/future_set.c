#include<xinu.h>
#include<future.h>
syscall future_set(future* f, char* value)
{
	intmask mask;
	mask=disable();
	if(f==(future*)NULL || f->state==FUTURE_FREE){
		restore(mask);
		return SYSERR;
	}
	if(f->state==FUTURE_FULL){
		restore(mask);
		return SYSERR;
	}
	else if(f->state==FUTURE_EMPTY || f->state==FUTURE_WAITING){
		memcpy(f->value,value,f->size);
		f->state=FUTURE_FULL;
		ready(f->pid);
	}
	restore(mask);
	return OK;
}	

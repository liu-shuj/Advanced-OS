#include<xinu.h>
#include<future.h>
#include<cqueue.h>
syscall future_get(future* f, char* value)
{
	struct procent *prptr;
	intmask mask;
	mask = disable();
	qElement wakepid;
	if(f==(future*)NULL || f->state==FUTURE_FREE){
		restore(mask);
		return SYSERR;
	}
	if(f->state==FUTURE_EMPTY){
		if(f->flags==FUTURE_EXCLUSIVE){
			f->state=FUTURE_WAITING;
			f->pid=currpid;
			prptr=&proctab[currpid];
			prptr->prstate=PR_WAIT;
			resched();
		}
		else if(f->flags==FUTURE_SHARED || f->flags==FUTURE_QUEUE){
			f->state=FUTURE_WAITING;
			push_queue(&f->get_queue,(qElement)currpid);
			prptr=&proctab[currpid];
			prptr->prstate=PR_WAIT;
			resched();
		}
	}
	else if(f->state==FUTURE_WAITING){
		if(f->flags==FUTURE_EXCLUSIVE){
			restore(mask);
			return SYSERR;
		}
		else if(f->flags==FUTURE_SHARED || f->flags==FUTURE_QUEUE){
			push_queue(&f->get_queue,(qElement)currpid);
			prptr=&proctab[currpid];
			prptr->prstate=PR_WAIT;
			resched();
		}
	}
	if(f->state==FUTURE_FULL){
		memcpy(value,f->value,f->size);
		if(f->flags==FUTURE_EXCLUSIVE || f->flags==FUTURE_QUEUE){
			f->state=FUTURE_EMPTY;
			if(f->flags==FUTURE_QUEUE){
				if(!(f->set_queue.empty)){
					pop_queue(&f->set_queue,&wakepid);
					ready((pid32)wakepid);
				}
			}
		}
	}
	restore(mask);
	return OK;
}

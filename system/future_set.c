#include<xinu.h>
#include<future.h>
#include<cqueue.h>
syscall future_set(future* f, char* value)
{
	intmask mask;
	mask=disable();
	qElement wakepid;
	struct procent *prptr;
	if(f==(future*)NULL || f->state==FUTURE_FREE){
		restore(mask);
		return SYSERR;
	}
	if(f->state==FUTURE_FULL){
	  if(f->flags==FUTURE_EXCLUSIVE || f->flags==FUTURE_SHARED){
		restore(mask);
		return SYSERR;
   	  }
	  else if(f->flags==FUTURE_QUEUE){
		push_queue(&f->set_queue,(qElement)currpid);
		prptr=&proctab[currpid];
		prptr->prstate=PR_WAIT;
		resched();
		memcpy(f->value,value,f->size);
		f->state=FUTURE_FULL;
	  }
	}
	else if(f->state==FUTURE_EMPTY || f->state==FUTURE_WAITING){
		memcpy(f->value,value,f->size);
		if(f->state==FUTURE_WAITING){
			f->state=FUTURE_FULL;
			if(f->flags==FUTURE_EXCLUSIVE){
				ready(f->pid);
		  	}
		  	else if(f->flags==FUTURE_QUEUE){
				pop_queue(&f->get_queue,&wakepid);
				ready((pid32)wakepid);
		  	}
			else if(f->flags==FUTURE_SHARED){
				while(!(f->get_queue.empty)){
					pop_queue(&f->get_queue,&wakepid);
					ready((pid32)wakepid);
				}
			}
		}
		else if(f->state==FUTURE_EMPTY){
			f->state=FUTURE_FULL;
		}
	}
	restore(mask);
	return OK;
}	

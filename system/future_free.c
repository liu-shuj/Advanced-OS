#include <xinu.h>
#include <future.h>
syscall future_free(future* f)
{
	intmask mask;
	mask=disable();
	f->state=FUTURE_FREE;
	if(freemem(f->value,f->size)==SYSERR){
		restore(mask);
		return SYSERR;
	}
	else{
		restore(mask);
		return OK;
	}
}

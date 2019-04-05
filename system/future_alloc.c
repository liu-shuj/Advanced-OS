#include <xinu.h>
#include <future.h>
#define QSIZE 1024
future* future_alloc(int future_flags, uint size)
{
	future* newfuture=(future*)getmem(sizeof(future));
	if(newfuture==(future*)SYSERR)
		return NULL;
	newfuture->state=FUTURE_EMPTY;
	newfuture->flags=future_flags;
	newfuture->size=size;
	newfuture->value=(char*)getmem(size);
	if(newfuture->value==(char*)SYSERR)
		return NULL;
	newfuture->pid=0;
	if(init_queue(&(newfuture->set_queue),QSIZE)!=0)
                return NULL;
        if(init_queue(&(newfuture->get_queue),QSIZE)!=0)
                return NULL;
	return newfuture;
}

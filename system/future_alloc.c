#include <xinu.h>
#include <future.h>
future* future_alloc(int future_flags, uint size)
{
	future* newfuture=(future*)getmem(sizeof(future));
	if(newfuture==(future*)SYSERR)
		return NULL;
	newfuture->state=FUTURE_EMPTY;
	newfuture->flags=FUTURE_EXCLUSIVE;
	newfuture->size=size;
	newfuture->value=(char*)getmem(size);
	if(newfuture->value==(char*)SYSERR)
		return NULL;
	newfuture->pid=0;
	return newfuture;
}

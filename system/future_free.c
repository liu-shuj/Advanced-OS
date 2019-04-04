#include <xinu.h>
#include <future.h>
syscall future_free(future* f)
{
	intmask mask;
	mask=disable();
	f->state=FUTURE_FREE;
	free_queue(f->set_queue);
	free_queue(f->get_queue);
	freemem(f->value,f->size);
	restore(mask);
	return OK;
}

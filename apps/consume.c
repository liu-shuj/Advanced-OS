#include <xinu.h>
#include <prodcons.h>

void consumer(int count) {
	int32 i;
	for(i=0;i<count;i++){
		wait(produced);
		printf("consumed: %d\n",n);
		signal(consumed);
	}
	return 0;
}


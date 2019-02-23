#include<cqueue.h>
#define MAXNPROC 1024

int stream_proc(int nargs, char* args[]);
struct Stream{
	sid32 mutex;
	struct Queue* q;
};
struct Data{
	int stream_id;
	int timestamp;
	int value;
};

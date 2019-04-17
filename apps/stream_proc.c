#include<stream_proc.h>
#include<tscdf.h>
#include<cqueue.h>
#include<tscdf-input1.h>
#include<future.h>

int init_stream(struct Stream* s,int maxsize)
{
	int status=0;
	s->q=(struct Queue*)getmem(sizeof(struct Queue));
	if(status=init_queue((s->q),maxsize)!=0)
		return status;
	status=(s->mutex=semcreate(1));
	return status;
}

int read_input(struct Data* data)
{
	static int i=0;
	if(i>10) return 0;
	char* a = (char *)stream_input[i++];
	data->stream_id = atoi(a);
	while (*a++ != '\t');
	data->timestamp = atoi(a);
	while (*a++ != '\t');
	data->value = atoi(a);
	return 1;
}

void s_producer(int usefuture, struct Stream** stream_table, future** future_array)
{	
	while(1){
		struct Data* data=(struct Data*)getmem(sizeof(struct Data));
		if(read_input(data)){
			int s_index=data->stream_id;
			struct Stream* s=stream_table[s_index];
			if(usefuture){
				while ((future_set(future_array[s_index], (char *)&data)) == SYSERR);
			}
			else{	
				wait(s->mutex);
				push_queue(s->q,data);
				signal(s->mutex);
			}
		}
		else{
			freemem((char*)data,sizeof(struct Data));
		}
	}
}

int s_consumer(int usefuture, future* f, struct Stream* s,int time_window,int output_time)
{
	struct tscdf* tc=tscdf_init(time_window);
	int counter=0;
	int poped=0;
	while(1){
		sleep(1);
		struct Data* data=NULL;
		if(usefuture){
			future_get(f,(char*)&data);
		}
		else{
			wait(s->mutex);
			pop_queue(s->q,&data);
			signal(s->mutex);
		}
		if(data){
			//printf("%d %d\n",data->timestamp,data->value);
			//printf("%d %d %d %d %d\n",s->q->head,s->q->tail,s->q->full,s->q->empty,s->q->size);
			tscdf_update(tc,data->timestamp,data->value);
			counter++;
			freemem(data,sizeof(struct Data));
			if(counter==output_time)
			{
				counter=0;
				int32* qarray = tscdf_quartiles(tc);

				if(qarray == NULL) {
					kprintf("tscdf_quartiles returned NULL\n");
				}
				int i;
				for(i=0; i < 5; i++) {
				   kprintf("%d ", qarray[i]);
				}
				kprintf("\n");
				  
				freemem((char *)qarray, (6*sizeof(int32)));
			}
		}
	}
}


int stream_proc(int usefuture, int nargs, char* args[])
{
	int num_streams=1;
	int work_queue,time_window,output_time;
	char usage[] = "sage: -s num_streams -w work_queue -t time_window -o output_time\n";
	
	// parse args
	int i;
	char* ch;
	char c;
	if (nargs==0||!(nargs % 2)) {
	  printf("%s", usage);
	  return(-1);
	}
	else {
	  i = nargs - 1;
	  while (i > 0) {
		ch = args[i-1];
		c = *(++ch);
		
		switch(c) {
			case 's':
			  num_streams = atoi(args[i]);
			  break;

			case 'w':
			  work_queue = atoi(args[i]);
			  break;

			case 't':
			  time_window = atoi(args[i]);
			  break;
			  
			case 'o':
			  output_time = atoi(args[i]);
			  break;

			default:
			  printf("%s", usage);
			  return(-1);
		}

		i -= 2;
	  }
	}
	//struct Stream* stream_table[num_streams];
	int id;
	char** names=(char**)getmem(num_streams*sizeof(char*));
	for(id=0;id<num_streams;id++){
		names[id]=(char*)getmem(20*sizeof(char));
		sprintf(names[id],"consumer_%d",id);
	}
	if(usefuture){
		future** farray=(future**)getmem(num_streams*sizeof(future*));
		for(i=0;i<num_streams;i++)
		{
			if((farray[i]=future_alloc(FUTURE_EXCLUSIVE,sizeof(struct Data*)))==(future*)SYSERR){
				printf("Error creating future!");
				return -1;
			}
		}
		for(id=0;id<num_streams;id++)
			resume(create(s_consumer,1024,20,names[id],5,1,farray[id],NULL,time_window,output_time));	
		resume(create(s_producer,1024,20,"s_producer",3,1,NULL,farray));
	}
	else{
		struct Stream** stream_table=(struct Stream**)getmem(num_streams*sizeof(struct Stream*));
		for(i=0;i<num_streams;i++)
		{
			stream_table[i]=(struct Stream*)getmem(sizeof(struct Stream));
			if(init_stream(stream_table[i],20)==-1){
				printf("Error creating stream!");
				return -1;
			}
		}
		resume(create(s_producer, 1024, 20, "s_producer", 3, 0, stream_table, NULL));
		for(id=0;id<num_streams;id++){
			resume(create(s_consumer, 1024, 20, names[id], 5, 0, NULL, stream_table[id],time_window,output_time));
		}
	}
	return 0;
}
	
	

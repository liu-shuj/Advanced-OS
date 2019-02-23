#include<stream_proc.h>
#include<tscdf.h>
#include<cqueue.h>
#include<tscdf-input1.h>


int init_stream(struct Stream* s,int maxsize)
{
	int status=0;
	s->q=(struct Queue*)getmem(sizeof(struct Queue));
	if(status==init_queue(&s->q,maxsize)!=0)
		return status;
	status=(s->mutex=semcreate(1));
	return status;
}

void read_input(struct Data* data)
{
	static int i=0;
	if(i>10) return;
	char* a = (char *)stream_input[i];
	data->stream_id = atoi(a);
	while (*a++ != '\t');
	data->timestamp = atoi(a);
	while (*a++ != '\t');
	data->value = atoi(a);
}

void s_producer(struct Stream** stream_table)
{	
	while(1){
		struct Data* data=(struct Data*)getmem(sizeof(struct Data));
		read_input(data);
		
		int s_index=data->stream_id;
		struct Stream* s=stream_table[s_index];
		wait(s->mutex);
		if(push_queue(s->q,data)!=-1)
		signal(s->mutex);
		printf("producer out of CS\n");
	}
}

int s_consumer(struct Stream* s,int time_window,int output_time)
{
	struct tscdf* tc=tscdf_init(time_window);
	int counter=0;
	while(1){
		struct Data* data=NULL;
		wait(s->mutex);
		pop_queue(s->q,&data);
		signal(s->mutex);
		printf("consumer out of CS\n");
		tscdf_update(tc,data->timestamp,data->value);
		counter++;
		freemem(data,sizeof(struct Data));
		if(counter==output_time)
		{
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


int stream_proc(int nargs, char* args[])
{
	int num_streams=1;
	int work_queue,time_window,output_time;
	char usage[] = "Usage: -s num_streams -w work_queue -t time_window -o output_time\n";
	
	// parse args
	int i;
	char* ch;
	char c;
	if (!(nargs % 2)) {
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
	
	struct Stream* stream_table[num_streams];
	for(i=0;i<num_streams;i++)
	{
		stream_table[i]=(struct Stream*)getmem(sizeof(struct Stream));
		if(init_stream(&(stream_table[i]),2*time_window)==-1){
			printf("Error creating stream!");
			return -1;
		}
	}
	resume(create(s_producer, 1024, 20, "s_producer", 1, stream_table));
	int stream_id;
	for(stream_id=0;stream_id<num_streams;stream_id++)
		resume(create(s_consumer, 1024, 20, "s_consumer", 3, stream_table[i],time_window,output_time));
	return 0;
}
	
	

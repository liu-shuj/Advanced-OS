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
	a = (char *)stream_input[i];
	data->stream_id = atoi(a);
	while (*a++ != '\t');
	data->timestamp = atoi(a);
	while (*a++ != '\t');
	data->value = atoi(a);
}

void producer(struct Stream** stream_table)
{	
	while(1){
		struct Data* data=(struct Data*)getmem(sizeof(struct Data));
		read_input(data);
		
		s_index=data->stream_id;
		struct Stream* s=stream_table[s_index];
		
		wait(s->mutex);
		while(push_queue(s->q,data)==-1);
		signal(s->mutex);
	}
}

int consumer(struct Stream** s,int time_window,int output_time)
{
	struct tscdf* mytscdf=tscdf_init(time_window);
	int counter=0;
	while(1){
		wait(s->mutex);
		struct Data* data=NULL;
		while(pop_queue(s->q,&data)==-1);
		signal(s->mutex);
		tscdf_update(mytscdf,data->timestamp,data->value);
		counter++;
		freemem(data,sizeof(struct Data));
		if(counter==output_time)
		{
			qarray = tscdf_quartiles(tc);

			if(qarray == NULL) {
				kprintf("tscdf_quartiles returned NULL\n");
			}

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
	
	# parse args
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
	
	struct Stream* stream_table[num_streams]={NULL};
	for(int i=0;i<num_streams;i++)
	{
		stream_table[i]=(struct Stream*)getmem(sizeof(struct Stream));
		if(init_stream(&(stream_table[i]),2*time_window)==-1){
			printf("Error creating stream!");
			return -1;
		}
	}
	resume(create(producer, 1024, 20, "producer", 1, stream_table));
	for(int stream_id=0;stream_id<num_streams;stream_id++)
		resume(create(consumer, 1024, 20, "consumer", 3, stream_table[i],time_window,output_time));
	return 0;
}
	
	

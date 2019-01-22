#include <xinu.h>
#include <prodcons.h>

int n=0;                 //Definition for global variable 'n'
/*Now global variable n will be on Heap so it is accessible all the processes i.e. consume and produce*/
sid32 produced, consumed;
shellcmd xsh_prodcons(int nargs, char *args[])
{
  if (nargs == 2 && strncmp(args[1], "--help", 7) == 0) {
    printf("Usage: %s\n\n", args[0]);
    printf("\tprodcons <n>:\tset n to <n>\n");
    printf("\t--help:\tdisplay this help and exit\n");
    return 0;
  }
  if (nargs >= 3) {
    fprintf(stderr, "%s: too many arguments\n", args[0]);
    fprintf(stderr, "Try '%s --help' for more information\n",
      args[0]);
    return 1;
  }
  int count;
  produced=semcreate(0);
  consumed=semcreate(1); 
  if(nargs==1){
    count=2000;
  }
  else{
    count=atoi(args[1]);
  } 
     
  //check args[1] if present assign value to count
          
  //create the process producer and consumer and put them in ready queue.
  //Look at the definations of function create and resume in the system folder for reference.      
  resume(create(producer, 1024, 20, "producer", 1, count));
  resume(create(consumer, 1024, 20, "consumer", 1, count));
  return (0);
} 

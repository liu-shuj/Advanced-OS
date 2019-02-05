
#include <xinu.h>
#include <stdio.h>
#include <prodcons.h>
extern int prodcons(int nargs, char* args[]);
/*------------------------------------------------------------------------
 *  * xsh-run
 *   *------------------------------------------------------------------------
 *    */
shellcmd xsh_run(int nargs, char *args[])
{
        if (nargs == 2 && strncmp(args[1], "--help", 7) == 0) {
                printf("Usage: %s\n\n", args[0]);
                printf("\trun <string>:\tcreate a process with function <string> as entry point.\n");
                printf("\trun list:\tlist supported functions.\n");
                printf("\t--help:\tdisplay this help and exit\n");
                return 0;
        }

	if ((nargs == 1) || (strncmp(args[1], "list", 5) == 0))
    	{
      		printf("prodcons\n");
      		return OK;
    	}

		/* This will go past "run" and pass the function/process name and its
 * 		 * arguments.
 * 		 		 */
    	args++;
    	nargs--;

    	if(strncmp(args[0], "prodcons", 9) == 0) {
		/* create a process with the function as an entry point. */
		resume (create((void *)prodcons, 4096, 20, "prodcons", 2, nargs, args));
   	 }
	else{
		printf("Not supported command!\n");
		return 1;
	}

	return 0;
}

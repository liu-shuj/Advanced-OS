/* xsh_echo.c - xsh_echo */

#include <xinu.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 *  * xsh-hello
 *   *------------------------------------------------------------------------
 *    */
shellcmd xsh_hello(int nargs, char *args[])
{
	int32	i;			/* walks through args array	*/

	if (nargs > 1) {
		printf("Hello %s, Welcome to the world of Xinu!", args[1]);
		}
	printf("\n");

	return 0;
}

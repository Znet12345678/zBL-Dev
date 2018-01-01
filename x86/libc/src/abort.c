#include <stdio.h>
#include <stdlib.h>
void abort(){
	printf("abort()");
	const char *argv = "/fs/init.elf";

	exec("/fs/init.elf",&argv);
}

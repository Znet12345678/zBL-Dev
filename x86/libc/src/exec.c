#include <stdio.h>
#include <stdlib.h>

int exec(const char *path,const char **argv){
	asm("pushal");
	asm("mov $4,%ah");
	asm("movl %0,%%ebx" : :"m"(path));
	asm("movl %0,%%ecx" : :"m"(argv));
	asm("movl $0,%edx");
	asm("int $0x80");
	int ret;
	asm("movl %%eax,%0" : "=m"(ret));
	asm("popal");
	return ret;
}

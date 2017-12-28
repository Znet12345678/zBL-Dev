#include <stdlib.h>
int lseek(int fd,int n,int mode){
	__asm__("pushal");
	__asm__("mov %0,%%ebx" : : "m"(fd));
	__asm__("mov %0,%%ecx" : : "m"(n));
	__asm__("mov %0,%%edx" : : "m"(mode));
	__asm__("mov $4,%ah");
	__asm__("int $0x80");
	int ret;
	__asm__("movl %%eax,%0": "=m"(ret));
	__asm__("popal");
	return ret;
}

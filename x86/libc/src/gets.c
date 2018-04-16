#include <stdlib.h>
#include <stdio.h>
#include <kernel.h>
void gets(char *str){
	asm("pushal");
	asm("xor %eax,%eax");
	asm("mov $18,%ah");
	asm("mov %0,%%ebx" : : "m"(str));
	asm("int $0x80");
	asm("popal");
	printf("\n");
	t_writevals();
}

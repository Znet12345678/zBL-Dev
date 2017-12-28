#include <stdlib.h>
void bzero(void *pntr,unsigned long n){
	for(unsigned long i = 0; i < n;i++)
		*((unsigned char*)pntr + i) = 0;
}

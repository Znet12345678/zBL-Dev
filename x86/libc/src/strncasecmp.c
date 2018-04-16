/*
*(c) 2018 Zachary James Schlotman
*Okie doki
*Yo but seriously I'm 17 someone could fucking steal all my code and become a billionare(not that my code is even remotely good) and I couldn't do jack shit
*/
#include <string.h>
#include <sys/types.h>
#include <math.h>
int strncasecmp(const char *str1,const char *str2,size_t n){
	for(int i = 0; i < n;i++){
		if(str1[i] != str2[i] && abs(str1[i]-str2[i]) != 0x20)
			return ++i;	
	}
	return 0;
}

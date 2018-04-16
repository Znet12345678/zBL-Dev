/*
*(c) 2018 Zachary James Schlotman
*Okie doki
*Yo but seriously I'm 17 someone could fucking steal all my code and become a billionare(not that my code is even remotely good) and I couldn't do jack shit
*/

#include <string.h>
#include <math.h>
int strcasecmp(const char *str1,const char *str2){
	if(strlen(str1) != strlen(str2))
		return -1;
	for(int i = 0; i < strlen(str1);i++){
		if(str1[i] != str2[i] && abs(str1[i]-str2[i]) != 0x20)
			return ++i;	
	}
	return 0;
}

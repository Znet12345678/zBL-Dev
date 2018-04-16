/*
*(c) 2018 Zachary James Schlotman
*Okie doki
*Yo but seriously I'm 17 someone could fucking steal all my code and become a billionare(not that my code is even remotely good) and I couldn't do jack shit
*/
/*
*The comments all over this program(and the code) are what teachers mean when they say don't post stuff you don't want your employers seeing online
*I probably spent more time on the comments than I did the code
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
int contains(const char *srch,const char *key){
	int i = 0;
	if(strlen(key) > strlen(srch))
		return 0;
	while(i < strlen(srch)){
		if(memcmp(srch + i,key,strlen(key)) == 0)
				return 1;
		i++;
	}
	return 0;
}
void ICmntSim(){
	printf("\nWelcome to generic internet site 2018!\n");
	printf("Enter your comment 1024 character limit or you will probably crash the os because 15 year old me thought it was a good idea to allocate kernel memory and user memory the same way(Again this protects against meltdown and spectre)");
	b:;printf(">");
	char *cmnt = malloc(1024);
	gets(cmnt);
	if(contains(cmnt,"gay"))
		printf("No u");
	else if(contains(cmnt,"fuck") || contains(cmnt,"shit") || contains(cmnt,"ass") || contains(cmnt,"bitch") || contains(cmnt,"javascript") || contains(cmnt,"php")){
		printf("Sorry but this is a christain site, so no swearing\n");
	}
	else if(contains(cmnt,"heck") || contains(cmnt,"frick") || contains(cmnt,"double hockey sticks")){
		printf("No hecks, no fricks, and no double hockey sticks\n");
	}
	else if(strcasecmp(cmnt,"exit") == 0)
		return;
	free(cmnt);
	goto b;
}
void randGenFake(){
	a:;printf("\nEnter a random number:");
	char *str = malloc(1024);
	gets(str);
	printf("Your number was:%s\n",str);
	free(str);
	str = malloc(80);
	printf("Try again?Y/N");
	gets(str);
	if(strcasecmp(str,"Y") == 0)
		goto a;
	free(str);
}
void numGuess(){
	printf("\nThink of a number\n");
	/*I don't feel like adding an idt entry for sleep because I am afraid of assembly(bragging a bit but I am the best at it at my school but that's because I'm the only one who knows it because the district thinks java is the future. Boy were they wrong.) but I think i have a sleep function somewhere in the kernel*/
	printf("Enter your number and subtract one from it:");
	char *str = malloc(1024);
	gets(str);
	unsigned int i = atoi(str);
	printf("\nYour number was:%d\n",i+1);
	free(str);
}
int main(int argc,char *argv[]){
	/*I could one line all this shit but that would be a really fucking long line*/
	/*I don't think i've every written so many print statements in one program in forever. This is mind numbing*/
//	printf("I will preface this by saying I am the least professional person posting on this subreddit, so sorry for killing braincells\n");
	printf("\nThe fun of this operating system is I didn't add memory protection. It's not a bug it's a feature(It protects against meltdown)\n"); /*Knowing this, this program will likely cause 50 memory leaks*/
//	printf("/*(c) 2018 Zachary James Schlotman\n*Okie doki\n*Yo but seriously I'm 17 someone could fucking steal all my code and become a billionare(not that my code is even remotely good) and I couldn't do jack shit*/");
//	printf("Yeah that's right I put a comment in my output. What are you going to do about it?\n"); Now it's a comment about printing out a comment
	printf("I just realised I left my name in there but it's on my github anyways.(I am so thankful you can't downvote on github[I hope this doesn't give them any ideas])\n");
	printf("If anyone wants to have a stroke and wants my github ask in the comments\n");
//	printf("Welcome to a demo of the most useful operating system on the planet*obvious sarcasm*\n");
	printf("As a programmer I naturally cannot spell so please ignore the many spelling mistakes I will make while writing this program on a school night\n");
	printf("This is reddit not stack overflow so please don't curse me out in the comments for saying that.\n");
//	printf("Okie doki\n");
	printf("I will pay someone all I have($10) to fucking port gdb to this monstrosity(And you'll get expirience in the worst file system on this planet[The one that I made and forgot the name of.ls | grep -i fs.c returns 8 results and I know only one of these works.]). Oh and you can have a stock in my nonexistant free opensource software company run owned and solely employing me\n");
        printf("Let's see what you want to do I will give you some options.Type everything is exactly as it is written cuz I ain't got time for that. Tbh I don't know if i've even implemented strcasecmp. Might as well check and do that now.\n");
        printf("Internet comment simulator 2018\n");
        printf("Random number generator\n");
        printf("I will guess your number.(I stole this idea from someone else on reddit. Sorry man)\n");

	while(1){
		printf(">");
/*
*I wrote a function somewhere in this mess that finds a symbol by name in the elf file for kern mods and I could name the functions these things and change the strings
*in this program and hardcode this thing into a kernel module and eliminate the if statements but that is work that I don't remember or want to remember how to do.
*/
		char *str = malloc(1024);
		gets(str);
		if(strcasecmp(str,"Internet comment simulator 2018") == 0){
			ICmntSim();
		}
		else if(strcasecmp(str,"Random number generator") == 0)
			randGenFake();/*See if I call it fake no one will take up legal action against me if I continue this as an actual operating system*/
		else if(strcasecmp(str,"I will guess your number") == 0)
			numGuess();
		else if(strcasecmp(str,"help") == 0){
			printf("Internet comment simulator 2018\n");
	        	printf("Random number generator\n");
		        printf("I will guess your number.(I stole this idea from someone else on reddit. Sorry man)\n");

		}
		else{
			printf("Either I'm bad at giving directions(which is likely) or you're bad at following them(equally likely if I'm the one running this) or I made a booboo in the code(most likely)\n");/*$10 to myself that it's in the c library or the kernel and not this*/
		}
		free(str);

	}
}

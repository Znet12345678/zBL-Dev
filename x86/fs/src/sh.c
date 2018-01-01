#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <environment.h>
void setVar(char *name,char *val){
	struct envVar *top = (struct envVar *)0x00900000;
	while(top->nxt != 0)
		top = top->nxt;
	struct envVar *env = malloc(sizeof(*env));
	strcpy(env->name,name);
	strcpy(env->val,val);
	env->nxt = 0;
	top->nxt = env;
}
char *getenv(char *name){
	struct envVar *top = (struct envVar *)0x00900000;
	while(top != 0){
		if(strcmp(top->name,name) == 0)
			return top->val;
		top = top->nxt;
	}
	return 0;
}
int getCIndx(char *str,int c){
	for(int i = 0; i < strlen(str);i++)
		if(str[i] == c)
			return i;
	return -1;
}
int main(int argc,char *argv[]){
	if(argc > 1){
		int fd = open(argv[1],O_RDONLY);
		if(fd < 0)
			return -1;
		char **args = malloc(1024);
		for(int i = 1;i < argc;i++){
			args[i-1] = malloc(strlen(argv[i]));
			strcpy(args[i-1],argv[i]);
		}
		exec(argv[1],args);
		return 0;
	}else{
		printf("Initializing Environment...\n");
		struct envVar *top = (struct envVar *)0x00900000;
		bzero(top,sizeof(*top));
		setVar("PATH","/fs/exec");
		setVar("ELFSUFFIX","1");
		setVar("PWD","/");
		printf("Done\n");
		while(1){
			printf("\n>");
			char *str = malloc(1024);
			gets(str);
			char **args = sep(str,' ');
			int cnt = 0;
			while(args[cnt] != 0 && args[cnt][0] != 0)
				cnt++;
			for(int i = 0; i < cnt;i++){
				if(args[i][0] == '$'){
					char *sv = malloc(strlen(args[i]));
					strcpy(sv,args[i] + 1);
					bzero(args[i],strlen(args[i]));
					strcpy(args[i],sv);
				}
			}
			t_writevals();
			if(getCIndx(args[0],'/') >= 0)
				exec(args[0],args);
			else{
				char **s = sep(getenv("PATH"),':');
				int i = 0;
				while(s[i] != 0){
					char *path = malloc(1024);
					strcpy(path,s[i]);
					strcat(path,"/");
					strcat(path,args[0]);
					strcat(path,".elf");
					if(exec(path,args) >= 0)
						break;
					i++;
				}
				if(s[i] == 0){
					printf("Command not found!\n");
				}
			}
		}
	}
}

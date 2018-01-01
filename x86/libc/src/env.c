#include <stdlib.h>
#include <string.h>
#include <environment.h>
void setenv(char *name,char *val){
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

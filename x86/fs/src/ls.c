#include <lib.h>
int main(int argc,char *argv[]){
	t_readvals();
	kprintf("LS:");
	if(argc < 2){
		list(getenv("PWD"));
		return 1;
	}
	list(argv[1]);
	return 1;
}

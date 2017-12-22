#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include "simpfs.h"
void usage(){/*TODO*/}
int main(int argc,char *argv[]){
	if(argc != 4){
		usage();
		return -1;
	}
	FILE *in = fopen(argv[1],"rb");
	if(!in){
		fprintf(stderr,"Error Opening %s",argv[1]);
		perror("");
		return -1;
	}
	int size = fsize(argv[2],in);
	uint8_t *buf = malloc(size);
	printf("%d\n",size);
	int n = read_file(argv[2],buf,in);
	fclose(in);
	FILE *out = fopen(argv[3],"wb");
	if(!out){
		fprintf(stderr,"Error opening %s",argv[1]);
		perror("");
	}
	fwrite(buf,1,size,out);
	fclose(out);
	return 0;
}

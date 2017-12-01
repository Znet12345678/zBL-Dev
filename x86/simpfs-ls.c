#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include "simpfs.h"
int main(int argc,char *argv[]){
	if(argc < 3)
		return -1;
	FILE *f = fopen(argv[1],"rb");
	if(!f){
		fprintf(stderr,"Failed to open %s",argv[1]);
		perror("");
	}
	for(int i = 2; i < argc;i++){
		printf("LS %s:\n",argv[i]);
		struct __DIR *d = __opendir(argv[i],f);
		if(!d){
			printf("I/O Error\n");
			continue;
		}
		uint8_t *buf = malloc(512);
		_ata_read_master(buf,d->dhdr->nxtTreeLba,f);
		struct tree_ent *ent = malloc(sizeof(*ent));
		struct tree_filehdr *fhdr = malloc(sizeof(*fhdr));
		struct tree_dirhdr *dhdr = malloc(sizeof(*dhdr));
		memcpy(ent,buf,sizeof(*ent));
		while(ent->alloc && ent->nxtLba != 0){
			memcpy(fhdr,buf + sizeof(*ent),sizeof(*fhdr));
			printf("%s\n",fhdr->name,ent->nxtLba);
			_ata_read_master(buf,ent->nxtLba,f);
			memcpy(ent,buf,sizeof(*ent));
		}	
	}
	return 0;
}

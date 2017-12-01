#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include "simpfs.h"
void dent(struct tree_ent *ent){
	printf("ENT:\n[alloc]%d [type]%d [nxtLba]%d\n",ent->alloc,ent->type,ent->nxtLba);
}
void dfhdr(struct tree_filehdr *fhdr){
	printf("FHDR:\n[alloc]%d [namelen]%d [name]%s\n",fhdr->alloc,fhdr->namelen,fhdr->name);
}
void ddhdr(struct tree_dirhdr *dhdr){
	printf("DHDR:\n[nxtTreeLba]%d\n",dhdr->nxtTreeLba);
}
void dfex(struct tree_fexclusive *fex){
	printf("FEX:\n[nxtFLba]%d\n",fex->nxtFLba);
}
void ddir(struct __DIR *d){
	dent(d->ent);
	dfhdr(d->fhdr);
	ddhdr(d->dhdr);
}
int main(int argc,char *argv[]){
	if(argc < 3)
		return -1;
	FILE *f = fopen(argv[1],"rb");
	if(!f){
		fprintf(stderr,"Couldnt open %s",argv[1]);
		perror("");
		return -1;
	}
	for(int i = 2; i < argc;i++){
		struct __DIR *d = __opendir(argv[i],f);
		if(!d){
			printf("I/O Error\n");
			continue;
		}
		ddir(d);
		uint8_t *buf = malloc(512);
		_ata_read_master(buf,d->dhdr->nxtTreeLba,f);
		struct tree_ent *ent = malloc(sizeof(*ent));
		memcpy(ent,buf,sizeof(*ent));
		int prevlba = -1;
		while(ent->alloc){
			dent(ent);

			struct tree_filehdr *fhdr = malloc(sizeof(*fhdr));
			memcpy(fhdr,buf + sizeof(*ent),sizeof(*fhdr));
			dfhdr(fhdr);
			if(ent->type == __TYPE_FILE){
				struct tree_fexclusive *fex = malloc(sizeof(*fex));

				memcpy(fex,buf + sizeof(*ent) + sizeof(*fhdr),sizeof(*fex));
				dfex(fex);
				free(fex);
			}else{
				struct tree_dirhdr *dhdr = malloc(sizeof(*dhdr));
				memcpy(dhdr,buf + sizeof(*fhdr),sizeof(*dhdr));
				ddhdr(dhdr);
				free(dhdr);
			}
			if(ent->nxtLba == 0)
				break;
			prevlba = ent->nxtLba;
			bzero(buf,512);
			free(fhdr);
			_ata_read_master(buf,ent->nxtLba,f);
			memcpy(ent,buf,sizeof(*ent));
		}
	}
}

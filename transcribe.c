#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <dirent.h>
#include <fcntl.h>
#include "simpfs.h"
uint32_t find_free(FILE *f){
	uint32_t ret = 1;
	uint8_t *buf = malloc(512);
	int r = _ata_read_master(buf,ret,f);
	if(r < 512){
		fseek(f,0,SEEK_END);
		return ftell(f)/512;
	}
	while(buf[0]){
		ret++;
		free(buf);
		buf = malloc(512);
		r = _ata_read_master(buf,ret,f);
		if(r < 512){
			fseek(f,0,SEEK_END);
			return ftell(f)/512;
		}
	}
	return ret;
}
int mkfs(FILE *f){
	uint8_t *buf = malloc(512);
	bzero(buf,512);
	buf[0] = 0x7f;
	buf[1] = 'S';
	buf[2] = 'I';
	buf[3] = 'M';
	buf[4] = 'P';
	struct tree_ent *head = malloc(512);
	bzero(head,512);
	head->alloc = 1;
	head->type = __TYPE_DIR;
	head->nxtLba = 1;
	memcpy(buf + 5,head,sizeof(*head));
	_ata_write_master(buf,0,f);

}

int _ata_read_master(void *buf,int lba,FILE * f){
	fseek(f,lba * 512,SEEK_SET);
	int r = fread(buf,sizeof(uint8_t),512,f);
	if(r == 0)
		bzero(buf,512);
	return r;
}
int _ata_write_master(void *buf,int lba,FILE * f){
	fseek(f,lba*512,SEEK_SET);
	int r = fwrite(buf,sizeof(uint8_t),512,f);

	return r;
}
char **sep(const char *str,int c){
	int i = 0,k = 0,l = 0;
	char **ret = (char**)malloc(102400);
	bzero(ret,102400);

	while(str[i] != 0){
		while(str[i] == c)
			i++;
		ret[l] = malloc(1024);
		bzero(ret[l],1024);
		while(str[i] != c && str[i] != 0){
			ret[l][k] = str[i];
			k++;
			i++;
		}
		l++;
		k = 0;
	}
	return ret;
}
struct __DIR *__opendir(const char *name,FILE *f){
	uint8_t **s = sep(name,'/');
	struct tree_ent *ent = malloc(sizeof(*ent));
	bzero(ent,sizeof(*ent));
	uint8_t *buf = malloc(1024);
	_ata_read_master(buf,0,f);
	memcpy(ent,buf + 5,sizeof(*ent));
	_ata_read_master(buf,ent->nxtLba,f);
	struct tree_dirhdr *dhdr = malloc(sizeof(*dhdr));
	bzero(dhdr,sizeof(*dhdr));  
	if(strcmp(name,"/") != 0){
		memcpy(ent,buf,sizeof(*ent));
		memcpy(dhdr,buf + sizeof(*ent) + sizeof(struct tree_filehdr),sizeof(*dhdr));
		_ata_read_master(buf,dhdr->nxtTreeLba,f);
		memcpy(ent,buf,sizeof(*ent));
		bzero(dhdr,sizeof(*dhdr));
	}
	int i = 0;
	int prevLba = 0;
	struct tree_filehdr *fhdr = malloc(sizeof(*fhdr));
	if(strcmp(name,"/") != 0){ 
		while(s[i] != 0 && s[i][0]!= 0){
			_ata_read_master(buf,ent->nxtLba,f);
			memcpy(ent,buf,sizeof(*ent));
			int prevAlloc = 1;
			while(ent->alloc){
				if(ent->type == __TYPE_DIR){
					memcpy(fhdr,buf + sizeof(*ent),sizeof(*fhdr));
					if(strcmp(fhdr->name,s[i]) == 0){
						memcpy(dhdr,buf + sizeof(*ent) + sizeof(*fhdr),sizeof(*dhdr));
						prevAlloc = ent->alloc;
						prevLba = ent->nxtLba;
						_ata_read_master(buf,dhdr->nxtTreeLba,f);
						memcpy(ent,buf,sizeof(*ent));
						memcpy(fhdr,buf + sizeof(*ent),sizeof(*fhdr));
						memcpy(dhdr,buf + sizeof(*ent) + sizeof(*fhdr),sizeof(*dhdr));

						break;
					}
				}
				prevAlloc = ent->alloc;
				_ata_read_master(buf,ent->nxtLba,f);
				memcpy(ent,buf,sizeof(*ent));
			}if(!prevAlloc){
				return 0;
			}
			i++;
		}
	}else{
		_ata_read_master(buf,ent->nxtLba,f);
		memcpy(fhdr,buf + sizeof(*ent),sizeof(*fhdr));
		memcpy(dhdr,buf + sizeof(*ent) + sizeof(*fhdr),sizeof(*dhdr));
	}
	struct __DIR *ret = malloc(sizeof(*ret));
	ret->ent = malloc(sizeof(*ent));
	memcpy(ret->ent,ent,sizeof(*ent));
	ret->fhdr = malloc(sizeof(*fhdr));
	memcpy(ret->fhdr,fhdr,sizeof(*fhdr));
	ret->dhdr = malloc(sizeof(*dhdr));
	memcpy(ret->dhdr,dhdr,sizeof(*dhdr));
	return ret;
}
int __mkdir(const char *name,FILE *f){

	uint8_t **s = sep(name,'/');
	int i = 0;
	uint8_t *buf = malloc(512);
	struct tree_ent *ent = malloc(sizeof(*ent));
	bzero(ent,sizeof(*ent));
	_ata_read_master(buf,0,f);
	memcpy(ent,buf + 5,sizeof(*ent));
	int prevLba = ent->nxtLba;
	struct tree_dirhdr *dhdr = malloc(sizeof(*dhdr));
	bzero(dhdr,sizeof(*dhdr));
	_ata_read_master(buf,ent->nxtLba,f);
	if(strcmp(name,"/") != 0){
		memcpy(ent,buf,sizeof(*ent));
		memcpy(dhdr,buf + sizeof(*ent) + sizeof(struct tree_filehdr),sizeof(*dhdr));
		_ata_read_master(buf,dhdr->nxtTreeLba,f);
		memcpy(ent,buf,sizeof(*ent));
		bzero(dhdr,sizeof(*dhdr));
	}
	int svLba = 0;
	while(s[i+1] != 0){
		_ata_read_master(buf,ent->nxtLba,f);
		memcpy(ent,buf,sizeof(*ent));
		int prevAlloc = 1;
		while(ent->alloc){
			if(ent->type == __TYPE_DIR){
				struct tree_filehdr *fhdr = malloc(sizeof(*fhdr));
				memcpy(fhdr,buf + sizeof(*ent),sizeof(*fhdr));
				if(strcmp(fhdr->name,s[i]) == 0){
					memcpy(dhdr,buf + sizeof(*ent) + sizeof(*fhdr),sizeof(*dhdr));
					prevAlloc = ent->alloc;
					prevLba = ent->nxtLba;
					_ata_read_master(buf,dhdr->nxtTreeLba,f);
					memcpy(ent,buf,sizeof(*ent));
					if(s[i + 2] == 0)
						svLba = dhdr->nxtTreeLba;
					break;
				}
			}
			prevAlloc = ent->alloc;
			_ata_read_master(buf,ent->nxtLba,f);
			memcpy(ent,buf,sizeof(*ent));
		}if(!prevAlloc){
			return 0;
		}
		i++;
	}
	struct tree_filehdr *tfhdr = buf + sizeof(*ent);
	if(strcmp(tfhdr->name,s[i]) == 0 && strlen(s[i]) > 0){
		printf("[0]Error: Directory exists!\n");
		return 0;
	}
	while(ent->nxtLba != 0){
		prevLba = ent->nxtLba;
		struct tree_filehdr *fhdr = malloc(sizeof(*fhdr));
		memcpy(fhdr,buf + sizeof(*ent),sizeof(*fhdr));
		if(strcmp(fhdr->name,s[i]) == 0 && strlen(s[i]) > 0){
			printf("[1]Error: Directory %s exists!\n",s[i]);
			return 0;
		}
		_ata_read_master(buf,ent->nxtLba,f);
		memcpy(ent,buf,sizeof(*ent));
	}
	ent->nxtLba = find_free(f);
	char *pntr = malloc(512);
	_ata_read_master(pntr,prevLba,f);
	memcpy(pntr,ent,sizeof(*ent));
	_ata_write_master(pntr,prevLba,f);
	free(ent);
	ent = malloc(sizeof(*ent));
	bzero(ent,sizeof(*ent));
	ent->alloc = 1;
	ent->type = __TYPE_DIR;
	ent->nxtLba = 0;
	free(dhdr);
	dhdr = malloc(sizeof(*dhdr));
	struct tree_filehdr *fhdr = malloc(1024);
	fhdr->alloc = 1;
	bzero(fhdr->name,80); 
	if(strcmp(name,"/") != 0){
		fhdr->namelen = strlen(s[i]);
		memcpy(fhdr->name,s[i],fhdr->namelen);
	}else{
		fhdr->namelen = strlen(name);

		memcpy(fhdr->name,name,strlen(name));
	}
	dhdr->nxtTreeLba = 0;
	buf = malloc(512);
	bzero(buf,512);
	uint32_t lba = find_free(f);
	memcpy(buf,ent,sizeof(*ent));
	_ata_write_master(buf,find_free(f),f);
	ent->nxtLba = 0;
	memcpy(buf,ent,sizeof(*ent));
	memcpy(buf + sizeof(*ent),fhdr,sizeof(*fhdr));
	memcpy(buf + sizeof(*ent) + sizeof(*fhdr),dhdr,sizeof(*dhdr));
	_ata_write_master(buf,lba,f);
	dhdr->nxtTreeLba = find_free(f);
	memcpy(buf + sizeof(*ent) + sizeof(*fhdr),dhdr,sizeof(*dhdr));
	_ata_write_master(buf,lba,f);
	struct tree_ent *nent = malloc(sizeof(*nent));
	struct tree_filehdr *nfhdr = malloc(sizeof(*nfhdr));
	struct tree_dirhdr *ndhdr = malloc(sizeof(*ndhdr));
	ndhdr->nxtTreeLba = dhdr->nxtTreeLba;
	nent->alloc = 1;
	nent->type = __TYPE_DIR;
	nent->nxtLba = 0;
	nfhdr->alloc = 1;
	nfhdr->namelen = 1;
	bzero(nfhdr->name,80);
	strcpy(nfhdr->name,".");
	uint8_t *b = malloc(512);
	memcpy(b,nent,sizeof(*nent));
	_ata_write_master(b,dhdr->nxtTreeLba,f);
	nent->nxtLba = find_free(f);
	int wrlba = nent->nxtLba;
	memcpy(b,nent,sizeof(*nent));
	memcpy(b + sizeof(*nent),nfhdr,sizeof(*nfhdr));
	memcpy(b + sizeof(*nent) + sizeof(*nfhdr),ndhdr,sizeof(*ndhdr));
	_ata_write_master(b,dhdr->nxtTreeLba,f);
	nent = malloc(sizeof(*nent));
	nfhdr = malloc(sizeof(*nfhdr));
	ndhdr = malloc(sizeof(*ndhdr));
	bzero(nent,sizeof(*nent));
	bzero(nfhdr,sizeof(*nfhdr));
	bzero(ndhdr,sizeof(*ndhdr));
	ndhdr->nxtTreeLba = svLba;
	nent->alloc = 1;
	nent->type = __TYPE_DIR;
	nent->nxtLba = 0;
	nfhdr->alloc = 1;
	nfhdr->namelen = 2;
	bzero(nfhdr->name,80);
	strcpy(nfhdr->name,"..");
	b = malloc(512);
	memcpy(b,nent,sizeof(*nent));
	memcpy(b + sizeof(*nent),nfhdr,sizeof(*nfhdr));
	memcpy(b + sizeof(*nent) + sizeof(*nfhdr),ndhdr,sizeof(*ndhdr));
	_ata_write_master(b,wrlba,f);
	return 1;
}
int fsize(const char *name,FILE *f){
        char **arr = sep(name,'/');
        char *path = malloc(1024);
        int i = 0;
        strcpy(path,"/");
        while(arr[i+1] != 0){
                strcat(path,arr[i]);
                strcat(path,"/");
                i++;
        }
        struct __DIR *d = __opendir(path,f);
        if(!d)
                return -1;
	uint32_t lba = d->dhdr->nxtTreeLba;
        struct tree_ent *ent = malloc(sizeof(*ent));
        bzero(ent,sizeof(*ent));
        uint8_t *buf = malloc(512);
        _ata_read_master(buf,lba,f);
        memcpy(ent,buf,sizeof(*ent));
        int prevLba = 0;
        struct tree_filehdr *fhdr = malloc(sizeof(*fhdr));
        while(ent->nxtLba != 0){
                if(ent->type == __TYPE_FILE){
			memcpy(fhdr,buf + sizeof(*ent),sizeof(*fhdr));
   
                        if(strcmp(fhdr->name,arr[i]) == 0){
                                break;
                        }
                }
                lba = ent->nxtLba;
                prevLba = ent->nxtLba;
                _ata_read_master(buf,ent->nxtLba,f);
                memcpy(ent,buf,sizeof(*ent));
        }
        if(!ent->alloc){
                return -2;
        }
        i = 0;
        _ata_read_master(buf,prevLba,f);
        struct tree_fexclusive *fex = malloc(sizeof(*fex));
        memcpy(fex,buf + sizeof(*ent) + sizeof(struct tree_filehdr),sizeof(*fex));
        lba = fex->nxtFLba;
        i = 0;
        while(fex->nxtFLba > 0){
                _ata_read_master(buf,lba,f);
                struct tree_ent *tent = malloc(sizeof(*tent));
                memcpy(tent,buf,sizeof(*tent));
                memcpy(fex,buf + sizeof(*tent),sizeof(*fex));
                lba = fex->nxtFLba;
                i+=512-sizeof(*ent)-sizeof(*fex);
        }
        return i;

}

int read_file(const char *_path,void *pntr,FILE *f){
	char **arr = sep(_path,'/');
	char *path = malloc(1024);
	int i = 0;
	strcpy(path,"/");
	while(arr[i+1] != 0){
		strcat(path,arr[i]);
		strcat(path,"/");
		i++;
	}
	struct __DIR *d = __opendir(path,f);
	if(!d)
		return 0;
	uint32_t lba = d->dhdr->nxtTreeLba;
	struct tree_ent *ent = malloc(sizeof(*ent));
	bzero(ent,sizeof(*ent));
	uint8_t *buf = malloc(512);
	bzero(buf,512);
	_ata_read_master(buf,lba,f);
	memcpy(ent,buf,sizeof(*ent));
	int prevLba = 0;
	struct tree_filehdr *fhdr = malloc(sizeof(*fhdr));
	while(ent->nxtLba != 0){
		if(ent->type == __TYPE_FILE){
			memcpy(fhdr,buf + sizeof(*ent),sizeof(*fhdr));

			if(strcmp(fhdr->name,arr[i]) == 0){
				break;
			}
		}
		lba = ent->nxtLba;
		prevLba = ent->nxtLba;
		_ata_read_master(buf,ent->nxtLba,f);
		memcpy(ent,buf,sizeof(*ent));
	}
	if(!ent->alloc){
		return -1;
	}
	i = 0;
	uint32_t offset = 0;
	/*
	 *This loop makes it so that we start at the correct lba and offset
	 *by looping through until we meet or exceed the offset set by the
	 *file descriptor
	 */
//	free(buf);
	_ata_read_master(buf,prevLba,f);
	struct tree_fexclusive *fex = malloc(sizeof(*fex));
	memcpy(fex,buf + sizeof(*ent) + sizeof(struct tree_filehdr),sizeof(*fex));
	lba = fex->nxtFLba;
	i = 0;
	/*
	 *Reads the data from the file into the pointer
	 */
	offset = 0;
	i = 0;
	int init = 1;
	int pos = 0;
	while(1){
		_ata_read_master(buf,lba,f);
		struct tree_ent *tent = malloc(sizeof(*tent));
		memcpy(tent,buf,sizeof(*tent));
		memcpy(fex,buf + sizeof(*ent),sizeof(*fex));
		lba = fex->nxtFLba;
		if(lba != 0){
			if(!init){
				memcpy(pntr + offset,buf + sizeof(*tent) + sizeof(*fex),512-sizeof(*tent)-sizeof(*fex));
			}else{
				memcpy(pntr + offset,buf+sizeof(*tent)+sizeof(*fex)+pos%(512-sizeof(*tent)-sizeof(*fex)),512-sizeof(*tent)-sizeof(*fex)-pos);
			}if(!init){
				offset+=512-sizeof(*ent)-sizeof(*fex);
//				remaining-=512-sizeof(*ent)-sizeof(*fex);
			}else{
				offset+=512-sizeof(*ent)-sizeof(*fex);
				init = 0;
//				remaining-=512-sizeof(*ent)-sizeof(*fex)-pos;
			}
		}else{
			struct tree_fend *fend = malloc(sizeof(*fend));
			memcpy(fend,buf + sizeof(*tent) + sizeof(*fex),sizeof(*fend));
			if(!init)
				memcpy(pntr + offset,buf + sizeof(*fex) + sizeof(*fend),fend->finalBytes);
			else
				memcpy(pntr + offset,buf + sizeof(*fex) + sizeof(*fend)+pos,fend->finalBytes);
			offset+=fend->finalBytes;
			pos +=offset;
			return offset;
		}
	}
	return offset;

}
int write_file(const char *path,void *dstpntr,int n,FILE *f){
	uint8_t **s = sep(path,'/');
	char *dirp = malloc(1024);
	bzero(dirp,1024);
	int i = 0;
	strcpy(dirp,"/");
	while(s[i+1] != 0){
		strcat(dirp,s[i]);
		strcat(dirp,"/");
		i++;
	}
	struct __DIR *d = __opendir(dirp,f);
	if(!d){
		printf("Failed to open dir %s\n",dirp);
		return 0;
	}
	struct tree_ent *ent = malloc(sizeof(*ent));
	uint8_t *pntr = malloc(512);

	memcpy(ent,d->ent,sizeof(*ent));
	int slba = 0;
	int prevLba = ent->nxtLba;
	while(ent->nxtLba != 0){
		_ata_read_master(pntr,ent->nxtLba,f);
		struct tree_filehdr *fhdr = malloc(sizeof(*fhdr));
		memcpy(fhdr,pntr + sizeof(*ent),sizeof(*fhdr));
		if(strcmp(fhdr->name,s[i]) == 0){
			slba = prevLba;
			break;
		}
		prevLba = ent->nxtLba;
		memcpy(ent,pntr,sizeof(*ent));
	}
	slba = slba == 0? prevLba : slba;
	ent->nxtLba = find_free(f);
	uint8_t *buf = malloc(512);
	_ata_read_master(buf,prevLba,f);
	memcpy(buf,ent,sizeof(*ent));
	_ata_write_master(buf,prevLba,f);
	int lba = find_free(f);
	struct tree_ent *entry = malloc(sizeof(*entry));
	bzero(entry,sizeof(*entry));
	entry->alloc = 1;
	entry->type = __TYPE_FILE;
	entry->nxtLba = 0;
	struct tree_filehdr *fhdr = malloc(sizeof(*fhdr));
	fhdr->alloc = 1;
	fhdr->namelen = strlen(s[i]);
	strcpy(fhdr->name,s[i]);
	int bytesRemaining = n;
	bzero(buf,512);
	memcpy(buf,entry,sizeof(*entry));
	memcpy(buf + sizeof(*entry),fhdr,sizeof(*fhdr));
	_ata_write_master(buf,lba,f);
	struct tree_fexclusive *fex = malloc(sizeof(*fex));
	fex->nxtFLba = find_free(f);
	memcpy(buf + sizeof(*entry) + sizeof(*fhdr),fex,sizeof(*fex));
	_ata_write_master(buf,lba,f);
	int currLba = fex->nxtFLba;
	bzero(buf,512);
	entry->type = __TYPE_DATA;
	memcpy(buf,entry,sizeof(*entry));
	int pos = 0;
	while(bytesRemaining > 512-sizeof(*fex)-sizeof(*entry)-sizeof(struct tree_fend)){
		_ata_write_master(buf,currLba,f);
		fex->nxtFLba = find_free(f);
		bzero(buf + sizeof(*entry),512-sizeof(*entry));
		memcpy(buf + sizeof(*entry),fex,sizeof(*fex));
		memcpy(buf + sizeof(*entry) + sizeof(*fex),dstpntr + pos*(512-sizeof(*fex)-sizeof(*entry)),512-sizeof(*fex)-sizeof(*entry));
		_ata_write_master(buf,currLba,f);
		currLba = fex->nxtFLba;
		pos++;
		bytesRemaining-=512-sizeof(*fex)-sizeof(*entry);
	}
	bzero(buf + sizeof(*entry),512-sizeof(*entry));
	fex->nxtFLba = 0;
	struct tree_fend *fend = malloc(sizeof(*fend));
	fend->finalBytes = bytesRemaining;
	memcpy(buf + sizeof(*entry),fex,sizeof(*fex));
	memcpy(buf + sizeof(*entry) + sizeof(*fex),fend,sizeof(*fend));
	memcpy(buf + sizeof(*entry) + sizeof(*fex) + sizeof(*fend),dstpntr + pos*(512-sizeof(*fex)-sizeof(*entry)),bytesRemaining);
	bytesRemaining-=bytesRemaining;
	_ata_write_master(buf,currLba,f);
	return 1;	
}

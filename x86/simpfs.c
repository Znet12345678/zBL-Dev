/*
 *simple FS Driver for zBL
 *(c) 2017 Zachary James Schlotman
 */
#include "lib.h"
#include "mem.h"
#include "simpfs.h"
#include "libc/include/environment.h"
#include <stdint.h>
/*
 *Finds free block and returns the lba of that block
 */
uint32_t find_free(){
	uint32_t ret = 1;
	uint8_t *buf = malloc(1024);
	_ata_read_master(buf,ret,0);
	while(buf[0]){
		ret++;
		_ata_read_master(buf,ret,0);
	}
	return ret;
}
/*
 *Opens a file returns a file descriptor
 *ONLY SUPPORTS READING AS OF NOW
 */
int open(const char *fname,int options){
	struct fd *tbl = (struct fd *)0x00007E00;
	int i = 0;
	while(tbl->alloc){
		i++;
		tbl+=sizeof(*tbl);
	}
	if(!(options >> 1 & 1)){
		struct fd *n = getInfo(fname);
		if(!n){
//			kprintf("Failed to open file\n");
			bzero(n,sizeof(*n));
			return -1;
		}
		n->type = options;
		memcpy(tbl,n,sizeof(*tbl));
		return i;
	}
}
/*
 *Gets file descriptor pointer from fd
 */
struct fd *getFd(int n){
	struct fd *tbl = (struct fd *)0x00007E00;
	int i = 0;
	while(i < n){
		i++;
		tbl+=sizeof(*tbl);
	}
	return tbl;
}
int tell(int fd){
	struct fd *f = getFd(fd);
	return f->pos;
}
int lseek(int fd,int n,int mode){
	struct fd *pntr = getFd(fd);
	if(mode == SEEK_CUR);
	else if (mode == SEEK_END){
		pntr->pos = fsize(fd);
	}else if(mode == SEEK_SET){
		pntr->pos = n;
	}
	return pntr->pos;
}
/*
 *Gets info from disk to store in a pointer that can be accessed through a file descriptor
 */
void ddump(DIR *d){
	kprintf("ENT:\n");
	kprintf("[alloc]%d [type]%d [nxtLba]%d\n",d->ent->alloc,d->ent->type,d->ent->nxtLba);
	kprintf("FHDR:\n");
	kprintf("[alloc]%d [namelen]%d [name]%s\n",d->fhdr->alloc,d->fhdr->namelen,d->fhdr->name);
	kprintf("DHDR:\n");
	kprintf("[nxtTreeLba]%d\n",d->dhdr->nxtTreeLba);
}
void dent(struct tree_ent *e){
	kprintf("ENT:\n");
	kprintf("[alloc]%d [type]%d [nxtLba]%d\n",e->alloc,e->type,e->nxtLba);
}
struct fd *getInfo(const char *str){
	char **s = sep(str,'/');
	char *dpath = malloc(strlen(str));
	int i = 0;
	strcpy(dpath,"/");
	while(s[i + 1] != 0){
		strcat(dpath,s[i]);
		strcat(dpath,"/");
		i++;
	}
	DIR *d = opendir(dpath);
	if(!d){
		kprintf("Error opening dir\n");
		return 0;
	}
	struct tree_ent *ent = malloc(sizeof(*ent));
	uint8_t *buf = malloc(512);
	struct tree_dirhdr *dhdr = malloc(sizeof(*dhdr));
	_ata_read_master(buf,d->dhdr->nxtTreeLba,0);
	memcpy(ent,buf,sizeof(*ent));
	int prevLba = d->dhdr->nxtTreeLba;
	struct tree_filehdr *fhdr = malloc(sizeof(*fhdr));
	while(ent->alloc){
		memcpy(fhdr,buf + sizeof(*ent),sizeof(*fhdr));
		memcpy(dhdr,buf + sizeof(*ent) + sizeof(*fhdr),sizeof(*dhdr));
		if(ent->type == __TYPE_FILE && strcmp(fhdr->name,s[i]) == 0){
			struct fd *ret = malloc(sizeof(*ret));
			bzero(ret,sizeof(*ret));
			ret->alloc = 1;
			ret->tLba = prevLba;
			ret->pos = 0;
			memcpy(ret->name,str,strlen(str));
			return ret;
		}
		prevLba = ent->nxtLba;
		if(ent->nxtLba == 0)
			break;
		_ata_read_master(buf,ent->nxtLba,0);
		memcpy(ent,buf,sizeof(*ent));
	}
	return 0;
}
/*
 *Returns 1 if the file system is present on the disk and 0 when it is not
 */
int isSimpfs(){
	uint8_t * buf = malloc(512);
	_ata_read_master(buf,0,0);
	return buf[0] == 0x7f && buf[1] == 'S' && buf[2] == 'I' && buf[3] == 'M' && buf[4] == 'P';
}
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
/*
 *Creates the file system
 */
int mkfs(){
	uint8_t *buf = malloc(512);
	buf[0] = 0x7f;
	buf[1] = 'S';
	buf[2] = 'I';
	buf[3] = 'M';
	buf[4] = 'P';
	struct tree_ent *head = malloc(512);
	head->alloc = 1;
	head->type = __TYPE_DIR;
	head->nxtLba = 1;
	memcpy(buf + 5,head,sizeof(*head));
	_ata_write_master(buf,0);
	mkdir("/");
}
/*
 *Zeros out pointer
 */
void bzero(void *vpntr,int n){
	uint8_t *pntr = (uint8_t*)vpntr;
	for(int i = 0; i < n;i++)
		pntr[i] = 0;
}
/*
 *Creates dir by the name of name
 *returns 0 on failure and 1 on success
 */
int f = 0;
int mkdir(const char *name){
	uint8_t **s = sep(name,'/');
	int i = 0;
	uint8_t *buf = malloc(512);
	struct tree_ent *ent = malloc(sizeof(*ent));
	bzero(ent,sizeof(*ent));
	_ata_read_master(buf,0,0);
	memcpy(ent,buf + 5,sizeof(*ent));
	int prevLba = ent->nxtLba;
	struct tree_dirhdr *dhdr = malloc(sizeof(*dhdr));
	bzero(dhdr,sizeof(*dhdr));
	_ata_read_master(buf,ent->nxtLba,0);
	if(strcmp(name,"/") != 0){
		memcpy(ent,buf,sizeof(*ent));
		memcpy(dhdr,buf + sizeof(*ent) + sizeof(struct tree_filehdr),sizeof(*dhdr));
		_ata_read_master(buf,dhdr->nxtTreeLba,0);
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
		kprintf("[0]Error: Directory exists!\n");
		return 0;
	}
	while(ent->nxtLba != 0){
		prevLba = ent->nxtLba;
		struct tree_filehdr *fhdr = malloc(sizeof(*fhdr));
		memcpy(fhdr,buf + sizeof(*ent),sizeof(*fhdr));
		if(strcmp(fhdr->name,s[i]) == 0 && strlen(s[i]) > 0){
			kprintf("[1]Error: Directory %s exists!\n",s[i]);
			return 0;
		}
		_ata_read_master(buf,ent->nxtLba,f);
		memcpy(ent,buf,sizeof(*ent));
	}
	ent->nxtLba = find_free(f);
	char *pntr = malloc(512);
	_ata_read_master(pntr,prevLba,0);
	memcpy(pntr,ent,sizeof(*ent));
	_ata_write_master(pntr,prevLba);
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
	_ata_write_master(buf,find_free(f));
	ent->nxtLba = 0;
	memcpy(buf,ent,sizeof(*ent));
	memcpy(buf + sizeof(*ent),fhdr,sizeof(*fhdr));
	memcpy(buf + sizeof(*ent) + sizeof(*fhdr),dhdr,sizeof(*dhdr));
	_ata_write_master(buf,lba);
	dhdr->nxtTreeLba = find_free(f);
	memcpy(buf + sizeof(*ent) + sizeof(*fhdr),dhdr,sizeof(*dhdr));
	_ata_write_master(buf,lba);
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
	_ata_write_master(b,dhdr->nxtTreeLba);
	nent->nxtLba = find_free(f);
	int wrlba = nent->nxtLba;
	memcpy(b,nent,sizeof(*nent));
	memcpy(b + sizeof(*nent),nfhdr,sizeof(*nfhdr));
	memcpy(b + sizeof(*nent) + sizeof(*nfhdr),ndhdr,sizeof(*ndhdr));
	_ata_write_master(b,dhdr->nxtTreeLba);
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
	_ata_write_master(b,wrlba);
	return 1;


}
/*
 *Returns DIR pointer that holds all the info currently needed for I/O on directories or 0 on error
 */
DIR *opendir(const char *name){
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

			while(ent->alloc == 1){
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
/*
 *Returns the size of the file
 */
int fsize(const char *name){
	char **arr = sep(name,'/');
	char *path = malloc(1024);
	int i = 0;
	strcpy(path,"/");
	while(arr[i+1] != 0){
		strcat(path,arr[i]);
		strcat(path,"/");
		i++;
	}
	DIR *d = opendir(path);
	if(!d)
		return -1;
	uint32_t lba = d->dhdr->nxtTreeLba;
	struct tree_ent *ent = malloc(sizeof(*ent));
	bzero(ent,sizeof(*ent));
	uint8_t *buf = malloc(512);
	_ata_read_master(buf,lba,0);
	memcpy(ent,buf,sizeof(*ent));
	int prevLba = 0;
	struct tree_filehdr *fhdr = malloc(sizeof(*fhdr));
	while(ent->alloc){
		if(ent->type == __TYPE_FILE){
			memcpy(fhdr,buf + sizeof(*ent),sizeof(*fhdr));
			if(strcmp(fhdr->name,arr[i]) == 0){
				return ent->size;
			}
		}
		lba = ent->nxtLba;
		if(lba == 0)
			return -1;
		prevLba = ent->nxtLba;
		_ata_read_master(buf,ent->nxtLba,0);
		memcpy(ent,buf,sizeof(*ent));
	}

	return -1;
}
/*
 *Reads from n bytes from fd into buf
 *Returns bytes read
 */
int read(int fd,void *pntr,int n){
	struct fd *f = getFd(fd);
	if(f->type & 1){
		char **arr = sep(f->name,'/');
		char *path = malloc(1024);
		int i = 0;
		strcpy(path,"/");
		while(arr[i+1] != 0){
			strcat(path,arr[i]);
			strcat(path,"/");
			i++;
		}
		DIR *d = opendir(path);
		if(!d)
			return 0;
		uint32_t lba = d->dhdr->nxtTreeLba;
		struct tree_ent *ent = malloc(sizeof(*ent));
		bzero(ent,sizeof(*ent));
		uint8_t *buf = malloc(512);
		bzero(buf,512);
		_ata_read_master(buf,lba,0);
		memcpy(ent,buf,sizeof(*ent));
		int prevLba = 0;
		struct tree_filehdr *fhdr = malloc(sizeof(*fhdr));
		while(ent->alloc){
			if(ent->type == __TYPE_FILE){
				memcpy(fhdr,buf + sizeof(*ent),sizeof(*fhdr));
				if(strcmp(fhdr->name,arr[i]) == 0){
					break;
				}
			}
			lba = ent->nxtLba;
			if(lba == 0)
				return -1;
			prevLba = ent->nxtLba;
			_ata_read_master(buf,ent->nxtLba,0);
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
		_ata_read_master(buf,prevLba,0);
		struct tree_fexclusive *fex = malloc(sizeof(*fex));
		memcpy(fex,buf + sizeof(*ent) + sizeof(struct tree_filehdr),sizeof(*fex));
		lba = fex->nxtFLba;
		i = 0;
		while(i < f->pos){
			_ata_read_master(buf,lba,0);
			struct tree_ent *tent = malloc(sizeof(*tent));
			memcpy(tent,buf,sizeof(*tent));
			memcpy(fex,buf + sizeof(*tent),sizeof(*fex));
			if(512-sizeof(*ent)-sizeof(*fex)>(f->pos-i)){
				break;
			}
			lba = fex->nxtFLba;
			i+=512-sizeof(*ent)-sizeof(*fex);
		}
		/*
		 *Reads the data from the file into the pointer
		 */
		offset = 0;
		i = 0;
		int init = 1;
		int remaining = n;
		while(offset < n){
			_ata_read_master(buf,lba,0);
			struct tree_ent *tent = malloc(sizeof(*tent));
			memcpy(tent,buf,sizeof(*tent));
			memcpy(fex,buf + sizeof(*ent),sizeof(*fex));
			lba = fex->nxtFLba;
			if(lba != 0){
				if(remaining < 512-sizeof(*ent)-sizeof(*fex)){
					if(!init)
						memcpy(pntr + offset,buf + sizeof(*tent) + sizeof(*fex),n-offset);
					else{
						memcpy(pntr + offset,buf + sizeof(*tent) + sizeof(*fex) + f->pos%(512-sizeof(*tent)-sizeof(*fex)),n-offset);
						init = 0;
					}offset+=n-offset;
					f->pos +=offset;
					remaining = 0;

					return offset;
				}else if(!init)
					memcpy(pntr + offset,buf + sizeof(*tent) + sizeof(*fex),512-sizeof(*tent)-sizeof(*fex));
				else{
					memcpy(pntr + offset,buf+sizeof(*tent)+sizeof(*fex)+f->pos%(512-sizeof(*tent)-sizeof(*fex)),512-sizeof(*tent)-sizeof(*fex)-f->pos);
					init=0;
				}if(!init){
					offset+=512-sizeof(*ent)-sizeof(*fex);
					remaining-=512-sizeof(*ent)-sizeof(*fex);
				}else{
					offset+=512-sizeof(*ent)-sizeof(*fex)-f->pos;
					init = 0;
					remaining-=512-sizeof(*ent)-sizeof(*fex)-f->pos;
				}
			}else{
				struct tree_fend *fend = malloc(sizeof(*fend));
				memcpy(fend,buf + sizeof(*tent) + sizeof(*fex),sizeof(*fend));
				if(!init)
					memcpy(pntr + offset,buf + sizeof(*fex) + sizeof(*fend),fend->finalBytes);
				else
					memcpy(pntr + offset,buf + sizeof(*fex) + sizeof(*fend)+f->pos,fend->finalBytes);
				offset+=fend->finalBytes;
				f->pos = f->pos+offset;
				return offset;
			}
		}	
		f->pos += offset;
		return offset;
	}else
		return -2;
}
int write(int fd,void *pntr,int n){
	struct fd *f = getFd(fd);
	if(f->type >> 1 & 1){
		char **s = sep(f->name,'/');
		char *path = malloc(1024);
		int i = 0;
		strcat(path,"/");
		while(s[i + 1] != 0){
			strcat(path,s[i]);
			strcat(path,"/");
			i++;
		}
		DIR *d = opendir(path);
		if(!d)
			return -1;
		uint32_t lba = d->dhdr->nxtTreeLba;
                struct tree_ent *ent = malloc(sizeof(*ent));
                bzero(ent,sizeof(*ent));
                uint8_t *buf = malloc(512);
                bzero(buf,512);
                _ata_read_master(buf,lba,0);
                memcpy(ent,buf,sizeof(*ent));
                int prevLba = lba;
                struct tree_filehdr *fhdr = malloc(sizeof(*fhdr));
                while(ent->alloc){
                        if(ent->type == __TYPE_FILE){
                                memcpy(fhdr,buf + sizeof(*ent),sizeof(*fhdr));
                                if(strcmp(fhdr->name,s[i]) == 0){
                                        break;
                                }
                        }
                        lba = ent->nxtLba;
			if(lba == 0)
				break;
                        prevLba = ent->nxtLba;
                        _ata_read_master(buf,ent->nxtLba,0);
                        memcpy(ent,buf,sizeof(*ent));
                }
                if(!ent->alloc || lba == 0){
                        ent->nxtLba = find_free();
			memcpy(buf,ent,sizeof(*ent));
			_ata_write_master(buf,prevLba);
			int initLba = ent->nxtLba;
			free(ent);
			ent = malloc(sizeof(*ent));
			ent->alloc = 1;
			ent->size = n;
			ent->nxtLba = 0;
			ent->type = __TYPE_FILE;
			free(fhdr);
			fhdr = malloc(sizeof(*fhdr));
			fhdr->alloc = 1;
			struct fd *f = getFd(fd);
			memcpy(fhdr->name,f->name,strlen(f->name));
			fhdr->namelen = strlen(fhdr->name);
			struct tree_fexclusive *fex = malloc(sizeof(*fex));
			fex->nxtFLba = 0;
			free(buf);
			buf = malloc(1024);
			memcpy(buf,ent,sizeof(*ent));
			memcpy(buf + sizeof(*ent),fhdr,sizeof(*fhdr));
			memcpy(buf + sizeof(*ent) + sizeof(*fhdr),fex,sizeof(*fex));
			_ata_write_master(buf,initLba);
			fex->nxtFLba = find_free();
			memcpy(buf + sizeof(*ent) + sizeof(*fhdr),fex,sizeof(*fex));
			_ata_write_master(buf,initLba);
                }
                i = 0;
                uint32_t offset = 0;
                /*
                 *This loop makes it so that we start at the correct lba and offset
                 *by looping through until we meet or exceed the offset set by the
                 *file descriptor
                 */
                _ata_read_master(buf,prevLba,0);
                struct tree_fexclusive *fex = malloc(sizeof(*fex));
                memcpy(fex,buf + sizeof(*ent) + sizeof(struct tree_filehdr),sizeof(*fex));
                lba = fex->nxtFLba;
                i = 0;
		int size = 0;
                while(i < f->pos){
                        _ata_read_master(buf,lba,0);
                        struct tree_ent *tent = malloc(sizeof(*tent));
                        memcpy(tent,buf,sizeof(*tent));
			tent->size+=n;
			if(i == 0)
				size = tent->size;
                        memcpy(fex,buf + sizeof(*tent),sizeof(*fex));
			_ata_write_master(buf,lba);
                        if(512-sizeof(*ent)-sizeof(*fex)>(f->pos-i)){
                                break;
                        }
			lba = fex->nxtFLba;
                        i+=512-sizeof(*ent)-sizeof(*fex);
                }
		int j = 0;
		while(j < n){
			struct tree_ent *ent = malloc(sizeof(*ent));
			struct tree_fexclusive *fex = malloc(sizeof(*fex));
			ent->size=size;
			ent->alloc = 1;
			ent->type = __TYPE_FILE;
			ent->nxtLba = 0;
			fex->nxtFLba = find_free();
			uint8_t *buf = malloc(512);
			memcpy(buf,ent,sizeof(*ent));
			memcpy(buf + sizeof(*ent),fex,sizeof(*fex));
			if(j + 512-sizeof(*ent)-sizeof(*fex) < n){
				memcpy(buf + sizeof(*ent) + sizeof(*fex),pntr + j,512-sizeof(*ent)-sizeof(*fex));
				f->pos+=512-sizeof(*ent)-sizeof(*fex);
				j+=512-sizeof(*ent)-sizeof(*fex);
				_ata_write_master(buf,lba);
			}else{
				memcpy(buf + sizeof(*ent) + sizeof(*fex),pntr + j,n%(512-sizeof(*ent)-sizeof(*fex)));
				f->pos+=n%(512-sizeof(*ent)-sizeof(*fex));
				j+=n%(512-sizeof(*ent)-sizeof(*fex));
				_ata_write_master(buf,lba);
				return j;
			}
		}
		return j;

	}
}
int close(int fd){
	struct fd *f = getFd(fd);
	f->alloc = 0;
	bzero(f,sizeof(*f));
	return 1;	
}
int __exec(const char *path){
	uint8_t *buf = malloc(fsize(path));
	bzero(buf,fsize(path));
	int fd = open(path,O_RDONLY);
	if(fd < 0){
		kprintf("Error opening\n");
		return -1;
	}
	int r = read(fd,buf,fsize(path));
	int (*main)() = exec_elf(0,buf);
	t_writevals(); 
	main();

}
int exec(const char *path,const char **argv){
	uint8_t *buf = malloc(fsize(path));
	int fd = open(path,O_RDONLY);
	if(fd < 0)
		return -1;
	read(fd,buf,fsize(path));
	int argc = 0;
	while(argv[argc] != 0)
		argc++;
	int (*main)(int argc,char **argv) = exec_elf(0,buf);
	t_writevals();
	main(argc,argv);
	t_readvals();
}

void list(const char *path){
	kprintf("LS %s:\n",path);
	struct __DIR *d = opendir(path);
	if(!d){
		kprintf("I/O Error\n");
	}
	uint8_t *buf = malloc(512);
	_ata_read_master(buf,d->dhdr->nxtTreeLba,0);
	struct tree_ent *ent = malloc(sizeof(*ent));
	struct tree_filehdr *fhdr = malloc(sizeof(*fhdr));
	struct tree_dirhdr *dhdr = malloc(sizeof(*dhdr));
	memcpy(ent,buf,sizeof(*ent));
	while(ent->alloc){
		memcpy(fhdr,buf + sizeof(*ent),sizeof(*fhdr));
		kprintf("%s\n",fhdr->name);
		_ata_read_master(buf,ent->nxtLba,0);
		if(ent->nxtLba == 0)
			break;
		memcpy(ent,buf,sizeof(*ent));

	}

}

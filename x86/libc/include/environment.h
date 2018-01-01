#ifndef __ENV_H
#define __ENV_H
struct envVar{
	char name[80];
	char val[80];
	struct envVar *nxt;
};
#endif

#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *checked_malloc(int len)
{
	void *p = malloc(len);
	assert(p);
	return p;
}

string String(char *s)
{
	string p = checked_malloc(strlen(s)+1);
	strcpy(p, s);
	return p;
}

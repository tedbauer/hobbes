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

string StringAppend(char *s1, char *s2)
{
	string p = checked_malloc(strlen(s1) + strlen(s2) + 1);
	strcat(p, s1);
	strcat(p, s2);
	return p;
}

string String(char *s)
{
	string p = checked_malloc(strlen(s) + 1);
	strcpy(p, s);
	return p;
}

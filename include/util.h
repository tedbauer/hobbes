#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef char *string;
string String(char *);
string StringAppend(char *s1, char *s2);

typedef char bool;
#define TRUE 1;
#define FALSE 0;

void *checked_malloc(int);

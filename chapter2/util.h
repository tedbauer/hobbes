#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef char *string;
string String(char *);

typedef char bool;
#define TRUE 1;
#define FALSE 0;

void *checked_malloc(int);

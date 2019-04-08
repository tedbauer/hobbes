/*
 * parse.c - Parse source file.
 */

#include <stdio.h>
#include "../include/util.h"
#include "../include/symbol.h"
#include "../include/absyn.h"
#include "../include/errormsg.h"
#include "../include/parse.h"

extern int yyparse(void);
extern A_exp absyn_root;

/* parse source file fname;
   return abstract syntax data structure */
A_exp parse(string fname)
{
	EM_reset(fname);
 	if (yyparse() == 0) { /* parsing worked */
		return absyn_root;
	} else {
		return NULL;
	}
}

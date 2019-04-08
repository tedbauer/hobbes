#include <stdio.h>
#include "../include/util.h"
#include "../include/symbol.h"
#include "../include/absyn.h"
#include "../include/parse.h"
#include "../include/prabsyn.h"
#include "../include/errormsg.h"
#include "../include/translate.h"
#include "../include/types.h"
#include "../include/env.h"
#include "../include/semant.h"

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "usage: tigger filename\n");
		exit(1);
	}
	A_exp ast = parse(argv[1]);
	printf("========== AST ==========\n");
	pr_exp(stdout, ast, 0);
	printf("\n");
	printf("=========================\n");
	SEM_transProg(ast);
	return 0;
}

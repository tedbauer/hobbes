#include <stdio.h>
#include "util.h"
#include "symbol.h"
#include "absyn.h"
#include "parse.h"
#include "prabsyn.h"
#include "errormsg.h"

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "usage: show_ast filename");
		exit(1);
	}

	A_exp ast = parse(argv[1]);
	printf("========== AST ==========\n");
	pr_exp(stdout, ast, 0);
	printf("\n");
	printf("=========================\n");
	return 0;
}

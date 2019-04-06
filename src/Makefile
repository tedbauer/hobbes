parsetest: parsetest.o y.tab.o lex.yy.o errormsg.o util.o absyn.o symbol.o
	cc -g -o $@ parsetest.o y.tab.o lex.yy.o errormsg.o util.o absyn.o symbol.o

show_ast: show_ast.o y.tab.o lex.yy.o errormsg.o util.o absyn.o symbol.o parse.o prabsyn.o table.o
	cc -g -o $@ show_ast.o y.tab.o lex.yy.o errormsg.o util.o absyn.o symbol.o parse.o prabsyn.o table.o

lextest: driver.o lex.yy.o errormsg.o util.o
	cc -g -o $@ driver.o lex.yy.o errormsg.o util.o

driver.o: driver.c tokens.h errormsg.h util.h
	cc -g -c driver.c

prabsyn.o: prabsyn.c prabsyn.h
	cc -g -c prabsyn.c

parse.o: parse.c parse.h
	cc -g -c parse.c

parsetest.o: parsetest.c errormsg.h util.h
	cc -g -c parsetest.c

table.o: table.c table.h
	cc -g -c table.c

show_ast.o: show_ast.c errormsg.h util.h
	cc -g -c show_ast.c

symbol.o: symbol.c symbol.h
	cc -g -c symbol.c

absyn.o: absyn.c absyn.h
	cc -g -c absyn.c

y.tab.o: y.tab.c
	cc -g -c y.tab.c

y.tab.c: tiger.grm
	yacc -dv tiger.grm

y.tab.h: y.tab.c
	echo "y.tab.h was created at the same time as y.tab.c"

errormsg.o: errormsg.c errormsg.h util.h
	cc -g -c errormsg.c

lex.yy.o: lex.yy.c y.tab.h errormsg.h util.h
	cc -g -c lex.yy.c

lex.yy.c: tiger.lex
	lex tiger.lex

util.o: util.c util.h
	cc -g -c util.c

clean:
	rm -f y.tab.c y.tab.h lex.yy.c lextest parsetest *.o y.output parsetest.out

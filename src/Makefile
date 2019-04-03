parsetest: parsetest.o y.tab.o lex.yy.o errormsg.o util.o
	cc -g -o $@ parsetest.o y.tab.o lex.yy.o errormsg.o util.o

lextest: driver.o lex.yy.o errormsg.o util.o
	cc -g -o $@ driver.o lex.yy.o errormsg.o util.o

driver.o driver.c tokens.h errormsg.h util.h:
	cc -g -c driver.c

parsetest.o: parsetest.c errormsg.h util.h
	cc -g -c parsetest.c

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
	rm -f y.tab.c y.tab.h lex.yy.c lextest parsetest *.o

a.out: interp.o prog1.o ast.o util.o
	cc -g interp.o prog1.o ast.o util.o

interp.o: interp.c ast.h util.h
	cc -g -c interp.c

prog1.o: prog1.c ast.h util.h
	cc -g -c prog1.c

ast.o: ast.c ast.h util.h
	cc -g -c ast.c

util.o: util.c util.h
	cc -g -c util.c

clean:
	rm -f a.out util.o prog1.o ast.o interp.o



a.out: main.o prog1.o ast.o util.o
	cc -g main.o prog1.o ast.o util.o

main.o: main.c ast.h util.h
	cc -g -c main.c

prog1.o: prog1.c ast.h util.h
	cc -g -c prog1.c

ast.o: ast.c ast.h util.h
	cc -g -c ast.c

util.o: util.c util.h
	cc -g -c util.c

clean:
	rm -f a.out util.o prog1.o ast.o main.o



.PHONY: clean hobbes

CFLAGS = -g

IDIR = ../include
ODIR = _build/obj

_DEPS = absyn.h errormsg.h prabsyn.h symbol.h tokens.h types.h env.h parse.h semant.h table.h translate.h util.h linkedlist.h
DEPS = $(patsubst %,$(IDIR)/%, $(_DEPS))

_OBJ = env.o parse.o prabsyn.o hobbes.o table.o util.o linkedlist.o absyn.o errormsg.o semant.o symbol.o types.o lex.yy.o y.tab.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

hobbes: $(OBJ)
	cc -o $@ $^ $(CFLAGS)

y.tab.c: tiger.grm
	yacc -dv tiger.grm

lex.yy.c: tiger.lex
	lex tiger.lex

$(ODIR)/%.o: %.c $(DEPS)
	@mkdir -p $(@D)
	cc -c -o $@ $< $(CFLAGS)

clean:
	rm -f $(ODIR)/*.o *~ hobbes y.tab.* lex.yy.c y.output

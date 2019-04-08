.PHONY: clean tigger

CFLAGS = -g

IDIR = ../include
ODIR = obj

_DEPS = absyn.h errormsg.h prabsyn.h symbol.h tokens.h types.h env.h parse.h semant.h table.h translate.h util.h
DEPS = $(patsubst %,$(IDIR)/%, $(_DEPS))

_OBJ = env.o parse.o prabsyn.o tigger.o table.o util.o absyn.o errormsg.o semant.o symbol.o types.o lex.yy.o y.tab.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

tigger: $(OBJ)
	cc -o $@ $^ $(CFLAGS)

y.tab.c: tiger.grm
	yacc -dv tiger.grm

lex.yy.c: tiger.lex
	lex tiger.lex

$(ODIR)/%.o: %.c $(DEPS)
	@mkdir -p $(@D)
	cc -c -o $@ $< $(CFLAGS)

clean:
	rm -f $(ODIR)/*.o *~ tigger y.tab.c lex.yy.c y.output

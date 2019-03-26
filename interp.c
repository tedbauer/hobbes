#include "util.h"
#include "ast.h"

typedef struct table *Table_;
typedef struct option *Option_;

struct table {
	string id;
	int value;
	Table_ tail;
};

struct option {
	enum { Some_, None_ } kind;
	int val;
};

Option_ Some(int val)
{
	Option_ o = checked_malloc(sizeof(*o));
	o->kind = Some_;
	o->val = val;
	return o;
}

Option_ None()
{
	Option_ o = checked_malloc(sizeof(*o));
	o->kind = None_;
	return o;
}

Table_ Table(string id, int value, struct table *tail)
{
	Table_ t = checked_malloc(sizeof(*t));
	t->id = id;
	t->value = value;
	t->tail = tail;
	return t;
}

Table_ interpStm(A_stm s, Table_ t);

struct IntAndTable {
	int i;
	Table_ t;
};

Table_ update(Table_ t, string key, int val)
{
	return Table(key, val, t);
}

Option_ lookup(Table_ t, string key)
{
	if (t == NULL) {
		return None();
	} else {
		if (key == t->id) {
			return Some(t->value);
		} else {
			return lookup(t->tail, key);
		}
	}
}

int try_lookup(Table_ t, string id) {
	Option_ result = lookup(t, id);
	if (result->kind == Some_) {
		return result->val;
	} else {
		printf("Error: unbound variable.\n");
		assert(0);
	}
}

struct IntAndTable interpExp(A_exp e, Table_ t)
{
	switch (e->kind) {
		case A_idExp: {
			struct IntAndTable it = {
				.i = try_lookup(t, e->u.id),
				.t = t
			};
			return it;
		}
		case A_numExp: {
			struct IntAndTable it = {
				.i = e->u.num,
				.t = t
			};
			return it;
		}
		case A_opExp:
			assert(0);
		case A_eseqExp:
			assert(0);
		default:
			assert(0);
	}
}

Table_ printExprs(A_expList e, Table_ t)
{
	switch (e->kind) {
		case A_pairExpList: {
			struct IntAndTable it = interpExp(e->u.pair.head, t);
			int val = it.i;
			Table_ t2 = it.t;
			printf("%d\n", val);
			return printExprs(e->u.pair.tail, t2);
		}
		case A_lastExpList: {
			struct IntAndTable it = interpExp(e->u.last, t);
			int val = it.i;
			Table_ t2 = it.t;
			printf("%d\n", val);
			return t2;
		}
		default:
			assert(0);
	}
	assert(0);
}

Table_ interpStm(A_stm s, Table_ t)
{
	switch (s->kind) {
		case A_compoundStm: {
			A_stm stm1 = s->u.compound.stm1;
			A_stm stm2 = s->u.compound.stm2;
			Table_ t2 = interpStm(stm1, t);
			return interpStm(stm2, t2);
		}
		case A_assignStm: {
			string id = s->u.assign.id;
			struct IntAndTable it = interpExp(s->u.assign.exp, t);
			int val = it.i;
			Table_ t2 = it.t;
			return Table(id, val, t2);
		}
		case A_printStm:
			break;
		default:
			assert(0);
	}
	assert(0);
}

int max_args(A_stm stm)
{
	assert(0);
	return 0;
}

void interp(A_stm stm)
{
	interpStm(stm, NULL);
}

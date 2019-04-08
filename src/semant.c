#include "../include/util.h"
#include "../include/errormsg.h"
#include "../include/symbol.h"
#include "../include/absyn.h"
#include "../include/translate.h"
#include "../include/types.h"
#include "../include/semant.h"

struct expty expTy(Tr_exp exp, Ty_ty ty) {
	struct expty e;
	e.exp = exp;
	e.ty = ty;
	return e;
}

struct expty transOpExp(S_table venv, S_table tenv, A_exp a)
{
	A_oper oper = a->u.op.oper;
	struct expty left = transExp(venv, tenv, a->u.op.left);
	struct expty right = transExp(venv, tenv, a->u.op.right);
	if (oper == A_plusOp) {
		if (left.ty->kind != Ty_int) {
			EM_error(a->u.op.left->pos, "integer required");
		}
		if (right.ty->kind != Ty_int) {
			EM_error(a->u.op.right->pos, "integer required");
		}
		return expTy(NULL, Ty_Int());
	}
}

struct expty transCallExp(S_table venv, S_table tenv, A_exp a)
{

}

struct expty transVar(S_table venv, S_table tenv, A_var v)
{
	switch (v->kind) {
		default: assert(0);
	}
}

struct expty transExp(S_table venv, S_table tenv, A_exp a)
{
	switch (a->kind) {
		case A_varExp: return transVar(venv, tenv, a->u.var);
		case A_nilExp: return expTy(NULL, Ty_Nil());
		case A_intExp: return expTy(NULL, Ty_Int());
		case A_stringExp: return expTy(NULL, Ty_String());
		case A_callExp: assert(0);
		case A_opExp: return transOpExp(venv, tenv, a);
		case A_recordExp: assert(0);
		case A_seqExp: assert(0);
		case A_assignExp: assert(0);
		case A_ifExp: assert(0);
		case A_whileExp: assert(0);
		case A_forExp: assert(0);
		case A_breakExp: assert(0);
		case A_letExp: assert(0);
		case A_arrayExp: assert(0);
		default: assert(0);
	}
	assert(0);
}

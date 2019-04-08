#include "../include/util.h"
#include "../include/errormsg.h"
#include "../include/symbol.h"
#include "../include/absyn.h"
#include "../include/translate.h"
#include "../include/types.h"
#include "../include/semant.h"
#include "../include/env.h"

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

struct expty transSimpleVar(S_table venv, S_table tenv, A_var a)
{
	E_enventry x = S_look(venv, a->u.simple);
	if (x && x->kind == E_varEntry) {
		return expTy(NULL, actual_ty(x->u.var.ty));
	} else {
		EM_error(a->pos, "undefined variable %s", S_name(a->u.simple));
		return expTy(NULL, Ty_Int());
	}
}

struct expty transVar(S_table venv, S_table tenv, A_var v)
{
	switch (v->kind) {
		case A_simpleVar: transSimpleVar(venv, tenv, v);
		case A_fieldVar: assert(0);
		case A_subscriptVar: assert(0);
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

void SEM_transProg(A_exp prog)
{
	// Typecheck the AST
	transExp(E_base_venv(), E_base_tenv(), prog);
}

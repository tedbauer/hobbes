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

/* FIXME(ted): why is the RHS of a type declaration wrapped
 * in an A_nametyList? Here, I'm making the assumption that
 * these are _always_ singleton lists -- a fact that is weird
 * and could be wrong. */
void transTyDec(S_table venv, S_table tenv, A_dec d)
{
	assert(d->kind == A_typeDec);
	S_enter(tenv, d->u.type->head->name, d->u.type->head->ty);
}

void transVarDec(S_table venv, S_table tenv, A_dec d)
{
	assert(d->kind == A_varDec);
	struct expty e = transExp(venv, tenv, d->u.var.init);
	S_enter(venv, d->u.var.var, E_VarEntry(e.ty));
}

void transDec(S_table venv, S_table tenv, A_dec d)
{
	switch(d->kind) {
		case A_functionDec: assert(0);
		case A_varDec: transVarDec(venv, tenv, d); break;
		case A_typeDec: transTyDec(venv, tenv, d); break;
		default: assert(0);
	}
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
	assert(0);
	return expTy(NULL, NULL);
}

struct expty transCallExp(S_table venv, S_table tenv, A_exp a)
{
	assert(0);
	return expTy(NULL, NULL);
}

struct expty transFieldVar(S_table venv, S_table tenv, A_var v)
{
	assert(v->kind == A_fieldVar);
	S_symbol accessorSym = v->u.field.sym;
	Ty_ty recordType = S_look(venv, v->u.field.var->u.simple);
	Ty_ty accessorType = S_look(venv, accessorSym);
	if (recordType->kind != Ty_record) {
		EM_error(v->pos, "accessed field of non-record %s", S_name(v->u.simple));
	}
	Ty_fieldList fields = recordType->u.record;
	Ty_field currField = fields->head;
	while (currField != NULL) {
		if (currField->name == accessorSym) {
			break;
		}
	}
	return expTy(NULL, currField->ty);
}

struct expty transSimpleVar(S_table venv, S_table tenv, A_var v)
{
	E_enventry x = S_look(venv, v->u.simple);
	if (x && x->kind == E_varEntry) {
		return expTy(NULL, actual_ty(x->u.var.ty, tenv));
	} else {
		EM_error(v->pos, "undefined variable %s", S_name(v->u.simple));
		return expTy(NULL, Ty_Int());
	}
}

struct expty transVar(S_table venv, S_table tenv, A_var v)
{
	switch (v->kind) {
		case A_simpleVar: return transSimpleVar(venv, tenv, v);
		case A_fieldVar: return transFieldVar(venv, tenv, v);
		case A_subscriptVar: assert(0);
		default: assert(0);
	}
}

void transDecs(S_table venv, S_table tenv, A_decList decs)
{
	if (decs) {
		transDec(venv, tenv, decs->head);
		transDecs(venv, tenv, decs->tail);
	}
}

struct expty transLetExp(S_table venv, S_table tenv, A_exp a)
{
	transDecs(venv, tenv, a->u.let.decs);
	return transExp(venv, tenv, a->u.let.body);
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
		case A_letExp: return transLetExp(venv, tenv, a);
		case A_arrayExp: return expTy(NULL, Ty_Array(S_look(tenv, a->u.array.typ)));
		default: assert(0);
	}
	assert(0);
}

void SEM_transProg(A_exp prog)
{
	// Typecheck the AST
	transExp(E_base_venv(), E_base_tenv(), prog);
	printf("Program typechecks!\n");
}

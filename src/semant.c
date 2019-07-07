#include "../include/util.h"
#include "../include/errormsg.h"
#include "../include/symbol.h"
#include "../include/absyn.h"
#include "../include/translate.h"
#include "../include/types.h"
#include "../include/semant.h"
#include "../include/env.h"

struct expty expTy(Tr_exp exp, Ty_ty ty)
{
	struct expty e;
	e.exp = exp;
	e.ty = ty;
	return e;
}

Ty_ty transRecordTy(S_table tenv, A_ty a)
{
	assert(a->kind == A_recordTy);
	Ty_fieldList* previousTail = NULL;
	Ty_fieldList resultFieldList;
	A_fieldList currFieldList = a->u.record;
	while (currFieldList) {
		S_symbol currFieldName = currFieldList->head->name;
		Ty_ty currFieldType = actual_ty(S_look(tenv, currFieldList->head->typ), tenv);
		Ty_field newHead = Ty_Field(currFieldName, currFieldType);
		Ty_fieldList newFieldList = Ty_FieldList(newHead, NULL);
		if (previousTail) {
			*previousTail = newFieldList;
			previousTail = &((*previousTail)->tail);
		} else {
			previousTail = &newFieldList;
			resultFieldList = newFieldList;
		}
		currFieldList = currFieldList->tail;
	}
	return Ty_Record(resultFieldList);
}

Ty_ty transTy(S_table tenv, A_ty a)
{
	switch (a->kind) {
		case A_nameTy:
			return actual_ty(S_look(tenv, a->u.name), tenv);
		case A_recordTy: 
			return transRecordTy(tenv, a);
		case A_arrayTy: 
			return Ty_Array(actual_ty(S_look(tenv, a->u.array), tenv));
	}
	assert(0);
}

/* FIXME(ted): why is the RHS of a type declaration wrapped
 * in an A_nametyList? Here, I'm making the assumption that
 * these are _always_ singleton lists -- a fact that is weird
 * and could be wrong.
 * One hypothesis: maybe one can parse type declarations
 * in such a way that could make use of A_nametyList? */
void transTyDec(S_table venv, S_table tenv, A_dec d)
{
	assert(d->kind == A_typeDec);
	Ty_ty t = transTy(tenv, d->u.type->head->ty);
	S_enter(tenv, d->u.type->head->name, t);
}

void printTenvBinding(S_symbol sym, Ty_ty t)
{
	printf("[%s: ", S_name(sym));
	Ty_print(t);
	printf("]\n");
}

bool typesEqual(S_table tenv, Ty_ty t1, Ty_ty t2)
{
	if (t1->kind == Ty_array && t2->kind == Ty_array) {
		S_dump(tenv, printTenvBinding);
		Ty_ty t1_base = actual_ty(t1->u.array, tenv);
		Ty_ty t2_base = actual_ty(t2->u.array, tenv);
		return typesEqual(tenv, t1_base, t2_base);
	} else if (t1->kind == t2->kind) { /* Primitives should fall under here */
		return TRUE;
	} else {
		return FALSE;
	}
	assert(0);
	return FALSE;
}

void transVarDec(S_table venv, S_table tenv, A_dec d)
{
	assert(d->kind == A_varDec);
	struct expty e = transExp(venv, tenv, d->u.var.init);
	if (d->u.var.typ) {
		Ty_ty varLabel = actual_ty(S_look(tenv, d->u.var.typ), tenv); 
		Ty_ty expType = actual_ty(e.ty, tenv);
		if (!typesEqual(tenv, varLabel, expType)) {
			EM_error(d->pos, "Type label mismatch");
		}
	}
	S_enter(venv, d->u.var.var, E_VarEntry(e.ty));
}

void transDec(S_table venv, S_table tenv, A_dec d)
{
	switch(d->kind) {
		case A_functionDec:
			assert(0);
		case A_varDec: 
			transVarDec(venv, tenv, d);
			S_dump(tenv, printTenvBinding);
			break;
		case A_typeDec:
			transTyDec(venv, tenv, d);
			S_dump(tenv, printTenvBinding);
			break;
		default: assert(0);
	}
}

struct expty transCallExp(S_table venv, S_table tenv, A_exp a)
{
	assert(0);
	return expTy(NULL, NULL);
}

struct expty transLetExp(S_table venv, S_table tenv, A_exp a)
{
	S_beginScope(venv);
	S_beginScope(tenv);
	A_decList d;
	for (d = a->u.let.decs; d; d = d->tail) {
		transDec(venv, tenv, d->head);
	}
	struct expty exp = transExp(venv, tenv, a->u.let.body);
	S_endScope(venv);
	S_endScope(tenv);
	return exp;
}

Ty_ty findFieldTy(Ty_ty recordTy, S_symbol fieldName, A_pos pos)
{
	assert(recordTy->kind == Ty_record);
	Ty_fieldList fields = recordTy->u.record;
	while (fields) {
		if (fields->head->name == fieldName) {
			return fields->head->ty;
		}
		fields = fields->tail;
	}
	EM_error(pos, "tried to access non-existent field");
}

struct expty transAssignExp(S_table venv, S_table tenv, A_exp a)
{
	switch (a->u.var->kind) {
		case A_simpleVar:
			return expTy(NULL, S_look(venv, a->u.var->u.simple));
		case A_subscriptVar:
			assert(0);
		case A_fieldVar:
			{
				//FIXME: recordTy is null. Dump venv and fix
				Ty_ty recordTy = transVar(venv, tenv, a->u.var->u.field.var).ty;
				printf("This is: ");
				Ty_print(recordTy);
				printf("\n");
				return expTy(NULL, findFieldTy(recordTy, a->u.var->u.field.sym, a->pos));
			}
	}
}

/* TODO(ted): abstract, this shares a lot of logic w/ transRecordTy */
struct expty transRecordExp(S_table venv, S_table tenv, A_exp a)
{
	// FIXME: check against provided record type (not just label)
	A_efieldList currEField = a->u.record.fields;
	Ty_fieldList* prevTail = NULL;
	Ty_fieldList resultFieldTypes;
	while (currEField) {
		S_symbol fieldName = currEField->head->name;
		Ty_ty fieldType = transExp(venv, tenv, currEField->head->exp).ty;
		Ty_field newHead = Ty_Field(fieldName, fieldType);
		Ty_fieldList newFieldList = Ty_FieldList(newHead, NULL);
		if (prevTail) {
			*prevTail = newFieldList;
			prevTail = &(newFieldList->tail);
		} else {
			prevTail = &newFieldList;
			resultFieldTypes = newFieldList;
		}
		currEField = currEField->tail;
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
		case A_simpleVar: 
			return transSimpleVar(venv, tenv, v);
		case A_fieldVar: 
			return transFieldVar(venv, tenv, v);
		case A_subscriptVar: 
			assert(0);
		default: 
			assert(0);
	}
}

struct expty transArrayExp(S_table venv, S_table tenv, A_exp a)
{
	Ty_ty initExpTy = transExp(venv, tenv, a->u.array.init).ty;
	Ty_ty arrayTy = actual_ty(S_look(tenv, a->u.array.typ), tenv);
	Ty_ty arrayBaseTy = arrayTy->u.array;
	if (typesEqual(tenv, initExpTy, arrayBaseTy)) {
		return expTy(NULL, Ty_Array(initExpTy));
	} else {
		EM_error(a->pos, "Array type incompatible with init type");
	}
}

struct expty transExp(S_table venv, S_table tenv, A_exp a)
{
	switch (a->kind) {
		case A_varExp: 
			return transVar(venv, tenv, a->u.var);
		case A_nilExp: 
			return expTy(NULL, Ty_Nil());
		case A_intExp: 
			return expTy(NULL, Ty_Int());
		case A_stringExp: 
			return expTy(NULL, Ty_String());
		case A_callExp: 
			assert(0);
		case A_opExp: 
			return transOpExp(venv, tenv, a);
		case A_recordExp: 
			return transRecordExp(venv, tenv, a);
		case A_seqExp: 
			transExp(venv, tenv, a->u.seq->head);
			return transExp(venv, tenv, A_SeqExp(a->pos, a->u.seq->tail));
		case A_assignExp: 
			return transAssignExp(venv, tenv, a);
		case A_ifExp: 
			assert(0);
		case A_whileExp: 
			assert(0);
		case A_forExp: 
			assert(0);
		case A_breakExp: 
			assert(0);
		case A_letExp: 
			return transLetExp(venv, tenv, a);
		case A_arrayExp: 
			return transArrayExp(venv, tenv, a);
		default: assert
			 (0);
	}
	assert(0);
}

void SEM_transProg(A_exp prog)
{
	// Typecheck the AST
	S_table tenv = E_base_tenv();
	transExp(E_base_venv(), tenv, prog);
	S_dump(tenv, printTenvBinding);
}

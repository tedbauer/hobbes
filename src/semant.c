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

Ty_fieldList constructFieldList(S_table tenv, A_fieldList fieldList)
{
	if (fieldList) {
		S_symbol fieldName = fieldList->head->name;
		Ty_ty fieldType = actual_ty(S_look(tenv, fieldList->head->typ), tenv);
		Ty_field newHead = Ty_Field(fieldName, fieldType);
		return Ty_FieldList(newHead, constructFieldList(tenv, fieldList->tail));
	} else {
		return NULL;
	}
}

Ty_ty transRecordTy(S_table tenv, A_ty a)
{
	assert(a->kind == A_recordTy);
	Ty_fieldList resultFieldList = constructFieldList(tenv, a->u.record);
	printf("Testing resultFieldList...\n");
	while (resultFieldList) {
		printf("Constructed field list contains field %s:", S_name(resultFieldList->head->name));
		Ty_print(resultFieldList->head->ty);
		printf("\n");
		resultFieldList = resultFieldList->tail;
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

bool recordsEqual(S_table tenv, Ty_ty t1, Ty_ty t2)
{
	assert(t1->kind == Ty_record);
	assert(t2->kind == Ty_record);
	return TRUE; // FIXME
}

/* FIXME(ted): compares records for structural equivalence,
 * so records can match their type labels; however, this should
 * not be allowed -- must augment. */
bool typesEqual(S_table tenv, Ty_ty t1, Ty_ty t2)
{
	if (t1->kind == Ty_array && t2->kind == Ty_array) {
		dumpTenv(tenv);
		Ty_ty t1_base = actual_ty(t1->u.array, tenv);
		Ty_ty t2_base = actual_ty(t2->u.array, tenv);
		return typesEqual(tenv, t1_base, t2_base);
	} else if (t1->kind == Ty_record && t2->kind == Ty_record) {
		return recordsEqual(tenv, actual_ty(t1, tenv), actual_ty(t2, tenv));
	}
	else if (t1->kind == t2->kind) { /* Primitives should fall under here */
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
	printf("Successful var dec.\n");
	S_enter(venv, d->u.var.var, E_VarEntry(e.ty));
}

void transDec(S_table venv, S_table tenv, A_dec d)
{
	switch(d->kind) {
		case A_functionDec:
			assert(0);
		case A_varDec: 
			transVarDec(venv, tenv, d);
			dumpTenv(tenv);
			break;
		case A_typeDec:
			transTyDec(venv, tenv, d);
			dumpTenv(tenv);
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
		printf("%s\n", S_name(fields->head->name));
		if (fields->head->name == fieldName) {
			printf("Found!!!\n");
			return fields->head->ty;
		}
		fields = fields->tail;
	}
	EM_error(pos, "tried to access non-existent field");
}

struct expty transAssignExp(S_table venv, S_table tenv, A_exp a)
{
	assert(a->kind == A_assignExp);
	Ty_ty expType = transExp(venv, tenv, a->u.assign.exp).ty;
	Ty_ty varType;
	switch (a->u.assign.var->kind) {
		case A_simpleVar:
			varType = actual_ty(S_look(venv, a->u.var->u.simple), tenv);
			break;
		case A_subscriptVar:
			assert(0);
		case A_fieldVar:
			{
				Ty_ty recordTy = transVar(venv, tenv, a->u.assign.var->u.field.var).ty;
				varType = findFieldTy(recordTy, a->u.assign.var->u.field.sym, a->pos);
				break;
			}
	}
	if (typesEqual(tenv, varType, expType)) {
		printf("Typechecked the assign.\n");
		return expTy(NULL, Ty_Void());
	} else {
		EM_error(a->pos, "LHS did not match type of RHS in assign exp");
	}
}

Ty_fieldList constructFieldListE(S_table venv, S_table tenv, A_efieldList efieldList)
{
	if (efieldList) {
		S_symbol efieldName = efieldList->head->name;
		Ty_ty efieldType = (transExp(venv, tenv, efieldList->head->exp)).ty;
		Ty_field newHead = Ty_Field(efieldName, efieldType);
		return Ty_FieldList(newHead, constructFieldListE(venv, tenv, efieldList->tail));
	} else {
		return NULL;
	}
}


/* TODO(ted): abstract, this shares a lot of logic w/ transRecordTy */
struct expty transRecordExp(S_table venv, S_table tenv, A_exp a)
{
	// FIXME: check against provided record type (not just label)
	Ty_fieldList resultFieldTypes = constructFieldListE(venv, tenv, a->u.record.fields);
	return expTy(NULL, Ty_Record(resultFieldTypes));
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
			printf("Typechecked head of seq.\n");
			transExp(venv, tenv, a->u.seq->head);
			if (a->u.seq->tail) {
				return transExp(venv, tenv, A_SeqExp(a->pos, a->u.seq->tail));
			} else {
				return expTy(NULL, Ty_Void());
			}
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
	dumpTenv(tenv);
}

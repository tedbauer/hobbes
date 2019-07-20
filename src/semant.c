#include "../include/util.h"
#include "../include/errormsg.h"
#include "../include/symbol.h"
#include "../include/absyn.h"
#include "../include/translate.h"
#include "../include/types.h"
#include "../include/semant.h"
#include "../include/env.h"

bool typesEqual(S_table tenv, Ty_ty t1, Ty_ty t2);

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
		Ty_ty fieldType =
		    actual_ty(S_look(tenv, fieldList->head->typ), tenv);
		Ty_field newHead = Ty_Field(fieldName, fieldType);
		return Ty_FieldList(newHead,
				    constructFieldList(tenv, fieldList->tail));
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
		printf("Constructed field list contains field %s:",
		       S_name(resultFieldList->head->name));
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

Ty_ty findFieldTy(Ty_ty recordTy, S_symbol fieldName)
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
	return NULL;
}

bool recordsEqual(S_table tenv, Ty_ty t1, Ty_ty t2)
{
	assert(t1->kind == Ty_record);
	assert(t2->kind == Ty_record);

	Ty_fieldList t1CurrFieldList = t1->u.record;
	printf("About to check these records outtt\n");
	while (t1CurrFieldList) {
		Ty_ty t1CurrFieldType =
		    findFieldTy(t1, t1CurrFieldList->head->name);
		Ty_ty t2CurrFieldType =
		    findFieldTy(t2, t1CurrFieldList->head->name);
		printf("Checked a fieldddd\n");
		if (!t1CurrFieldType || !t2CurrFieldType) {
			return FALSE;
		}
		if (!typesEqual(tenv, t1CurrFieldType, t2CurrFieldType)) {
			return FALSE;
		}
		t1CurrFieldList = t1CurrFieldList->tail;
	}
	return TRUE;		// FIXME
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
		printf("Comparing 2 records rn.\n");
		return recordsEqual(tenv, actual_ty(t1, tenv),
				    actual_ty(t2, tenv));
	} else {	/* Primitives should fall under here */
		return t1->kind == t2->kind;
	}
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
	switch (d->kind) {
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
	default:
		assert(0);
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

Ty_fieldList constructFieldListE(S_table venv, S_table tenv, A_efieldList efieldList)
{
	if (efieldList) {
		S_symbol efieldName = efieldList->head->name;
		Ty_ty efieldType =
		    (transExp(venv, tenv, efieldList->head->exp)).ty;
		Ty_field newHead = Ty_Field(efieldName, efieldType);
		return Ty_FieldList(newHead,
				    constructFieldListE(venv, tenv,
							efieldList->tail));
	} else {
		return NULL;
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

bool containsAssignTo(A_exp a, S_symbol s)
{
	switch (a->kind) {
	case A_assignExp:
		if (a->u.assign.var->kind == A_simpleVar) {
			return a->u.assign.var->u.simple == s;
		}
	case A_varExp:
	case A_nilExp:
	case A_intExp:
	case A_stringExp:
	case A_callExp:
	case A_opExp:
	case A_recordExp:
	case A_seqExp:
	case A_ifExp:
	case A_whileExp:
	case A_forExp:
	case A_breakExp:
	case A_letExp:
	case A_arrayExp:
		return FALSE;
	default:
		assert(0);
	}
	assert(0);
}

struct expty transForExp(S_table venv, S_table tenv, A_exp a)
{
	Ty_ty loType = transExp(venv, tenv, a->u.forr.lo).ty;
	Ty_ty hiType = transExp(venv, tenv, a->u.forr.hi).ty;
	S_enter(venv, a->u.forr.var, E_VarEntry(Ty_Int()));
	Ty_ty bodyType = transExp(venv, tenv, a->u.forr.body).ty;
	if (loType->kind != Ty_int) {
		EM_error(a->pos,
			 "Start index in for loop must be int");
	}
	if (hiType->kind != Ty_int) {
		EM_error(a->pos,
			 "End index in for loop must be int");
	}
	if (bodyType->kind != Ty_void) {
		EM_error(a->pos,
			 "For loop body must be type void");
	}
	if (containsAssignTo(a->u.forr.body, a->u.forr.var)) {
		EM_error(a->pos,
			 "Cannot assign to index in for loop");
	}
	return expTy(NULL, Ty_Void());
}

struct expty transWhileExp(S_table venv, S_table tenv, A_exp a)
{
	Ty_ty testType = transExp(venv, tenv, a->u.whilee.test).ty;
	Ty_ty bodyType = transExp(venv, tenv, a->u.whilee.body).ty;
	if (testType->kind != Ty_int) {
		EM_error(a->pos, "While condition must be int");
	}
	if (bodyType->kind != Ty_void) {
		EM_error(a->pos, "While body must be void");
	}
	return expTy(NULL, Ty_Void());
}

struct expty transIfExp(S_table venv, S_table tenv, A_exp a)
{
	Ty_ty testType = transExp(venv, tenv, a->u.iff.test).ty;
	Ty_ty leftType = transExp(venv, tenv, a->u.iff.then).ty;
	Ty_ty rightType =
	    transExp(venv, tenv, a->u.iff.elsee).ty;
	if (testType->kind != Ty_int) {
		EM_error(a->pos, "If condition must be int");
	}
	if (!typesEqual(tenv, leftType, rightType)) {
		EM_error(a->pos,
			 "If branches' types must match");
	}
	return expTy(NULL, leftType);
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
			Ty_ty recordTy = transVar(venv, tenv,
						  a->u.assign.var->u.field.var).
			    ty;
			varType =
			    findFieldTy(recordTy, a->u.assign.var->u.field.sym);
			if (!varType) {
				EM_error(a->pos,
					 "tried to access non-existent field");
			}
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

struct expty transSeqExp(S_table venv, S_table tenv, A_exp a)
{
	transExp(venv, tenv, a->u.seq->head);
	if (a->u.seq->tail) {
		return transExp(venv, tenv,
				A_SeqExp(a->pos, a->u.seq->tail));
	} else {
		return expTy(NULL, Ty_Void());
	}
}

/* TODO(ted): abstract, this shares a lot of logic w/ transRecordTy */
struct expty transRecordExp(S_table venv, S_table tenv, A_exp a)
{
	// FIXME: check against provided record type (not just label)
	Ty_fieldList resultFieldTypes =
	    constructFieldListE(venv, tenv, a->u.record.fields);
	return expTy(NULL, Ty_Record(resultFieldTypes));
}

struct expty transOpExp(S_table venv, S_table tenv, A_exp a)
{
	A_oper oper = a->u.op.oper;
	struct expty left = transExp(venv, tenv, a->u.op.left);
	struct expty right = transExp(venv, tenv, a->u.op.right);
	bool arithOp = (
		oper == A_plusOp || oper == A_minusOp || oper == A_timesOp ||
		oper == A_divideOp
	);
	bool eqOp = (
		oper == A_eqOp || oper == A_neqOp
	);
	bool compOp = (
		oper == A_gtOp || oper == A_ltOp || oper == A_geOp ||
		oper == A_leOp
	);
	bool boolOp = FALSE; /* FIXME */
	if (arithOp) {
		if (left.ty->kind != Ty_int) {
			EM_error(a->u.op.left->pos, "integer required");
		}
		if (right.ty->kind != Ty_int) {
			EM_error(a->u.op.right->pos, "integer required");
		}
		return expTy(NULL, Ty_Int());
	} else if (eqOp) {
		if (left.ty->kind != right.ty->kind) {
			EM_error(a->u.op.left->pos, "operands must match types");
		}
		return expTy(NULL, Ty_Int());
	} else if (compOp) {
		if (left.ty->kind != right.ty->kind) {
			EM_error(a->u.op.left->pos, "operands must match types");
		}
		if (left.ty->kind != Ty_int || left.ty->kind != Ty_string) {
			EM_error(a->u.op.left->pos, "operand must be string or int");
		}
		if (right.ty->kind != Ty_int || right.ty->kind != Ty_string) {
			EM_error(a->u.op.right->pos, "operand must be string or int");
		}
		return expTy(NULL, Ty_Int());
	} else if (boolOp) {
		if (left.ty->kind != Ty_int) {
			EM_error(a->u.op.left->pos, "integer required");
		}
		if (right.ty->kind != Ty_int) {
			EM_error(a->u.op.right->pos, "integer required");
		}
		return expTy(NULL, Ty_Int());
	} else {
		assert(0);
	}
}

struct expty transVar(S_table venv, S_table tenv, A_var v)
{
	switch (v->kind) {
	case A_simpleVar:
		{
			E_enventry x = S_look(venv, v->u.simple);
			if (x && x->kind == E_varEntry) {
				return expTy(NULL, actual_ty(x->u.var.ty, tenv));
			} else {
				EM_error(v->pos, "undefined variable %s", S_name(v->u.simple));
				return expTy(NULL, Ty_Int());
			}
		}
	case A_fieldVar:
		{
			assert(v->kind == A_fieldVar);
			S_symbol accessorSym = v->u.field.sym;
			Ty_ty recordType = S_look(venv, v->u.field.var->u.simple);
			Ty_ty accessorType = S_look(venv, accessorSym);
			if (recordType->kind != Ty_record) {
				EM_error(v->pos, "accessed field of non-record %s",
					 S_name(v->u.simple));
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
	case A_subscriptVar:
		assert(0);
	default:
		assert(0);
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
		return transSeqExp(venv, tenv, a);
	case A_assignExp:
		return transAssignExp(venv, tenv, a);
	case A_ifExp:
		return transIfExp(venv, tenv, a);
	case A_whileExp:
		return transWhileExp(venv, tenv, a);
	case A_forExp:
		return transForExp(venv, tenv, a);
	case A_breakExp:
		return expTy(NULL, Ty_Void());
	case A_letExp:
		return transLetExp(venv, tenv, a);
	case A_arrayExp:
		return transArrayExp(venv, tenv, a);
	default:
		assert(0);
	}
	assert(0);
}

void SEM_transProg(A_exp prog)
{
	S_table tenv = E_base_tenv();
	S_table venv = E_base_venv();
	transExp(venv, tenv, prog);
	dumpTenv(tenv);
	dumpVenv(venv);
}

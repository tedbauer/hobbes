#include "../include/util.h"
#include "../include/errormsg.h"
#include "../include/symbol.h"
#include "../include/absyn.h"
#include "../include/translate.h"
#include "../include/types.h"
#include "../include/semant.h"
#include "../include/env.h"

/*
 * TODO:
 * - [ ] Implement check for break statement in for/while loop
 * - [ ] Detect recursive type cycles
 * - [ ] Make sure type equality is correct
 * - [ ] Better/more error messages
 * - [X] Subscript vars
 * - [ ] Finish binops
 * - [ ] Cleanup
 */

/* 
 * Special symbol stored in the variable environment that either:
 * - maps to an arbitrary E_enventry when typechecking _is_ occurring
 *   in a for/while loop
 * - maps to NULL when typechecking is _not_ occurring in a for/while
 *   loop
 */
//S_symbol IN_LOOP = S_Symbol("_IN_LOOP");

bool typesEqual(S_table tenv, Ty_ty t1, Ty_ty t2);

Ty_tyList transParams(S_table tenv, A_fieldList params)
{
	if (params) {
		Ty_ty paramType = actual_ty(S_look(tenv, params->head->typ), tenv);
		return Ty_TyList(paramType, transParams(tenv, params->tail));
	} else {
		return NULL;
	}
}

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
		Ty_ty fieldType = S_look(tenv, fieldList->head->typ);
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
	while (resultFieldList) {
		Ty_print(resultFieldList->head->ty);
		resultFieldList = resultFieldList->tail;
	}
	return Ty_Record(resultFieldList);
}

Ty_ty transTy(S_table tenv, A_ty a)
{
	switch (a->kind) {
	case A_nameTy:
		return Ty_Name(a->u.name, S_look(tenv, a->u.name));
	case A_recordTy:
		return transRecordTy(tenv, a);
	case A_arrayTy:
		return Ty_Array(actual_ty(S_look(tenv, a->u.array), tenv));
	}
	assert(0);
}

Ty_ty findFieldTy(Ty_ty recordTy, S_symbol fieldName)
{
	assert(recordTy->kind == Ty_record);
	Ty_fieldList fields = recordTy->u.record;
	while (fields) {
		if (fields->head->name == fieldName) {
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
	while (t1CurrFieldList) {
		Ty_ty t1CurrFieldType =
		    findFieldTy(t1, t1CurrFieldList->head->name);
		Ty_ty t2CurrFieldType =
		    findFieldTy(t2, t1CurrFieldList->head->name);
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

bool typesEqual(S_table tenv, Ty_ty t1, Ty_ty t2)
{
	if (t1->kind == Ty_array && t2->kind == Ty_array) {
		Ty_ty t1_base = actual_ty(t1->u.array, tenv);
		Ty_ty t2_base = actual_ty(t2->u.array, tenv);
		return typesEqual(tenv, t1_base, t2_base);
	} else if (t1->kind == Ty_record && t2->kind == Ty_record) {
		return recordsEqual(tenv, actual_ty(t1, tenv),
				    actual_ty(t2, tenv));
	} else {	/* Primitives should fall under here */
		return t1->kind == t2->kind;
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

void transTyDec(S_table venv, S_table tenv, A_dec d)
{
	assert(d->kind == A_typeDec);
	A_nametyList nametyList = d->u.type;
	
	A_nametyList currNametyList = nametyList;
	while (currNametyList) {
		A_namety currNametyListHead = currNametyList->head;
		Ty_ty emptyBinding = Ty_Name(currNametyListHead->name, NULL);
		S_enter(tenv, currNametyListHead->name, emptyBinding);
		currNametyList = currNametyList->tail;
	}

	A_nametyList currNametyList2 = nametyList;
	while (currNametyList2) {
		A_namety currNametyListHead = currNametyList2->head;
		Ty_ty currTy = transTy(tenv, currNametyListHead->ty);
		S_enter(tenv, currNametyListHead->name, currTy);
		currNametyList2 = currNametyList2->tail;
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
	S_enter(venv, d->u.var.var, E_VarEntry(e.ty));
}

void transFunDec(S_table venv, S_table tenv, A_dec d)
{
	assert(d->kind == A_functionDec);

	A_fundecList currFundecList = d->u.function;
	while (currFundecList) {
		A_fundec currFundec = currFundecList->head;
		Ty_tyList paramTypes = transParams(tenv, currFundec->params);
		Ty_ty returnType = actual_ty(S_look(tenv, currFundec->result), tenv);
		E_enventry funEntry = E_FunEntry(paramTypes, returnType);
		S_enter(venv, currFundec->name, funEntry);
		currFundecList = currFundecList->tail;
	}

	A_fundecList currFundecList2 = d->u.function;
	while (currFundecList2) {
		A_fundec currFundec = currFundecList2->head;

		A_fieldList currParams = currFundec->params;
		while (currParams) {
			Ty_ty paramType = S_look(tenv, currParams->head->typ);
			S_enter(venv, currParams->head->name, E_VarEntry(paramType));
			currParams = currParams->tail;
		}

		Ty_ty bodyType = transExp(venv, tenv, currFundec->body).ty;
		Ty_ty returnType = actual_ty(S_look(tenv, currFundec->result), tenv);
		if (bodyType != returnType) {
			EM_error(currFundec->pos, "return type must match type label");
		}
		currFundecList2 = currFundecList2->tail;
	}
}

struct expty transLetExp(S_table venv, S_table tenv, A_exp a)
{
	S_beginScope(venv);
	S_beginScope(tenv);
	A_decList d;
	for (d = a->u.let.decs; d; d = d->tail) {
		A_dec currDec = d->head;
		if (currDec->kind == A_functionDec) {
			transFunDec(venv, tenv, currDec);
		} else if (currDec->kind == A_varDec) {
			transVarDec(venv, tenv, currDec);
		} else if (currDec->kind == A_typeDec) {
			transTyDec(venv, tenv, currDec);
		} else {
			assert(0);
		}
	}
	struct expty exp = transExp(venv, tenv, a->u.let.body);
	S_endScope(venv);
	S_endScope(tenv);
	return exp;
}

struct expty transBreakExp(S_table venv, S_table tenv, A_exp a)
{
	// TODO: check if in for or while loop
	return expTy(NULL, Ty_Void());
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
}

struct expty transForExp(S_table venv, S_table tenv, A_exp a)
{
	Ty_ty loType = transExp(venv, tenv, a->u.forr.lo).ty;
	Ty_ty hiType = transExp(venv, tenv, a->u.forr.hi).ty;
	S_enter(venv, a->u.forr.var, E_VarEntry(Ty_Int()));
	S_beginScope(venv);
	Ty_ty bodyType = transExp(venv, tenv, a->u.forr.body).ty;
	S_endScope(venv);
	if (loType->kind != Ty_int) {
		EM_error(a->pos, "Start index in for loop must be int");
	}
	if (hiType->kind != Ty_int) {
		EM_error(a->pos, "End index in for loop must be int");
	}
	if (bodyType->kind != Ty_void) {
		EM_error(a->pos, "For loop body must be type void");
	}
	if (containsAssignTo(a->u.forr.body, a->u.forr.var)) {
		EM_error(a->pos, "Cannot assign to index in for loop");
	}
	return expTy(NULL, Ty_Void());
}

struct expty transWhileExp(S_table venv, S_table tenv, A_exp a)
{
	Ty_ty testType = transExp(venv, tenv, a->u.whilee.test).ty;
	Ty_ty bodyType = transExp(venv, tenv, a->u.whilee.body).ty;
	if (testType->kind != Ty_int) {
		EM_error(a->pos, "while condition must be int");
	}
	if (bodyType->kind != Ty_void) {
		EM_error(a->pos, "while body must be void");
	}
	return expTy(NULL, Ty_Void());
}

struct expty transIfExp(S_table venv, S_table tenv, A_exp a)
{
	Ty_ty condType = transExp(venv, tenv, a->u.iff.test).ty;
	Ty_ty leftType = transExp(venv, tenv, a->u.iff.then).ty;
	Ty_ty rightType = transExp(venv, tenv, a->u.iff.elsee).ty;
	if (condType->kind != Ty_int) {
		EM_error(a->pos, "if condition must be int");
	}
	if (!typesEqual(tenv, leftType, rightType)) {
		EM_error(a->pos, "if branches' types must match");
	}
	return expTy(NULL, leftType);
}

struct expty transAssignExp(S_table venv, S_table tenv, A_exp a)
{
	assert(a->kind == A_assignExp);
	Ty_ty expType = transExp(venv, tenv, a->u.assign.exp).ty;
	Ty_ty varType = transVar(venv, tenv, a->u.var).ty;
	if (typesEqual(tenv, varType, expType)) {
		return expTy(NULL, Ty_Void());
	} else {
		EM_error(a->pos, "mismatched types in assign statement");
		return expTy(NULL, Ty_Void());
	}
}

struct expty transSeqExp(S_table venv, S_table tenv, A_exp a)
{
	transExp(venv, tenv, a->u.seq->head);
	if (a->u.seq->tail) {
		return transExp(venv, tenv, A_SeqExp(a->pos, a->u.seq->tail));
	} else {
		return expTy(NULL, Ty_Void());
	}
}

Ty_fieldList transEFieldList(S_table venv, S_table tenv, A_efieldList efieldList)
{
	if (efieldList) {
		S_symbol efieldName = efieldList->head->name;
		Ty_ty efieldType = (transExp(venv, tenv, efieldList->head->exp)).ty;
		Ty_field newHead = Ty_Field(efieldName, efieldType);
		return Ty_FieldList(newHead, transEFieldList(venv, tenv, efieldList->tail));
	} else {
		return NULL;
	}
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

struct expty transCallExp(S_table venv, S_table tenv, A_exp a)
{
	assert(a->kind == A_callExp);
	S_symbol funcName = a->u.call.func;
	E_enventry funcEntry = S_look(venv, funcName);

	assert(funcEntry->kind == E_funEntry);
	Ty_tyList expectedArgTypes = funcEntry->u.fun.formals;
	A_expList actualArgExps = a->u.call.args;
	Ty_ty resultType = funcEntry->u.fun.result;
	Ty_tyList currExpectedArgTypeList = expectedArgTypes;
	A_expList currActualArgExpList = actualArgExps;

	while (currExpectedArgTypeList) {
		if (!currActualArgExpList) {
			EM_error(
				a->pos,
				"incorrect number of arguments supplied to function `%s'",
				funcName
			);
			return expTy(NULL, Ty_Int());
		}

		Ty_ty currExpectedArgType = currExpectedArgTypeList->head;
		Ty_ty currActualArgType = transExp(
			venv, tenv, 
			currActualArgExpList->head
		).ty;
		if (currExpectedArgType != currActualArgType) {
			EM_error(
				currActualArgExpList->head->pos,
				"incorrect types supplied to function call on `%s'",
				funcName
			);
			return expTy(NULL, Ty_Int());
		}
		currExpectedArgTypeList = currExpectedArgTypeList->tail;
		currActualArgExpList = currActualArgExpList->tail;
	}
	return expTy(NULL, resultType);
}

struct expty transSubscriptVar(S_table venv, S_table tenv, A_var v)
{
	assert(v->kind == A_subscriptVar);
	S_symbol arrName = v->u.subscript.var->u.simple;
	A_exp accessorExp = v->u.subscript.exp;
	Ty_ty arrType = S_look(venv, arrName);
	Ty_ty accessorType = transExp(venv, tenv, accessorExp).ty;

	if (arrType->kind != Ty_array) {
		EM_error(
			v->pos,
			"tried to subscript non-subscriptable variable `%s'",
			arrName
		);
		return expTy(NULL, Ty_Int());
	}

	if (accessorType->kind != Ty_int) {
		EM_error(
			v->pos,
			"tried to access array `%s' with non-int",
			arrName
		);
		return expTy(NULL, Ty_Int());
	}

	return expTy(NULL, arrType->u.array);
}

struct expty transFieldVar(S_table venv, S_table tenv, A_var v)
{
	assert(v->kind == A_fieldVar);
	A_var recordVar = v->u.field.var;
	S_symbol recordName = recordVar->u.simple;
	S_symbol fieldAccessorName = v->u.field.sym;
	Ty_ty recordType = S_look(venv, recordName);
	Ty_ty accessorType = S_look(venv, fieldAccessorName);

	if (recordType->kind != Ty_record) {
		EM_error(
			v->pos, 
			"accessed field of non-record `%s'",
			S_name(recordName)
		);
		return expTy(NULL, Ty_Int());
	}

	Ty_fieldList fieldTypes = recordType->u.record;
	Ty_field currField = fieldTypes->head;
	while (currField) {
		if (currField->name == fieldAccessorName) {
			break;
		}
	}

	if (currField) {
		return expTy(NULL, currField->ty);
	} else {
		EM_error(
			v->pos,
			"accessed nonexistent field `%s' from `%s'",
			fieldAccessorName, recordName
		);
		return expTy(NULL, Ty_Int());
	}
}

struct expty transSimpleVar(S_table venv, S_table tenv, A_var v)
{
	assert(v->kind == A_simpleVar);
	S_symbol simpleVar = v->u.simple;
	E_enventry simpleVarEntry = S_look(venv, simpleVar);

	if (simpleVarEntry && simpleVarEntry->kind == E_varEntry) {
		Ty_ty simpleVarEntryType = simpleVarEntry->u.var.ty;
		return expTy(NULL, simpleVarEntryType);
	} else {
		EM_error(
			v->pos,
			"undefined variable `%s'",
			S_name(simpleVar)
		);
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
		return transSubscriptVar(venv, tenv, v);
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
		return transCallExp(venv, tenv, a);
	case A_opExp:
		return transOpExp(venv, tenv, a);
	case A_recordExp:
		return expTy(NULL, Ty_Record(transEFieldList(venv, tenv, a->u.record.fields)));
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

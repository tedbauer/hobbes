#include "../include/util.h"
#include "../include/errormsg.h"
#include "../include/symbol.h"
#include "../include/absyn.h"
#include "../include/translate.h"
#include "../include/types.h"
#include "../include/semant.h"
#include "../include/env.h"
#include "../include/linkedlist.h"

/*
 * TODO:
 * - [ ] Implement check for break statement in for/while loop
 * - [ ] Detect recursive type cycles
 * - [ ] Make sure type equality is correct
 * - [ ] Better/more error messages
 * - [X] Subscript vars
 * - [ ] Finish binops
 * - [ ] Cleanup
 * - [ ] Resolve "name" type confusion.
 *
 * Known bugs:
 * - Record order is messed up
 */

bool inLoop = FALSE;

bool typesEqual(Ty_ty t1, Ty_ty t2)
{
	if (t1->kind == Ty_record && t2->kind == Ty_nil) {
		return TRUE;
	} else if (t1->kind == Ty_nil && t2->kind == Ty_record) {
		return TRUE;
	} else {
		return t1 == t2;
	}
}

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

Ty_ty transTy(S_table tenv, A_ty a)
{
	switch (a->kind) {
	case A_nameTy:
		return S_look(tenv, a->u.name);
	case A_recordTy:
		return Ty_Record(constructFieldList(tenv, a->u.record));
	case A_arrayTy:
		return Ty_Array(S_look(tenv, a->u.array));
	}
	assert(0);
}

struct expty transArrayExp(S_table venv, S_table tenv, A_exp a)
{
	Ty_ty initExpTy = transExp(venv, tenv, a->u.array.init).ty;
	Ty_ty arrayTy = actual_ty(S_look(tenv, a->u.array.typ), tenv);
	Ty_ty arrayBaseTy = actual_ty(arrayTy->u.array, tenv);
	if (typesEqual(initExpTy, arrayBaseTy)) {
		return expTy(NULL, arrayTy);
	} else {
		EM_error(a->pos, "array type incompatible with init type");
	}
}

void detectRecursiveTypeCycle(S_table tenv, S_symbol s, A_pos pos)
{
	LL_list seenNodes = LL_listOf(1, s);
	S_symbol origin = s;
	Ty_ty nameTy = S_look(tenv, s);
	Ty_ty currTy = nameTy->u.name.ty;
	while (currTy) {
		if (currTy->kind != Ty_name) {
			return;
		}
		if (LL_contains(seenNodes, currTy->u.name.sym)) {
			EM_error(pos, "type cycle detected");
			return;
		}

		seenNodes = LL_List(currTy->u.name.sym, seenNodes);
		currTy = currTy->u.name.ty;
	}
}

void transTyDec(S_table venv, S_table tenv, A_dec d)
{
	assert(d->kind == A_typeDec);
	A_nametyList nametyList = d->u.type;

	LL_list typeNames = LL_emptyList();

	A_nametyList currNametyList = nametyList;
	while (currNametyList) {
		S_symbol currNametyListHead = currNametyList->head->name;
		typeNames = LL_List(currNametyList->head->name, typeNames);
		Ty_ty emptyBinding = Ty_Name(currNametyListHead, NULL);
		S_enter(tenv, currNametyListHead, emptyBinding);
		dumpTenv(tenv);
		currNametyList = currNametyList->tail;
	}

	dumpTenv(tenv);
	A_nametyList currNametyList2 = nametyList;
	while (currNametyList2) {
		S_symbol currNametyListHead = currNametyList2->head->name;
		Ty_ty currTy = transTy(tenv, currNametyList2->head->ty);
		Ty_ty existingBinding = S_look(tenv, currNametyListHead);
		existingBinding->u.name.ty = currTy;
		currNametyList2 = currNametyList2->tail;
	}

	LL_list currNametyList3 = typeNames;
	while (currNametyList3) {
		S_symbol head = currNametyList3->head;
		detectRecursiveTypeCycle(tenv, head, d->pos);
		currNametyList3 = currNametyList3->tail;
	}
}

void transVarDec(S_table venv, S_table tenv, A_dec d)
{
	assert(d->kind == A_varDec);
	struct expty e = transExp(venv, tenv, d->u.var.init);
	Ty_print(e.ty);
	if (d->u.var.typ) {
		Ty_ty varLabel = actual_ty(S_look(tenv, d->u.var.typ), tenv);
		Ty_ty expType = actual_ty(e.ty, tenv);
		if (!typesEqual(varLabel, expType)) {
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
		if (currFundec->result) {
			Ty_ty returnType = actual_ty(S_look(tenv, currFundec->result), tenv);
			E_enventry funEntry = E_FunEntry(paramTypes, returnType);
			S_enter(venv, currFundec->name, funEntry);
		} else {
			E_enventry funEntry = E_FunEntry(paramTypes, Ty_Void());
			S_enter(venv, currFundec->name, funEntry);
		}
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
		Ty_ty returnType = NULL;
		if (currFundec->result) {
			returnType = actual_ty(S_look(tenv, currFundec->result), tenv);
		} else {
			returnType = Ty_Void();
		}
		if (!typesEqual(bodyType, returnType)) {
			EM_error(
				currFundec->pos, "return type must match type label; expected `%s', found `%s'",
				Ty_toString(returnType),
				Ty_toString(bodyType)
			);
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
	if (!inLoop) {
		EM_error(a->pos, "break statement must be inside for or while loop");
	}
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


struct expty transLoopBody(S_table venv, S_table tenv, A_exp body)
{
	bool inOuterLoop = inLoop;
	inLoop = TRUE;
	struct expty bodyType = transExp(venv, tenv, body);
	if (!inOuterLoop) {
		inLoop = FALSE;
	}
	return bodyType;
}

struct expty transForExp(S_table venv, S_table tenv, A_exp a)
{
	Ty_ty loType = transExp(venv, tenv, a->u.forr.lo).ty;
	Ty_ty hiType = transExp(venv, tenv, a->u.forr.hi).ty;
	S_enter(venv, a->u.forr.var, E_VarEntry(Ty_Int()));
	S_beginScope(venv);
	Ty_ty bodyType = transLoopBody(venv, tenv, a->u.forr.body).ty;
	S_endScope(venv);
	if (loType->kind != Ty_int) {
		EM_error(
			a->pos,
			"Start index in for loop must be int; found `%s'",
			Ty_toString(loType)
		);
	}
	if (hiType->kind != Ty_int) {
		EM_error(
			a->pos,
			"End index in for loop must be int; found `%s'",
			Ty_toString(hiType)
		);
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
	Ty_ty bodyType = transLoopBody(venv, tenv, a->u.whilee.body).ty;
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
	Ty_ty condType = actual_ty(transExp(venv, tenv, a->u.iff.test).ty, tenv);
	Ty_ty leftType = actual_ty(transExp(venv, tenv, a->u.iff.then).ty, tenv);
	Ty_ty rightType = actual_ty(transExp(venv, tenv, a->u.iff.elsee).ty, tenv);
	if (condType->kind != Ty_int) {
		EM_error(a->pos, "if condition must be int");
	}
	if (!typesEqual(leftType, rightType)) {
		EM_error(
			a->pos,
			"if branches' types must match; found `%s' and `%s'",
			Ty_toString(leftType),
			Ty_toString(rightType)
		);
	}
	return expTy(NULL, leftType);
}

struct expty transAssignExp(S_table venv, S_table tenv, A_exp a)
{
	assert(a->kind == A_assignExp);
	Ty_ty expType = actual_ty(transExp(venv, tenv, a->u.assign.exp).ty, tenv);
	Ty_ty varType = actual_ty(transVar(venv, tenv, a->u.var).ty, tenv);
	if (typesEqual(expType, varType)) {
		return expTy(NULL, Ty_Void());
	} else {
		EM_error(
			a->pos,
			"tried to assign expression of type `%s' to var of type `%s'",
			Ty_toString(expType),
			Ty_toString(varType)
		);
		return expTy(NULL, Ty_Void());
	}
}

struct expty transSeqExp(S_table venv, S_table tenv, A_exp a)
{
	assert(a->kind == A_seqExp);
	A_expList expList = a->u.seq;
	if (!expList) {
		return expTy(NULL, Ty_Void());
	}

	Ty_ty headType = actual_ty(transExp(venv, tenv, a->u.seq->head).ty, tenv);
	if (a->u.seq->tail) {
		return transExp(venv, tenv, A_SeqExp(a->pos, a->u.seq->tail));
	} else {
		return expTy(NULL, headType);
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

struct expty transRecordExp(S_table venv, S_table tenv, A_exp a)
{
	assert(a->kind == A_recordExp);
	Ty_ty recordTyp = actual_ty(S_look(tenv, a->u.record.typ), tenv);

	Ty_fieldList currFieldList = recordTyp->u.record;
	A_efieldList currEFieldList = a->u.record.fields;

	while (currFieldList) {
		Ty_field currField = currFieldList->head;
		S_symbol currFieldName = currField->name;
		Ty_ty currFieldTyp = actual_ty(currField->ty, tenv);

		A_efield currEField = currEFieldList->head;
		S_symbol currEFieldName = currEField->name;
		Ty_ty currEFieldTyp = transExp(venv, tenv, currEField->exp).ty;
		Ty_ty currEFieldTypResolved = actual_ty(currEFieldTyp, tenv);

		if (currFieldName != currEFieldName) {
			EM_error(
				a->pos,
				"mismatched field names in record creation"
			);
		}

		if (!typesEqual(currFieldTyp, currEFieldTypResolved)) {
			EM_error(
				a->pos,
				"mismatched types in record creation; expected `%s', found `%s'",
				Ty_toString(currFieldTyp),
				Ty_toString(currEFieldTypResolved)
			);
		}

		currFieldList = currFieldList->tail;
		currEFieldList = currEFieldList->tail;
	}

	return expTy(NULL, recordTyp);
}

struct expty transOpExp(S_table venv, S_table tenv, A_exp a)
{
	A_oper op = a->u.op.oper;
	Ty_ty leftTy = actual_ty(transExp(venv, tenv, a->u.op.left).ty, tenv);
	Ty_ty rightTy = actual_ty(transExp(venv, tenv, a->u.op.right).ty, tenv);
	bool boolOp = FALSE; /* FIXME */
	if (op == A_plusOp || op == A_minusOp || op == A_timesOp || op == A_divideOp) {
		if (leftTy->kind != Ty_int) EM_error(a->u.op.left->pos, "integer required");
		if (rightTy->kind != Ty_int) EM_error(a->u.op.right->pos, "integer required");

		return expTy(NULL, Ty_Int());
	} else if (op == A_eqOp || op == A_neqOp) {
		if (leftTy->kind != rightTy->kind)
			EM_error(
				a->u.op.left->pos,
				"operands must match types; found `%s' and `%s'",
				Ty_toString(leftTy),
				Ty_toString(rightTy)
			);

		return expTy(NULL, Ty_Int());
	} else if (op == A_gtOp || op == A_ltOp || op == A_geOp || op == A_leOp) {
		if (leftTy->kind != rightTy->kind)
			EM_error(
				a->u.op.left->pos,
				"operands must match types; found `%s' and `%s'",
				Ty_toString(leftTy),
				Ty_toString(rightTy)
			);

		if (leftTy->kind != Ty_int && leftTy->kind != Ty_string)
			EM_error(
				a->u.op.left->pos,
				"operand must be string or int; found `%s'",
				Ty_toString(leftTy)
			);

		if (rightTy->kind != Ty_int && rightTy->kind != Ty_string)
			EM_error(
				a->u.op.right->pos,
				"operand must be string or int; found `%s'",
				Ty_toString(rightTy)
			);

		return expTy(NULL, Ty_Int());
	} else if (boolOp) {
		if (leftTy->kind != Ty_int) EM_error(a->u.op.left->pos, "integer required");
		if (rightTy->kind != Ty_int) EM_error(a->u.op.right->pos, "integer required");

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
	Ty_ty resultType = actual_ty(funcEntry->u.fun.result, tenv);
	Ty_tyList currExpectedArgTypeList = expectedArgTypes;
	A_expList currActualArgExpList = actualArgExps;

	while (currExpectedArgTypeList) {
		if (!currActualArgExpList) {
			EM_error(
				a->pos,
				"incorrect number of arguments supplied to function `%s'",
				S_name(funcName)
			);
			return expTy(NULL, Ty_Int());
		}

		Ty_ty currExpectedArgType = currExpectedArgTypeList->head;
		Ty_ty currActualArgType = transExp(
			venv, tenv,
			currActualArgExpList->head
		).ty;
		if (!typesEqual(currExpectedArgType, currActualArgType)) {
			EM_error(
				currActualArgExpList->head->pos,
				"incorrect types supplied to function call on `%s'; expected `%s', found `%s'",
				S_name(funcName),
				Ty_toString(currExpectedArgType),
				Ty_toString(currActualArgType)
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
	Ty_ty arrType = actual_ty(S_look(venv, arrName), tenv);
	Ty_ty accessorType = transExp(venv, tenv, accessorExp).ty;

	if (arrType->kind != Ty_array) {
		EM_error(
			v->pos,
			"tried to subscript non-subscriptable variable `%s'",
			S_name(arrName)
		);
		return expTy(NULL, Ty_Int());
	}

	if (accessorType->kind != Ty_int) {
		EM_error(
			v->pos,
			"tried to access array `%s' with non-int",
			S_name(arrName)
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
	E_enventry recordTypeEntry = S_look(venv, recordName);
	Ty_ty recordType = recordTypeEntry->u.var.ty;

	if (recordType->kind != Ty_record) {
		EM_error(
			v->pos,
			"accessed field of non-record `%s'",
			S_name(recordName)
		);
		return expTy(NULL, Ty_Int());
	}

	Ty_fieldList currFieldList = recordType->u.record;
	while (currFieldList) {
		Ty_field currField = currFieldList->head;
		if (currField->name == fieldAccessorName) {
			break;
		}
		currFieldList = currFieldList->tail;
	}

	if (currFieldList) {
		Ty_ty currFieldTy = currFieldList->head->ty;
		Ty_ty resolvedCurrFieldTy = actual_ty(currFieldTy, tenv);
		return expTy(NULL, currFieldTy);
	} else {
		EM_error(
			v->pos,
			"accessed nonexistent field `%s' from `%s'",
			S_name(fieldAccessorName),
			S_name(recordName)
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
		Ty_ty simpleVarType = actual_ty(simpleVarEntry->u.var.ty, tenv);
		return expTy(NULL, simpleVarType);
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
		return expTy(NULL, Ty_Void());
	case A_intExp:
		return expTy(NULL, Ty_Int());
	case A_stringExp:
		return expTy(NULL, Ty_String());
	case A_callExp:
		return transCallExp(venv, tenv, a);
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
		return transBreakExp(venv, tenv, a);
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

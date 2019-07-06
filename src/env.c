#include "../include/util.h"
#include "../include/symbol.h"
#include "../include/types.h"
#include "../include/env.h"

E_enventry E_VarEntry(Ty_ty ty)
{
	E_enventry e = checked_malloc(sizeof(*e));
	e->kind = E_varEntry;
	e->u.var.ty = ty;
	return e;
}

E_enventry E_FunEntry(Ty_tyList formals, Ty_ty result)
{
	E_enventry e = checked_malloc(sizeof(*e));
	e->kind = E_funEntry;
	e->u.fun.formals = formals;
	e->u.fun.result = result;
	return e;
}

E_enventry E_PrintEntry()
{
	E_enventry e = checked_malloc(sizeof(*e));
	e->kind = E_funEntry;
	e->u.fun.formals = Ty_TyList(Ty_String(), NULL);
	e->u.fun.result = Ty_Nil();
	return e;
}

E_enventry E_FlushEntry()
{
	E_enventry e = checked_malloc(sizeof(*e));
	e->kind = E_funEntry;
	e->u.fun.formals = Ty_TyList(NULL, NULL);
	e->u.fun.result = Ty_Nil();
	return e;
}

E_enventry E_GetcharEntry()
{
	E_enventry e = checked_malloc(sizeof(*e));
	e->kind = E_funEntry;
	e->u.fun.formals = Ty_TyList(NULL, NULL);
	e->u.fun.result = Ty_String();
	return e;
}


E_enventry E_OrdEntry()
{
	E_enventry e = checked_malloc(sizeof(*e));
	e->kind = E_funEntry;
	e->u.fun.formals = Ty_TyList(Ty_String(), NULL);
	e->u.fun.result = Ty_String();
	return e;
}

E_enventry E_ChrEntry()
{
	E_enventry e = checked_malloc(sizeof(*e));
	e->kind = E_funEntry;
	e->u.fun.formals = Ty_TyList(Ty_Int(), NULL);
	e->u.fun.result = Ty_String();
	return e;
}

E_enventry E_SizeEntry()
{
	E_enventry e = checked_malloc(sizeof(*e));
	e->kind = E_funEntry;
	e->u.fun.formals = Ty_TyList(Ty_String(), NULL);
	e->u.fun.result = Ty_Int();
	return e;
}

E_enventry E_SubstringEntry()
{
	E_enventry e = checked_malloc(sizeof(*e));
	e->kind = E_funEntry;
	e->u.fun.formals = Ty_TyList(
		Ty_String(), Ty_TyList(
			Ty_Int(), Ty_TyList(Ty_Int(), NULL)
		)
	);
	return e;
}

E_enventry E_ConcatEntry()
{
	assert(0);
	E_enventry e = checked_malloc(sizeof(*e));
	e->kind = E_funEntry;
	return e;
}

E_enventry E_NotEntry()
{
	assert(0);
	E_enventry e = checked_malloc(sizeof(*e));
	e->kind = E_funEntry;
	return e;
}

E_enventry E_ExitEntry()
{
	assert(0);
	E_enventry e = checked_malloc(sizeof(*e));
	e->kind = E_funEntry;
	return e;
}

S_table E_base_tenv()
{
	S_table t = S_empty();
	S_enter(t, S_Symbol("int"), Ty_Int());
	S_enter(t, S_Symbol("string"), Ty_String());
	return t;
}

S_table E_base_venv()
{
	S_table t = S_empty();
	/*
	S_enter(t, S_Symbol("print"), E_PrintEntry());
	S_enter(t, S_Symbol("flush"), E_FlushEntry());
	S_enter(t, S_Symbol("getchar"), E_GetcharEntry());
	S_enter(t, S_Symbol("ord"), E_OrdEntry());
	S_enter(t, S_Symbol("chr"), E_ChrEntry());
	S_enter(t, S_Symbol("size"), E_SizeEntry());
	S_enter(t, S_Symbol("substring"), E_SubstringEntry());
	S_enter(t, S_Symbol("concat"), E_ConcatEntry());
	S_enter(t, S_Symbol("not"), E_NotEntry());
	S_enter(t, S_Symbol("exit"), E_ExitEntry());
	*/
	return t;
}

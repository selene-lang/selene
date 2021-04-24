#include <stdarg.h>
#include <string.h>

#include "array.h"
#include "syntax.h"
#include "memory.h"
#include "types.h"

static void unify_error(void);

static int isftv(Type t, int v);
static void bind(int v, Type t);

Array type_variables = {
	.p = NULL,
	.esize = sizeof(Type),
	.length = 0,
	.capacity = 0
};

static void
unify_error(void)
{
	exit(1);
}

static int
isftv(Type t, int v)
{
	switch (t.type) {
	case T_CON:
		for (int i = 0; i < t.arity; ++i)
			if (isftv(t.args[i], v))
				return 1;
		return 0;
	case T_VAR:
		return t.tvar == v;
	case T_FUN:
		for (int i = 0; i < t.arity; ++i)
			if (isftv(t.args[i], v))
				return 1;
		return isftv(*t.res, v);
	}
}

static void
bind(int v, Type t)
{
	if (isftv(t, v))
		unify_error();
	((Type *)type_variables.p)[v] = t;
}

Type
types_mktcon(char *s, int arity, ...)
{
	Type t;
	va_list ap;

	va_start(ap, arity);
	t.type = T_CON;
	t.arity = arity;
	t.args = emalloc(arity * sizeof(Type));
	for (int i = 0; i < arity; ++i)
		t.args[i] = va_arg(ap, Type);
	va_end(ap);
	return t;
}

void
types_unify(Type t1, Type t2)
{
	if (t1.type == T_CON && t2.type == T_CON) {
		if (!strcmp(t1.name, t2.name) && t1.arity == t2.arity) {
			for (int i = 0; i < t1.arity; ++i)
				types_unify(t1.args[i], t2.args[i]);
		} else {
			unify_error();
		}
	} else if (t1.type == T_FUN && t2.type == T_FUN) {
		if (t1.arity == t2.arity)
			for (int i = 0; i < t1.arity; ++i)
				types_unify(t1.args[i], t2.args[i]);
		types_unify(*t1.res, *t2.res);
	}
}

void
types_eval(Type *t)
{
	switch (t->type) {
	case T_FUN:
		types_eval(t->res); /* fallthrought */
	case T_CON:
		for (int i = 0; i < t->arity; ++i)
			types_eval(t->args + i);
		break;
	case T_VAR:
		*t = ((Type *)type_variables.p)[t->tvar];
		break;
	}
}

void
types_eval_expr(Expr *e)
{
	types_eval(&e->t);
	switch (e->type) {
	case E_VAR:
		for (int i = 0; i < e->polybind.len; ++i)
			types_eval(e->polybind.args + i);
		break;
	case E_NUM:
		break;
	case E_OP:
		types_eval_expr(e->left);
		types_eval_expr(e->right);
		break;
	case E_FUNCALL:
		types_eval_expr(e->left);
		for (int i = 0; i < e->args.length; ++i)
			types_eval_expr((Expr *)(e->args.p) + i);
	}
}

Type
types_fresh_tvar(void)
{
	Type t;

	t.tvar = type_variables.length;
	t.type = T_VAR;
	array_write(&type_variables, &t);
	return t;
}

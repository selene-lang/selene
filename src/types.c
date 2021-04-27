#include <stdarg.h>
#include <string.h>

#include "array.h"
#include "syntax.h"
#include "memory.h"
#include "types.h"

typedef struct {
	struct selem {
		int n;
		Type t;
	} *s;
	int length;
} Substitution;

static void unify_error(void);

static int isftv(Type t, int v);
static void bind(int v, Type t);
static void ftv(Array *a, Type t);

static void app_subst(Substitution s, Type *t);

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
	case T_FUN:
		if (isftv(*t.res, v))
			return 1; /* fallthrought */
	case T_CON:
		for (int i = 0; i < t.arity; ++i)
			if (isftv(t.args[i], v))
				return 1;
		return 0;
	case T_VAR:
		return t.tvar == v;
	}
}

static void
bind(int v, Type t)
{
	if (isftv(t, v))
		unify_error();
	((Type *)type_variables.p)[v] = t;
}

static void
ftv(Array *a, Type t)
{
	switch (t.type) {
	case T_VAR:
		for (int i = 0; i < a->length; ++i)
			if (((int *)a->p)[i] == t.tvar)
				return;
		array_write(a, &t.tvar);
		break;
	case T_FUN:
		ftv(a, *t.res); /* fallthrought */
	case T_CON:
		for (int i = 0; i < t.arity; ++i)
			ftv(a, t.args[i]);
		break;
	}
}

static void
app_subst(Substitution s, Type *t)
{
	switch (t->type) {
	case T_VAR:
		for (int i = 0; i < s.length; ++i)
			if (s.s[i].n == t->tvar) {
				*t = s.s[i].t;
				return;
			}
		break;
	case T_FUN:
		app_subst(s, t->res); /* fallthrought */
	case T_CON:
		for (int i = 0; i < s.length; ++i)
			app_subst(s, t->args + i);

	}
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

Type *
types_dup(Type t)
{
	Type *p;

	p = emalloc(sizeof(Type));
	*p = t;
	return p;
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

Scheme
types_gen(Type t)
{
	Scheme s;

	array_init(&s.bindings, sizeof(int));
	ftv(&s.bindings, t);
	s.t = t;
	return s;
}

Expr
types_inst(char *var, Scheme s)
{
	Expr e;
	Substitution subst;

	e.type = E_VAR;
	subst.length = s.bindings.length;
	subst.s = emalloc(subst.length * sizeof(struct selem));
	e.polybind.len = subst.length;
	e.polybind.args = emalloc(subst.length * sizeof(Type));
	for (int i = 0; i < subst.length; ++i) {
		subst.s[i].n = ((int *)s.bindings.p)[i];
		subst.s[i].t = types_fresh_tvar();
		e.polybind.args[i] = subst.s[i].t;
	}
	app_subst(subst, &s.t);
	e.t = s.t;
	e.name = var;
	return e;
}

Scheme
types_get_ctx(char *var)
{
	
}

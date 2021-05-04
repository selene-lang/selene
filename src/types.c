#include <stdarg.h>
#include <string.h>

#include "array.h"
#include "syntax.h"
#include "memory.h"
#include "types.h"

typedef struct {
	struct selem {
		int n;
		Type *t;
	} *s;
	int length;
} Substitution;

typedef struct {
	char *var;
	Scheme s;
} Celem;

static void unify_error(void);

static int isftv(Type t, int v);
static void bind(int v, Type t);
static void ftv(Array *a, Type t);

static Type *app_subst(Substitution s, Type *t);

Array type_variables = {
	.p = NULL,
	.esize = sizeof(Type),
	.length = 0,
	.capacity = 0
};

Array context = {
	.p = NULL,
	.esize = sizeof(Celem),
	.length = 0,
	.capacity = 0
};

Type types_int = {.type = T_CON, .args = NULL, .arity = 0, .name = "int"};
Type types_bool = {.type = T_CON, .args = NULL, .arity = 0, .name = "bool"};

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
			if (isftv(*t.args[i], v))
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
			ftv(a, *t.args[i]);
		break;
	}
}

static Type *
app_subst(Substitution s, Type *t)
{
	switch (t->type) {
	case T_VAR:
		for (int i = 0; i < s.length; ++i)
			if (s.s[i].n == t->tvar)
				return s.s[i].t;
		return t;
	case T_FUN:
		t->res = app_subst(s, t->res); /* fallthrought */
	case T_CON:
		for (int i = 0; i < s.length; ++i)
			t->args[i] = app_subst(s, t->args[i]);
		return t;
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
		t.args[i] = va_arg(ap, Type *);
	va_end(ap);
	t.name = s;
	return t;
}

void
types_unify(Type *t1, Type *t2)
{
	if (t1->type == T_CON && t2->type == T_CON) {
		if (!strcmp(t1->name, t2->name) && t1->arity == t2->arity) {
			for (int i = 0; i < t1->arity; ++i)
				types_unify(t1->args[i], t2->args[i]);
		} else {
			unify_error();
		}
	} else if (t1->type == T_FUN && t2->type == T_FUN) {
		if (t1->arity == t2->arity) {
			for (int i = 0; i < t1->arity; ++i)
				types_unify(t1->args[i], t2->args[i]);
		} else {
			unify_error();
		}
		types_unify(t1->res, t2->res);
	} else if (t1->type == T_VAR) {
		bind(t1->tvar, *t2);
	} else if (t2->type == T_VAR) {
		bind(t2->tvar, *t1);
	} else {
		unify_error();
	}
}

Type *
types_fresh_tvar(void)
{
	Type t;

	t.tvar = type_variables.length;
	t.type = T_VAR;
	array_write(&type_variables, &t);
	return (Type *)type_variables.p + type_variables.length - 1;
}

Scheme
types_gen(Type *t)
{
	Scheme s;

	array_init(&s.bindings, sizeof(int));
	ftv(&s.bindings, *t);
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
		e.polybind.args[i] = *subst.s[i].t;
	}
	e.t = app_subst(subst, s.t);
	e.name = var;
	return e;
}

Scheme
types_get_ctx(char *var)
{
	for (int i = 0; i < context.length; ++i)
		if (!strcmp(var, ((Celem *)context.p)[i].var))
			return ((Celem *)context.p)[i].s;
	exit(1);
}

void
types_add_var(char *var, Scheme s)
{
	Celem c;

	c = (Celem){.var = var, .s = s};
	array_write(&context, &c);
}

int
types_get_ctx_len(void)
{
	return context.length;
}

void
types_set_ctx_len(int len)
{
	context.length = len;
}

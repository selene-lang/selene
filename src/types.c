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

typedef struct {
	char *var;
	Scheme s;
} Celem;

static void unify_error(void);

static int isftv(Type t, int v);
static void ftv(Array *a, Type t);
static void bind(int v, Type t);

static void app_subst(Substitution s, Type *t);

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
Type types_char = {.type = T_CON, .args = NULL, .arity = 0, .name = "char"};
Type types_void = {.type = T_CON, .args = NULL, .arity = 0, .name = "void"};
Type types_float = {.type = T_CON, .args = NULL, .arity = 0, .name = "float"};

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
			if (isftv(t.args[i], v)) return 1;
		return 0;
	case T_VAR:
		return t.tvar == v;
	}
}

static void
ftv(Array *a, Type t)
{
	switch (t.type) {
	case T_VAR:
		for (int i = 0; i < a->length; ++i)
			if (((int *)a->p)[i] == t.tvar) return;
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
bind(int v, Type t)
{
	if (isftv(t, v) && t.type != T_VAR)
		unify_error();
	((Type *)type_variables.p)[v] = t;
}

static void
app_subst(Substitution s, Type *t)
{
	switch (t->type) {
	case T_VAR:
		for (int i = 0; i < s.length; ++i)
			if (s.s[i].n == t->tvar) *t = s.s[i].t;
		break;
	case T_FUN:
		app_subst(s, t->res); /* fallthrought */
	case T_CON:
		for (int i = 0; i < t->arity; ++i)
			app_subst(s, t->args + i);
		break;
	}
}

static Type
type_dup(Type t)
{
	switch (t.type) {
	case T_VAR:
		return t;
	case T_CON: {
		Type t2;
		t2.type = T_CON;
		t2.name = strdup(t.name);
		t2.arity = t.arity;
		t2.args = emalloc(t.arity * sizeof(Type));
		for (int i = 0; i < t.arity; ++i)
			t2.args[i] = type_dup(t.args[i]);
		return t2;
	}
	case T_FUN: {
		Type t2;
		t2.type = T_FUN;
		t2.res = emalloc(sizeof(Type));
		*t2.res = type_dup(*t.res);
		t2.arity = t.arity;
		t2.args = emalloc(t.arity * sizeof(Type));
		for (int i = 0; i < t.arity; ++i)
			t2.args[i] = type_dup(t.args[i]);
		return t2;
	}
	}
}

void
types_unify(Type t1, Type t2)
{

	if (t1.type == T_CON && t2.type == T_CON) {
		if (strcmp(t1.name, t2.name) && t1.arity == t2.arity)
			unify_error();
		for (int i = 0; i < t1.arity; ++i)
			types_unify(t1.args[i], t2.args[i]);
	} else if (t1.type == T_FUN && t2.type == T_FUN) {
		printf("%d\n%d\n", t1.arity, t2.arity);
		if (t1.arity != t2.arity)
			unify_error();
		for (int i = 0; i < t1.arity; ++i)
			types_unify(t1.args[i], t2.args[i]);
		types_unify(*t1.res, *t2.res);
	} else if (t1.type == T_VAR) {
		bind(t1.tvar, t2);
	} else if (t2.type == T_VAR) {
		bind(t2.tvar, t1);
	} else {
		unify_error();
	}
}

Type
types_fresh_tvar(void)
{
	Type t;

	t.tvar = type_variables.length;
	t.type = T_VAR;
	array_write(&type_variables, &t);
	return ((Type *)type_variables.p)[type_variables.length - 1];
}

Type *
types_get_tvar(Type t)
{
	return ((Type *)type_variables.p) + t.tvar;
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
	e.t = type_dup(s.t);
	app_subst(subst, &e.t);
	types_eval(&e.t);
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

	c = (Celem){.var = strdup(var), .s = s};
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
	case T_VAR: {
		int prev_tvar;
		do {
			prev_tvar = t->tvar;
			*t = ((Type *)type_variables.p)[t->tvar];
		} while (t->type == T_VAR && prev_tvar != t->tvar);
		break;
	}
	}
}

void
types_eval_expr(Expr *e)
{
	types_eval(&e->t);
	switch (e->type) {
	case E_OP:
		types_eval_expr(e->left);
		if (e->op != O_NEG)
			types_eval_expr(e->right);
		break;
	case E_FUNCALL:
		types_eval_expr(e->left);
		for (int i = 0; i < e->args.length; ++i)
			types_eval_expr((Expr *)e->args.p + i);
		break;
	case E_VAR:
		for (int i = 0; i < e->polybind.len; ++i)
			types_eval(e->polybind.args + i);
		break;
	default:
		break;
	}
}

void
types_eval_statement(Statement *s)
{
	switch (s->type) {
	case S_RETURN:
	case S_EXPR:
		types_eval_expr(&s->e);
		break;
	case S_IF:
		types_eval_block(s->elseb); /* fallthrought */
	case S_WHILE:
		types_eval_expr(&s->e);
		types_eval_block(s->body);
		break;
	case S_VAR_DECL:
		types_eval(&s->t);
		break;
	}
}

void
types_eval_block(Array a)
{
	for (int i = 0; i < a.length; ++i)
		types_eval_statement((Statement *)a.p + i);
}

Type *
types_dup(Type t)
{
	Type *dup;

	dup = emalloc(sizeof(Type));
	*dup = t;
	return dup;
}

int
types_equ(Type t1, Type t2)
{
	if (t1.type != t2.type)
		return 0;
	switch (t1.type) {
	case T_FUN:
		if (!types_equ(*t1.res, *t2.res))
			return 0; /* fallthrought */
	case T_CON:
		if (t1.arity != t2.arity)
			return 0;
		for (int i = 0; i < t1.arity; ++i)
			if (!types_equ(t1.args[i], t2.args[i]))
				return 0;
		break;
	case T_VAR:
		return -1; /* types_equ should not be used on types with type
		            * variables. */ 
	}
	return 1;
}

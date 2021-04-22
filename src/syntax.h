#ifndef __SYNTAX_H_
#define __SYNTAX_H_

#include "array.h"
#include "common.h"

typedef struct type Type;
typedef struct scheme Scheme;
typedef struct expr Expr;
typedef struct statement Statement;

struct type {
	enum {
		T_CON, T_VAR, T_FUN
	} type;
	Type *res;
	Array args;
	char *name;
	int tvar;
};

struct scheme {
	Array bindings;
	Type t;
};

struct expr {
	enum {
		E_VAR, E_LIT, E_BINOP, E_FUNCALL, E_TAPP
	} type;
	Array args;
	Expr *fun;
	char *name;
	enum {
		O_PLUS, O_MINUS, O_MULT, O_DIV
	} op;
};

struct statement {
	enum {
		S_EXPR, S_IF, S_WHILE
	} type;
	Array body;
	Array elsb;
	Expr e;
};

#endif

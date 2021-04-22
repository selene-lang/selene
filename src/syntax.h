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
		E_VAR, E_NUM, E_OP, E_FUNCALL, E_TAPP
	} type;
	Array args;
	Expr *left;
	Expr *right;
	char *name;
	int number;
	enum {
		O_PLUS, O_MINUS, O_MULT, O_DIV, O_NEG
	} op;
	Type t;
};

struct statement {
	enum {
		S_EXPR, S_IF, S_WHILE, S_VAR_DECL, S_AFFE
	} type;
	Array body;
	Array elsb;
	char *name;
	Expr e;
};

#endif

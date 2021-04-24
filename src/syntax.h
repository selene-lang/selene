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
	Type *args;
	Type *res;

	int tvar;

	char *name;
	int arity;
};

struct scheme {
	Array bindings;
	Type t;
};

struct expr {
	enum {
		E_VAR, E_NUM, E_OP, E_FUNCALL
	} type;
	Expr *left;
	Expr *right;
	Array args;
	char *name;
	struct {
		Type *args;
		int len;
	} polybind;
	int number;
	enum {
		O_PLUS, O_MINUS, O_MULT, O_DIV, O_NEG, O_EQU, O_GRT, O_GRTEQ,
		O_LWR, O_LWREQ
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

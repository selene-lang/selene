#ifndef __SYNTAX_H_
#define __SYNTAX_H_

#include "array.h"
#include "common.h"

typedef struct type Type;
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

typedef struct {
	Array bindings;
	Type t;
} Scheme;

struct expr {
	enum {
		E_VAR, E_INT, E_OP, E_FUNCALL, E_TUPLE, E_FLOAT
	} type;
	Expr *left;
	Expr *right;
	Array args;
	char *name;
	struct {
		Type *args;
		int len;
	} polybind;
	union {
		double fnumber;
		long inumber;
	};
	enum {
		O_PLUS, O_MINUS, O_MULT, O_DIV, O_NEG, O_EQU, O_GRT, O_GRTEQ,
		O_LWR, O_LWREQ, O_ASSGN
	} op;
	Type t;
};

struct statement {
	enum {
		S_EXPR, S_IF, S_WHILE, S_VAR_DECL, S_RETURN
	} type;
	Array body;
	Array elseb;
	Expr e;
	Type t;
	char *name;
};

typedef struct {
	Array body;
	Scheme s;
	char *name;
	Array args;
} Function;

typedef struct {
	char *name;
	Scheme s;
} Extern;

typedef struct {
	enum {
		TL_FUN, TL_EXT
	} type;
	union {
		Function fun;
		Extern ext;
	};
} TopLevel;

#endif

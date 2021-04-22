#include "lexer.h"
#include "memory.h"
#include "parser.h"
#include "syntax.h"
#include "types.h"

typedef struct {
	Token previous;
	Token current;
} Parser;

enum precedence {
	P_NONE, P_ASSIGN, P_EQU, P_COMP, P_TERM, P_FACT, P_UNARY, P_CALL, P_PRIM
};

static void next_token(void);
static void expect(enum token t);
static Expr *exprdup(Expr e);

static Expr parse_precedence(int precedence);
static Expr number(void);
static Expr unary(void);
static Expr expr(void);

static Parser parser;

static void
next_token(void)
{
	parser.previous = parser.current;
	parser.current = lexer_get_token();
}

static void
expect(enum token t)
{
	if (parser.current.t == t) {
		next_token();
		return;
	}
	exit(1);
}

static Expr *
exprdup(Expr e)
{
	Expr *p;

	p = emalloc(sizeof(Expr));
	*p = e;
	return p;
}

static Expr
number(void)
{
	Expr e;

	e.number = 0;
	for (int i = 0; i < parser.previous.length; ++i)
		if (parser.previous.src[i] != '_')
			e.number = e.number * 10 + parser.previous.src[i] - '0';
	e.type = E_NUM;
	e.t = mktcon("int");
	return e;
}

static Expr
unary(void)
{
	enum token t;
	Expr e, arg;

	t = parser.previous.t;
	arg = parse_precedence(P_UNARY);
	e.type = E_OP;
	e.t = arg.t;
	e.op = O_NEG;
	e.left = exprdup(arg);
	return e;
}

static Expr
expr(void)
{
	return parse_precedence(P_ASSIGN);
}

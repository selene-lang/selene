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
	P_ASSIGN = 10, P_EQU = 20, P_COMP = 30, P_TERM = 40, P_FACT = 50,
	P_UNARY = 60, P_CALL = 70
};

enum associativity {
	ASSOC_LEFT, ASSOC_RIGHT, ASSOC_NONE
};

typedef struct {
	enum associativity assoc;
	enum precedence precedence;
	Expr (*f)(Expr);
} ParseRule;

static void next_token(void);
static void expect(enum token t);
static int match(enum token t);
static enum token peek(void);
static Expr *exprdup(Expr e);

static int isunary(enum token t);
static int prec(enum token t);
static int assoc(enum token t);
static Expr rulef(enum token t, Expr e);

static Expr number(void);
static Expr unary(void);
static Expr simple_expr(void);
static Expr binop(Expr lhs);
static Expr fun_call(Expr fun);
static Expr expr(void);

static Expr parse_precedence(int precedence);

static Parser parser;
static const ParseRule rules[TOKEN_EOF] = {
	[TOKEN_ASSIGN]    = {ASSOC_RIGHT, P_ASSIGN, binop},
	[TOKEN_EQUAL]     = {ASSOC_LEFT,  P_EQU,    binop},
	[TOKEN_GREATER]   = {ASSOC_NONE,  P_COMP,   binop},
	[TOKEN_GREATEREQ] = {ASSOC_NONE,  P_COMP,   binop},
	[TOKEN_LOWER]     = {ASSOC_NONE,  P_COMP,   binop},
	[TOKEN_LOWEREQ]   = {ASSOC_NONE,  P_COMP,   binop},
	[TOKEN_PLUS]      = {ASSOC_LEFT,  P_TERM,   binop},
	[TOKEN_MINUS]     = {ASSOC_LEFT,  P_TERM,   binop},
	[TOKEN_MULT]      = {ASSOC_LEFT,  P_FACT,   binop},
	[TOKEN_DIV]       = {ASSOC_LEFT,  P_FACT,   binop},
	[TOKEN_OPAR]      = {ASSOC_LEFT,  P_CALL,   fun_call},
};

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

static int
match(enum token t)
{
	if (parser.current.t == t) {
		next_token();
		return 1;
	}
	return 0;
}

static enum token
peek(void)
{
	return parser.current.t;
}

static Expr *
exprdup(Expr e)
{
	Expr *p;

	p = emalloc(sizeof(Expr));
	*p = e;
	return p;
}

static int
isunary(enum token t)
{
	switch (t) {
	case TOKEN_MINUS:
		return 1;
	default:
		return 0;
	}
}

static int
prec(enum token t)
{
	if (rules[t].f != NULL)
		return rules[t].precedence;
	return -1;
}

static int
assoc(enum token t)
{
	if (rules[t].f != NULL)
		return rules[t].assoc;
	return -1;
}

static Expr
rulef(enum token t, Expr e)
{
	return rules[t].f(e);
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
	e.t = mktcon("int", 0);
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
simple_expr(void)
{
	if (match(TOKEN_INT)) {
		return number();
	} else if (isunary(peek())) {
		next_token();
		return unary();
	} else if (match(TOKEN_OPAR)) {
		Expr e = expr();
		expect(TOKEN_CPAR);
		return e;
	} else {
		exit(1);
	}
}

static Expr
binop(Expr lhs)
{
	Expr e, rhs;
	enum token t;

	t = parser.previous.t;
	switch (prec(t)) {
	case ASSOC_LEFT:
		rhs = parse_precedence(prec(t) + 1);
		break;
	case ASSOC_NONE:
		rhs = parse_precedence(prec(t) + 10);
		break;
	case ASSOC_RIGHT:
		rhs = parse_precedence(prec(t));
		break;
	}
}

static Expr
expr(void)
{
	return parse_precedence(P_ASSIGN);
}


static Expr
parse_precedence(int precedence)
{
	Expr e;

	next_token();
	e = simple_expr();
	while (prec(peek()) >= precedence) {
		enum token t = peek();
		next_token();
		e = rulef(t, e);
	}
	return e;
}

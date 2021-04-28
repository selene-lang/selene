#include <string.h>

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
static int isopbool(enum token t);
static int prec(enum token t);
static int assoc(enum token t);
static Expr rulef(enum token t, Expr e);

static Expr number(void);
static Expr var(void);
static Expr unary(void);
static Expr simple_expr(void);
static Expr binop(Expr lhs);
static Expr fun_call(Expr fun);
static Expr expr(void);
static Expr parse_precedence(int precedence);

static Statement ifstatement(void);
static Statement varstatement(void);

static Parser parser;
static const ParseRule rules[TOKEN_EOF + 1] = {
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

static const int tok2op[TOKEN_EOF + 1] = {
	[TOKEN_EQUAL]     = O_EQU,
	[TOKEN_GREATER]   = O_GRT,
	[TOKEN_GREATEREQ] = O_GRTEQ,
	[TOKEN_LOWER]     = O_LWR,
	[TOKEN_LOWEREQ]   = O_LWREQ,
	[TOKEN_PLUS]      = O_PLUS,
	[TOKEN_MINUS]     = O_MINUS,
	[TOKEN_MULT]      = O_MULT,
	[TOKEN_DIV]       = O_DIV
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
isopbool(enum token t)
{
	switch (t) {
	case TOKEN_LOWER:
	case TOKEN_LOWEREQ:
	case TOKEN_GREATER:
	case TOKEN_GREATEREQ:
	case TOKEN_EQUAL:
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
	e.t = types_mktcon("int", 0);
	return e;
}

static Expr
var(void)
{
	char v[parser.previous.length + 1];

	memcpy(v, parser.previous.src, parser.previous.length);
	v[parser.previous.length] = '\0';
	return types_inst(v, types_get_ctx(v));
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
	} else if (match(TOKEN_IDENT)) {
		return var();
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
	types_unify(lhs.t, rhs.t);
	e.t = isopbool(t) ? types_mktcon("bool", 0) : lhs.t;
	e.type = E_OP;
	e.left = exprdup(lhs);
	e.right = exprdup(rhs);
	e.op = tok2op[t];
	types_eval_expr(&e);
	return e;
}

static Expr
fun_call(Expr fun)
{
	Expr e, tmp;
	Type t;

	e.left = exprdup(fun);
	array_init(&e.args, sizeof(Expr));
	do {
		tmp = expr();
		array_write(&e.args, &tmp);
	} while (match(TOKEN_COMMA));
	expect(TOKEN_CPAR);

	t.type = T_FUN;
	t.res = types_dup(types_fresh_tvar());
	t.args = emalloc(e.args.length * sizeof(Type));
	for (int i = 0; i < e.args.length; ++i) {
		types_eval_expr((Expr *)e.args.p + i);
		t.args[i] = ((Expr *)e.args.p)[i].t;
	}
	types_unify(t, fun.t);
	e.t = *t.res;
	types_eval_expr(&e);
	return e;
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

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
static char *ident(void);
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

Array block(void);
static Statement ifstatement(void);
static Statement whilestatement(void);
static Statement varstatement(void);
static Statement returnstatement(void);
static Statement statement(void);

static Function parser_function(void);

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
	[TOKEN_ASSIGN]    = O_ASSGN,
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

void
parser_init(char *program)
{
	lexer_init(program);
	next_token();
}

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

static char *
ident(void)
{
	char *v;

	expect(TOKEN_IDENT);
	v = emalloc(parser.previous.length + 1);
	memcpy(v, parser.previous.src, parser.previous.length);
	v[parser.previous.length] = '\0';
	return v;
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

	e = (Expr){.number = 0};
	for (int i = 0; i < parser.previous.length; ++i)
		if (parser.previous.src[i] != '_')
			e.number = e.number * 10 + parser.previous.src[i] - '0';
	e.type = E_NUM;
	e.t = types_int;
	return e;
}

static Expr
var(void)
{
	char *v;

	v = emalloc(parser.previous.length + 1);
	memcpy(v, parser.previous.src, parser.previous.length);
	v[parser.previous.length] = '\0';
	return types_inst(v, types_get_ctx(v));
}

static Expr
unary(void)
{
	Expr e, arg;

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
	switch (assoc(t)) {
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
	types_eval_expr(&lhs);
	types_eval_expr(&rhs);
	e.t = isopbool(t) ? types_bool : lhs.t;
	e.type = E_OP;
	e.left = exprdup(lhs);
	e.right = exprdup(rhs);
	e.op = tok2op[t];
	return e;
}

static Expr
fun_call(Expr fun)
{
	Expr e, tmp;
	Type t;

	e.left = exprdup(fun);
	array_init(&e.args, sizeof(Expr));
	while (!match(TOKEN_CPAR)) {
		tmp = expr();
		array_write(&e.args, &tmp);
		expect(TOKEN_COMMA);
	}

	t.type = T_FUN;
	t.res = types_get_tvar(types_fresh_tvar());
	t.args = emalloc(e.args.length * sizeof(Type));
	for (int i = 0; i < e.args.length; ++i)
		t.args[i] = ((Expr *)e.args.p)[i].t;
	types_unify(t, fun.t);
	e.t = *t.res;
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

	e = simple_expr();
	while (prec(peek()) >= precedence) {
		enum token t = peek();
		next_token();
		e = rulef(t, e);
	}
	return e;
}

Array
block(void)
{
	Array a;
	Statement s;
	int ctx_len;

	expect(TOKEN_OBRA);

	ctx_len = types_get_ctx_len();
	array_init(&a, sizeof(Statement));

	while (!match(TOKEN_CBRA)) {
		s = statement();
		array_write(&a, &s);
	}
	types_eval_block(a);
	types_set_ctx_len(ctx_len);
	return a;
}

static Statement
ifstatement(void)
{
	Statement s;

	s.e = expr();
	s.type = S_IF;
	types_unify(s.e.t, types_int);
	s.body = block();
	if (match(TOKEN_ELSE)) {
		s.elseb = block();
	} else {
		array_init(&s.elseb, sizeof(Statement));
	}
	return s;
}

static Statement
whilestatement(void)
{
	Statement s;

	s.e = expr();
	s.type = S_WHILE;
	types_unify(s.e.t, types_int);
	s.body = block();
	return s;
}

static Statement
varstatement(void)
{
	Statement s;
	Scheme sch;

	s.name = ident();
	s.type = S_VAR_DECL;

	sch.t = types_fresh_tvar();
	array_init(&sch.bindings, sizeof(int));
	types_add_var(s.name, sch);

	s.t = sch.t;

	expect(TOKEN_SEMI);
	return s;
}

static Statement
returnstatement(void)
{
	Statement s;

	s.e = expr();
	s.type = S_RETURN;
	expect(TOKEN_SEMI);
	return s;
}

static Statement
statement(void)
{
	if (match(TOKEN_IF)) {
		return ifstatement();
	} else if (match(TOKEN_WHILE)) {
		return whilestatement();
	} else if (match(TOKEN_VAR)) {
		return varstatement();
	} else if (match(TOKEN_RETURN)) {
		return returnstatement();
	} else {
		Statement s = {.type = S_EXPR, .e = expr()};
		expect(TOKEN_SEMI);
		return s;
	}
}

static Function
parser_function(void)
{
	int clen;
	Scheme s;
	Function f;
	Array a, targs;

	clen = types_get_ctx_len();
	array_init(&s.bindings, sizeof(int));
	array_init(&targs, sizeof(Type));

	f.name = ident();

	expect(TOKEN_OPAR);
	while (!match(TOKEN_CPAR)) {
		s.t = types_fresh_tvar();
		array_write(&targs, &s.t);
		types_add_var(ident(), s);
	}

	a = block();
	f.body = a.p;
	f.bodylen = a.length;

	types_set_ctx_len(clen);

	return f;
}

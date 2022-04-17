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

static int hexdigit(char c);

static Expr integer(void);
static Expr bool(void);
static Expr character(void);
static Expr var(void);
static Expr unary(void);
static Expr simple_expr(void);
static Expr binop(Expr lhs);
static Expr fun_call(Expr fun);
static Expr expr(void);
static Expr parse_precedence(int precedence);

static Type simple_type(void);
static Type function_type(void);
static Type type(void);

static Array block(void);
static Statement ifstatement(void);
static Statement whilestatement(void);
static Statement varstatement(void);
static Statement returnstatement(void);
static Statement statement(void);

static void block_ret_type(Array a, Type t);
static Function function(void);
static Extern pextern(void);
static TopLevel top_level(void);

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

static const long escape_character[256] = {
	['\\'] = '\\', ['n'] = '\n', ['t'] = '\t', ['\''] = '\''
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

static int
hexdigit(char c)
{
	return c <= '9' ? c - '0' : c - '0' + 10;
}

static Expr
integer(void)
{
	Expr e;

	e = (Expr){.inumber = 0, .type = E_INT, .t = types_int};
	for (int i = 0; i < parser.previous.length; ++i) {
		if (parser.previous.src[i] != '_') {
			e.inumber *= 10;
			e.inumber += parser.previous.src[i] - '0';
		}
	}
	return e;
}

static Expr
bool(void)
{
	Expr e;

	e = (Expr){.type = E_INT, .t = types_bool,
	           .inumber = (long)(parser.previous.t - TOKEN_FALSE)};

	return e;
}

static Expr
character(void)
{
	Expr e;
	char *s;

	e = (Expr){.type = E_INT, .t = types_char};
	s = parser.previous.src;
	if (s[1] == '\\') {
		if (s[2] == 'x')
			e.inumber = 16 * hexdigit(s[3]) + (int)s[4];
		else
			e.inumber = escape_character[(int)s[2]];
	} else {
		e.inumber = (long)(s[1]);
	}
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
	e.right = NULL;
	return e;
}

static Expr
simple_expr(void)
{
	if (match(TOKEN_INT)) {
		return integer();
	} else if (match(TOKEN_TRUE) || match(TOKEN_FALSE)) {
		return bool();
	} else if (match(TOKEN_CHAR)) {
		return character();
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
	e.type = E_FUNCALL;
	array_init(&e.args, sizeof(Expr));
	if (!match(TOKEN_CPAR)) {
		do {
			tmp = expr();
			array_write(&e.args, &tmp);
		} while (match(TOKEN_COMMA));
		expect(TOKEN_CPAR);
	}

	t.type = T_FUN;
	t.res = types_get_tvar(types_fresh_tvar());
	t.arity = e.args.length;
	t.args = emalloc(e.args.length * sizeof(Type));
	for (int i = 0; i < e.args.length; ++i)
		t.args[i] = ((Expr *)e.args.p)[i].t;


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

	e = simple_expr();
	while (prec(peek()) >= precedence) {
		enum token t = peek();
		next_token();
		e = rulef(t, e);
	}
	return e;
}

static Type
simple_type(void)
{
	Type t;

	t.name = ident();
	t.type = T_CON;
	t.arity = 0;
	return t;
}

static Type
function_type(void)
{
	Type t;
	Array args;

	array_init(&args, sizeof(Type));

	if (!match(TOKEN_CPAR)) {
		do {
			t = type();
			array_write(&args, &t);
		} while (match(TOKEN_COMMA));
		expect(TOKEN_CPAR);
	}
	expect(TOKEN_ARR);

	t.type = T_FUN;
	t.args = (Type *)args.p;
	t.arity = args.length;
	t.res = types_dup(type());
	return t;
}

static Type
type(void)
{
	Type t;

	if (match(TOKEN_OPAR))
		return function_type();
	t = simple_type();
	if (match(TOKEN_ARR)) {
		Type res = type();
		Type f = {.type = T_FUN, .arity = 1,
		          .args = types_dup(t), types_dup(res)};
		return f;
	}
	return t;
}

static Array
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
	types_unify(s.e.t, types_bool);
	s.body = block();
	if (match(TOKEN_ELSE))
		s.elseb = block();
	else
		array_init(&s.elseb, sizeof(Statement));
	return s;
}

static Statement
whilestatement(void)
{
	Statement s;

	s.e = expr();
	s.type = S_WHILE;
	s.body = block();
	types_unify(s.e.t, types_bool);
	return s;
}

static Statement
varstatement(void)
{
	Statement s;
	Scheme sch;

	s.name = ident();
	s.type = S_VAR_DECL;

	expect(TOKEN_ASSIGN);
	s.e = expr();

	sch.t = s.e.t;
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

	s = (Statement){.e = expr(), .type = S_RETURN};
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
	} else if (match(TOKEN_LET)) {
		return varstatement();
	} else if (match(TOKEN_RETURN)) {
		return returnstatement();
	} else {
		Statement s = {.type = S_EXPR, .e = expr()};
		expect(TOKEN_SEMI);
		return s;
	}
}

static void
block_ret_type(Array a, Type t)
{
	Statement *p;

	p = a.p;
	for (int i = 0; i < a.length; ++i) {
		switch (p[i].type) {
		case S_EXPR:
		case S_VAR_DECL:
			break;
		case S_IF:
			block_ret_type(p[i].elseb, t);
			types_eval(&t); /* fallthrought */
		case S_WHILE:
			block_ret_type(p[i].body, t);
			break;
		case S_RETURN:
			types_unify(t, p[i].e.t);
			break;
		}
		types_eval(&t);
	}
}

static Function
function(void)
{
	int clen;
	Scheme s;
	Function f;
	Array targs;
	Type buf, tfun;

	clen = types_get_ctx_len();
	array_init(&targs, sizeof(Type));
	array_init(&f.args, sizeof(char *));

	f.name = ident();

	expect(TOKEN_OPAR);
	if (!match(TOKEN_CPAR)) {
		do {
			char *x = ident();
			s.t = types_fresh_tvar();
			array_write(&targs, &s.t);
			types_add_var(x, s);
			array_write(&f.args, &x);
		} while (match(TOKEN_COMMA));
		expect(TOKEN_CPAR);
	}

	tfun.type = T_FUN;
	tfun.arity = targs.length;
	tfun.args = erealloc(targs.p, targs.capacity * sizeof(Type),
	                     targs.length * sizeof(Type));
	tfun.res = types_dup(types_fresh_tvar());

	array_init(&s.bindings, sizeof(int));
	s.t = tfun;
	types_add_var(f.name, s);

	f.body = block();

	buf = *tfun.res;
	block_ret_type(f.body, buf);
	types_eval(tfun.res);

	if (tfun.res->type == T_VAR && tfun.res->tvar == buf.tvar)
		tfun.res = &types_void;
	else
		tfun.res = types_get_tvar(buf);
	types_eval(&tfun);
	f.s = types_gen(tfun);
	types_set_ctx_len(clen);

	return f;
}

static Extern
pextern(void)
{
	Extern ext;

	ext.name = ident();
	expect(TOKEN_COL);
	ext.s = types_gen(type());
	expect(TOKEN_SEMI);
	return ext;
}

static TopLevel
top_level(void)
{
	TopLevel tl;

	if (match(TOKEN_FUN)) {
		tl.fun = function();
		tl.type = TL_FUN;
	} else if (match(TOKEN_EXTERN)) {
		tl.ext = pextern();
		tl.type = TL_EXT;
	} else {
		exit(1);
	}
	return tl;
}

Array
parser_program(void)
{
	Array a;
	TopLevel tl;

	array_init(&a, sizeof(TopLevel));
	while (!match(TOKEN_EOF)) {
		tl = top_level();
		if (tl.type == TL_FUN)
			types_add_var(tl.fun.name, tl.fun.s);
		else
			types_add_var(tl.ext.name, tl.ext.s);
		array_write(&a, &tl);
	}
	return a;
}

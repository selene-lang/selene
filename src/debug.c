#include <stdio.h>

#include "debug.h"
#include "lexer.h"
#include "syntax.h"

static char *op2string[] = {
	[O_DIV] = "division",
	[O_NEG] = "negation",
	[O_EQU] = "equal",
	[O_GRT] = "greater",
	[O_LWR] = "lower",
	[O_PLUS] = "plus",
	[O_MULT] = "multiplication",
	[O_MINUS] = "minus",
	[O_GRTEQ] = "greater/equal",
	[O_LWREQ] = "lower/equal",
	[O_ASSGN] = "assignment"
};

static struct print_instruction {
	char *op;
	int narg;
	u8 extra_arg;
} instruction2string[] = {
	[OP_RET] = {"ret", 1, 0},
	[OP_MOV] = {"mov", 2, 0},
	[OP_ADDI] = {"addi", 3, 0},
	[OP_SUBI] = {"subi", 3, 0},
	[OP_MULI] = {"multi", 3, 0},
	[OP_DIVI] = {"divi", 3, 0},
	[OP_EQUI] = {"equi", 3, 0},
	[OP_CJMP] = {"cjmp", 1, 1},
	[OP_UJMP] = {"ujmp", 0, 1},
	[OP_NJMP] = {"njmp", 1, 1},
	[OP_CALL] = {"call", 2, 2},
	[OP_CCALL] = {"ccall", 2, 2},
};

void
print_token(Token t)
{
	printf("%4d ", t.linum);
	switch (t.t) {
	case TOKEN_IDENT:
		printf("ident");
		break;
	case TOKEN_ARR:
	case TOKEN_SEMI:
	case TOKEN_COL:
	case TOKEN_ASSIGN:
	case TOKEN_EQUAL:
	case TOKEN_COMMA:
	case TOKEN_OPAR:
	case TOKEN_CPAR:
	case TOKEN_OBRACE:
	case TOKEN_CBRACE:
	case TOKEN_OBRACK:
	case TOKEN_CBRACK:
	case TOKEN_PLUS:
	case TOKEN_MINUS:
	case TOKEN_MULT:
	case TOKEN_DIV:
	case TOKEN_GREATER:
	case TOKEN_GREATEREQ:
	case TOKEN_LOWER:
	case TOKEN_LOWEREQ:
		printf("symbol");
		break;
	case TOKEN_INT:
		printf("int");
		break;
	case TOKEN_CHAR:
		printf("char");
		break;
	case TOKEN_FLOAT:
		printf("float");
		break;
	case TOKEN_EXTERN:
	case TOKEN_WHILE:
	case TOKEN_FUN:
	case TOKEN_RETURN:
	case TOKEN_LET:
	case TOKEN_IF:
	case TOKEN_ELSE:
	case TOKEN_TRUE:
	case TOKEN_FALSE:
		printf("keyword");
		break;
	case TOKEN_EOF:
		printf("EOF\n");
		return;
	}
	printf("\t%.*s\n", t.length, t.src);
}

void
print_type(Type t)
{
	printf("{");
	switch (t.type) {
	case T_VAR:
		printf("\"type variable\":\"%lc\"", L'α' + t.tvar);
		break;
	case T_CON:
		printf("\"type constructor\":{\"name\":\"%s\",\"arguments\":[",
		       t.name);
		for (int i = 0; i < t.arity; ++i) {
			if (i != 0) printf(",");
			print_type(t.args[i]);
		}
		printf("]}");
		break;
	case T_FUN:
		printf("\"function type\":{\"arguments\":[");
		for (int i = 0; i < t.arity; ++i) {
			if (i != 0) printf(",");
			print_type(t.args[i]);
		}
		printf("],\"return\":");
		print_type(*t.res);
		printf("}");
		break;
	}
	printf("}");
}

void
print_scheme(Scheme s)
{
	printf("{\"bindings\":[");
	for (int i = 0; i < s.bindings.length; ++i) {
		if (i != 0) printf(",");
		printf("\"%lc\"", L'α' + ((int *)s.bindings.p)[i]);
	}
	printf("],\"type\":");
	print_type(s.t);
	printf("}");
}

void
print_expr(Expr e)
{
	printf("{");
	switch (e.type) {
	case E_VAR:
		printf("\"var\":\"%s\",\"bind\":[", e.name);
		for (int i = 0; i < e.polybind.len; ++i) {
			if (i != 0) printf(",");
			print_type(e.polybind.args[i]);
		}
		printf("]");
		break;
	case E_INT:
		printf("\"integer\":%ld", e.inumber);
		break;
	case E_FUNCALL:
		printf("\"call\":{\"function\":");
		print_expr(*e.left);
		printf(",\"arguments\":[");
		for (int i = 0; i < e.args.length; ++i) {
			if (i != 0) printf(",");
			print_expr(((Expr *)e.args.p)[i]);
		}
		printf("]}");
		break;
	case E_OP:
		printf("\"operator\":{\"name\":\"%s\",\"lhs\":",
		       op2string[e.op]);
		print_expr(*e.left);
		printf(",\"rhs\":");
		print_expr(*e.right);
		printf("}");
		break;
	case E_TUPLE:
		printf("\"tuple\":{\"left\":");
		print_expr(*e.left);
		printf(",\"right\":");
		print_expr(*e.right);
		printf("}");
		break;
	}
	printf(",\"type\":");
	print_type(e.t);
	printf("}");
}

void
print_block(Array a)
{
	printf("[");
	for (int i = 0; i < a.length; ++i) {
		if (i != 0) printf(",");
		print_statement(((Statement *)a.p)[i]);
	}
	printf("]");
}

void
print_statement(Statement s)
{
	printf("{");
	switch (s.type) {
	case S_EXPR:
		printf("\"expr\":");
		print_expr(s.e);
		break;
	case S_RETURN:
		printf("\"return\":");
		print_expr(s.e);
		break;
	case S_VAR_DECL:
		printf("\"variable declaration\":{\"name\":\"%s\",\"type\":",
		       s.name);
		print_type(s.t);
		printf("}");
		break;
	case S_IF:
		printf("\"if\":{\"condition\":");
		print_expr(s.e);
		printf(",\"true branch\":");
		print_block(s.body);
		if (s.elseb.length != 0) {
			printf(",\"false branch\":");
			print_block(s.elseb);
		}
		printf("}");
		break;
	case S_WHILE:
		printf("\"while\":{\"condition\":");
		print_expr(s.e);
		printf(", \"body\":");
		print_block(s.body);
		printf("}");
		break;
	}
	printf("}");
}

void
print_function(Function f)
{
	printf("{\"name\":\"%s\",\"scheme\":", f.name);
	print_scheme(f.s);
	printf(",\"body\":");
	print_block(f.body);
	printf("}");
}

void
print_extern(Extern e)
{
	printf("{\"name\":\"%s\",\"scheme\":", e.name);
	print_scheme(e.s);
	printf("}");
}

void
print_top_levels(Array tl)
{
	printf("[");
	for (int i = 0; i < tl.length; ++i) {
		TopLevel t = ((TopLevel *)tl.p)[i];
		if (i != 0) printf(",");
		if (t.type == TL_FUN)
			print_function(t.fun);
		else
			print_extern(t.ext);
	}
	printf("]");
}

void
print_instruction(Instruction *i)
{
	struct print_instruction pi;

	pi = instruction2string[i->op];
	printf("{\"instruction\":{\"name\":\"%s\"", pi.op);

	if (pi.narg >= 1)
		printf(",\"a\":%d", i->a);
	if (pi.narg >= 2)
		printf(",\"b\":%d", i->b);
	if (pi.narg >= 3)
		printf(",\"c\":%d", i->c);

	if (pi.extra_arg == 1)
		printf(",\"address\":%d", *(u32*)(i+1));
	if (pi.extra_arg == 2) {
		printf(",\"arguments\":[");
		for (int j = 0; j < i->c; ++j) {
			if (j != 0) printf(",");
			printf("%d", i[j].a);
		}
		printf("]");
	}
	printf("}}");
}

void
print_chunk(Chunk c)
{
	printf("{\"chunk\":{\"constants\":[");
	for (int i = 0; i < 128; ++i) {
		if (i != 0) printf(",");
		printf("%ld", (long)c.values[i]);
	}
	printf("],\"instructions\":[");
	for (int i = 0; i < c.code.length; ++i) {
		if (i != 0) printf(",");
		OpCode op = (*(Instruction *)c.code.p).op;
		print_instruction(((Instruction *)c.code.p) + i);
		if (op == OP_UJMP || op == OP_NJMP || op == OP_CJMP)
			++i;
		if (op == OP_CALL || op == OP_CCALL)
			i += (*(Instruction *)c.code.p).c;
	}
	printf("]}}");
}

void
print_cprogram(Program p)
{
	for (int i = 0; i < p.nfun; ++i)
		print_chunk(p.fun[i]);
}

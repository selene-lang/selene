#include <stdio.h>

#include "debug.h"
#include "lexer.h"
#include "syntax.h"

static char *op2string(int op);

static char *
op2string(int op)
{
	switch (op) {
	case O_PLUS:  return "plus";
	case O_MINUS: return "minus";
	case O_MULT:  return "multiplication";
	case O_DIV:   return "division";
	case O_NEG:   return "negation";
	case O_EQU:   return "equal";
	case O_GRT:   return "greater";
	case O_GRTEQ: return "greater/equal";
	case O_LWR:   return "lower";
	case O_LWREQ: return "lower/equal";
	}
	return NULL;
}

void
print_token(Token t)
{
	printf("%4d ", t.linum);
	switch (t.t) {
	case TOKEN_IDENT:
		printf("ident");
		break;
	case TOKEN_SEMI:
	case TOKEN_ASSIGN:
	case TOKEN_EQUAL:
	case TOKEN_COMMA:
	case TOKEN_OPAR:
	case TOKEN_CPAR:
	case TOKEN_OBRA:
	case TOKEN_CBRA:
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
	case TOKEN_WHILE:
	case TOKEN_FUN:
	case TOKEN_RETURN:
	case TOKEN_VAR:
	case TOKEN_IF:
	case TOKEN_ELSE:
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
	switch (t.type) {
	case T_VAR:
		printf("{\"type variable\":\"%lc\"}", L'α' + t.tvar);
		break;
	case T_CON:
		printf("{\"type constructor\":{\"name\":\"%s\",\"arguments\":[",
		       t.name);
		for (int i = 0; i < t.arity; ++i) {
			if (i != 0) printf(",");
			print_type(*t.args[i]);
		}
		printf("]}}");
		break;
	case T_FUN:
		printf("{\"function type\":{\"arguments\":[");
		for (int i = 0; i < t.arity; ++i) {
			if (i != 0) printf(",");
			print_type(*t.args[i]);
		}
		printf("],\"return\":");
		print_type(*t.res);
		printf("}}");
		break;
	}
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
	case E_NUM:
		printf("\"integer\":%d", e.number);
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
		       op2string(e.op));
		print_expr(*e.left);
		printf(",\"rhs\":");
		print_expr(*e.right);
		printf("}");
		break;
	}
	printf(",\"type\":");
	print_type(*e.t);
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
	case S_VAR_DECL:
		printf("\"variable declaration\":{\"name\":\"%s\",\"type\":",
		       s.name);
		print_type(*s.t);
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
		break;
	}
	printf("}");
}


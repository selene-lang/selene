#include <stdio.h>

#include "lexer.h"
#include "syntax.h"

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
	printf("\t%*s\n", t.length, t.src);
}

#ifndef __LEXER_H_
#define __LEXER_H_

#include "common.h"

enum token {
	TOKEN_IDENT = 1, TOKEN_INT,

	TOKEN_IF, TOKEN_ELSE, TOKEN_WHILE, TOKEN_VAR, TOKEN_FUN, TOKEN_RETURN,

	TOKEN_SEMI, TOKEN_OPAR, TOKEN_CPAR, TOKEN_OBRA, TOKEN_CBRA,

	TOKEN_ASSIGN, TOKEN_GREATER, TOKEN_LOWER,

	TOKEN_EQUAL, TOKEN_GREATEREQ, TOKEN_LOWEREQ, TOKEN_PLUS, TOKEN_MINUS,
	TOKEN_MULT, TOKEN_DIV,

	TOKEN_EOF
};

typedef struct {
	enum token t;
	int length;
	char *src;

	int linum;
} Token;

typedef struct {
	char *start;
	char *current;

	int linum;
} Lexer;

void lexer_init(char *src);
Token lexer_get_token(void);

#endif

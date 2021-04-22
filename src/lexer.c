#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "lexer.h"

static Token mktoken(enum token t);

static char next_char(void);
static void skip_whitespaces(void);

static char peek(void);
static int match(char c);

static Lexer lexer;
static const enum token single_token_table[255] = {
	['('] = TOKEN_OPAR, [')'] = TOKEN_CPAR, ['{'] = TOKEN_OBRA,
	['}'] = TOKEN_CBRA, [';'] = TOKEN_SEMI, ['+'] = TOKEN_PLUS,
	['/'] = TOKEN_DIV,  ['*'] = TOKEN_MULT, ['-'] = TOKEN_MINUS,
};

static Token
mktoken(enum token t)
{
	Token token;

	token.t = t;
	token.length = (int)(lexer.current - lexer.start);
	token.src = lexer.start;
	token.linum = lexer.linum;
	return token;
}

static char
next_char(void)
{
	return (lexer.current++)[0];
}

static void
skip_whitespaces(void)
{
	for (;;) {
		switch (peek()) {
		case ' ':
		case '\t':
		case '\r':
			next_char();
			break;
		case '\n':
			++lexer.linum;
			next_char();
			break;
		default:
			break;
		}
	}
}

void
lexer_init(char *src)
{
	lexer.linum = 1;
	lexer.start = src;
	lexer.current = src;
}

static char
peek(void)
{
	return *lexer.current;
}

static int
match(char c)
{
	if (lexer.current[0] != c)
		return 0;

	++lexer.current;
	return 1;
}

Token
lexer_get_token(void)
{
	char c;

	lexer.start = lexer.current;

	if (lexer.current[0] == '\0')
		return mktoken(TOKEN_EOF);
	c = next_char();

	if (single_token_table[c])
		return mktoken(single_token_table[c]);

	switch (c) {
	case '=':
		return mktoken(match('=') ? TOKEN_EQUAL : TOKEN_ASSIGN);
	case '<':
		return mktoken(match('=') ? TOKEN_LOWEREQ : TOKEN_LOWER);
	case '>':
		return mktoken(match('=') ? TOKEN_GREATEREQ : TOKEN_GREATER);
	}
}

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "lexer.h"

static Token mktoken(enum token t);

static char next_char(void);
static void skip_whitespaces(void);

static char peek(void);
static int match(char c);

static enum token check_keyword(int n, char *s, enum token t);
static enum token ident_type(void);

static Token number(void);
static Token identifier(void);

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

static enum token
check_keyword(int n, char *s, enum token t)
{
	if ((int)(lexer.current - lexer.start) == strlen(s) &&
	    memcmp(lexer.start + n, s + n, strlen(s) - n)) {
		return t;
	}
	return TOKEN_IDENT;
}

static enum token
ident_type(void)
{
	switch (lexer.start[0]) {
	case 'e': return check_keyword(1, "else", TOKEN_ELSE);
	case 'f': return check_keyword(1, "fun", TOKEN_FUN);
	case 'i': return check_keyword(1, "if", TOKEN_IF);
	case 'r': return check_keyword(1, "return", TOKEN_RETURN);
	case 'v': return check_keyword(1, "var", TOKEN_VAR);
	case 'w': return check_keyword(1, "while", TOKEN_WHILE);
	}
	return TOKEN_IDENT;
}

static Token
number(void)
{
	while (isdigit(peek())) next_char();
	return mktoken(TOKEN_INT);
}

static Token
identifier(void)
{
	while (isalnum(peek()) || peek() == '_' || peek() == '\'')
		next_char();
	return mktoken(ident_type());
}

Token
lexer_get_token(void)
{
	char c;

	skip_whitespaces();

	lexer.start = lexer.current;

	if (lexer.current[0] == '\0')
		return mktoken(TOKEN_EOF);
	c = next_char();

	if (single_token_table[c])
		return mktoken(single_token_table[c]);
	if (isdigit(c))
		return number();
	if (isalpha(c) || c == '_')
		return identifier();

	switch (c) {
	case '=':
		return mktoken(match('=') ? TOKEN_EQUAL : TOKEN_ASSIGN);
	case '<':
		return mktoken(match('=') ? TOKEN_LOWEREQ : TOKEN_LOWER);
	case '>':
		return mktoken(match('=') ? TOKEN_GREATEREQ : TOKEN_GREATER);
	}
	exit(1);
}

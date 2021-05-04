#ifndef __DEBUG_H_
#define __DEBUG_H_

#include "common.h"
#include "lexer.h"
#include "syntax.h"

void print_token(Token t);
void print_type(Type t);
void print_expr(Expr e);
void print_block(Array a);
void print_statement(Statement s);

#endif

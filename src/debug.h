#ifndef __DEBUG_H_
#define __DEBUG_H_

#include "common.h"
#include "lexer.h"
#include "syntax.h"

void print_token(Token t);
void print_type(Type t);
void print_expr(Expr e);

#endif

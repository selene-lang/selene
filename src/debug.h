#ifndef __DEBUG_H_
#define __DEBUG_H_

#include "common.h"
#include "chunk.h"
#include "compile.h"
#include "lexer.h"
#include "syntax.h"

void print_token(Token t);
void print_type(Type t);
void print_scheme(Scheme s);
void print_expr(Expr e);
void print_block(Array a);
void print_statement(Statement s);
void print_function(Function f);
void print_program(Array prog);
void print_instruction(Instruction *i);
void print_chunk(Chunk c);
void print_cporgram(Program f);

#endif

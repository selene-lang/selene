#ifndef __TYPES_H_
#define __TYPES_H_

#include "common.h"
#include "syntax.h"

extern Type types_int;
extern Type types_bool;
extern Type types_void;
extern Type types_char;
extern Type types_float;

void types_unify(Type t1, Type t2);
Type types_fresh_tvar(void);
Type *types_get_tvar(Type t);

Scheme types_gen(Type t);
Expr types_inst(char *var, Scheme s);

Scheme types_get_ctx(char *var);
void types_add_var(char *var, Scheme s);
int types_get_ctx_len(void);
void types_set_ctx_len(int len);

void types_eval(Type *t);
void types_eval_expr(Expr *e);
void types_eval_statement(Statement *s);
void types_eval_block(Array a);

Type *types_dup(Type t);

int types_equ(Type t1, Type t2);

#endif

#ifndef __TYPES_H_
#define __TYPES_H_

#include "common.h"
#include "syntax.h"

extern Type types_int;
extern Type types_bool;
extern Type *types_pint;
extern Type *types_pbool;

void types_unify(Type **t1, Type **t2);
Type *types_fresh_tvar(void);

Scheme types_gen(Type *t);
Expr types_inst(char *var, Scheme s);

Scheme types_get_ctx(char *var);
void types_add_var(char *var, Scheme s);
int types_get_ctx_len(void);
void types_set_ctx_len(int len);

#endif

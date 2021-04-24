#ifndef __TYPES_H_
#define __TYPES_H_

#include "common.h"
#include "syntax.h"

Type types_mktcon(char *s, int arity, ...);
void types_unify(Type t1, Type t2);
void types_eval(Type *t);
void types_eval_expr(Expr *e);
Type types_fresh_tvar(void);

#endif

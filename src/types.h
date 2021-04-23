#ifndef __TYPES_H_
#define __TYPES_H_

#include "common.h"
#include "syntax.h"

Type mktcon(char *s, int arity, ...);
void unify(Type t1, Type t2);
Type eval_type(Type t);

#endif

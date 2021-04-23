#include <stdarg.h>
#include <string.h>

#include "array.h"
#include "syntax.h"
#include "memory.h"
#include "types.h"

static void unify_error(void);

static void bind(int v, Type t);

Type
mktcon(char *s, int arity, ...)
{
	Type t;
	va_list ap;

	va_start(ap, arity);
	t.type = T_CON;
	t.arity = arity;
	t.targs = emalloc(arity * sizeof(Type));
	for (int i = 0; i < arity; ++i)
		t.targs[i] = va_arg(ap, Type);
	va_end(ap);
	return t;
}

void
unify(Type t1, Type t2)
{
	if (t1.type == T_CON && t2.type == T_CON) {
		if (!strcmp(t1.name, t2.name) && t1.arity == t2.arity) {
			for (int i = 0; i < t1.arity; ++i)
				unify(t1.targs[i], t2.targs[i]);
		} else {
			unify_error();
		}
	} else if (t1.type == T_FUN && t2.type == T_FUN) {

	}
}

Type eval_type(Type t);

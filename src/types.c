#include "array.h"
#include "syntax.h"
#include "types.h"

Type
mktcon(char *s)
{
	Type t;

	t.type = T_CON;
	array_init(&t.args, sizeof(char *));
	return t;
}

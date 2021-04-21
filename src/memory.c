#include <stdlib.h>

#include "memory.h"

void *
emalloc(size_t s)
{
	void *p;

	p = malloc(s);
	if (p == NULL)
		exit(1);
	return p;
}

void *
erealloc(void *p, size_t os, size_t ns)
{
	if (ns == 0) {
		free(p);
		return NULL;
	}
	p = os == 0 ? malloc(ns) : realloc(p, ns);

	if (p == NULL)
		exit(1);
	return p;
}

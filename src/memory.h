#ifndef __MEMORY_H_
#define __MEMORY_H_

#include "common.h"

void *emalloc(size_t s);
void *erealloc(void *p, size_t os, size_t ns);
int grow_capacity(int n);

#endif

#ifndef __MEMORY_H_
#define __MEMORY_H_

#include "common.h"

void *emalloc(size_t);
void *erealloc(void *, size_t, size_t);

#endif

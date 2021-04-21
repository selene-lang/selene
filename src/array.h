#ifndef __ARRAY_H_
#define __ARRAY_H_

#include "common.h"

typedef struct {
	int length;
	int capacity;
	size_t esize;
	void *p;
} Array;

void array_init(Array *array, size_t size);
void array_write(Array *array, void *p);
void array_free(Array *array);

#endif

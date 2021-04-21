#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "memory.h"

void
array_init(Array *array, size_t size)
{
	array->p = NULL;
	array->length = 0;
	array->capacity = 0;
	array->esize = size;
}

void
array_write(Array *array, void *p)
{
	if (array->capacity < array->length + 1) {
		int old_capacity = array->capacity;
		array->capacity = grow_capacity(old_capacity);
		array->p = erealloc(array->p, old_capacity * array->esize,
		                    array->capacity * array->esize);
	}
	memcpy(array->p + array->length * array->esize, p, array->esize);
	++array->length;
}

void
array_free(Array *array)
{
	erealloc(array->p, array->capacity * array->esize, 0);
	array_init(array, array->esize);
}

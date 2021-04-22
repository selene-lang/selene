#ifndef __CHUNK_H_
#define __CHUNK_H_

#include "array.h"
#include "common.h"

typedef enum {
	OP_RETURN
} OpCode;

typedef struct {
	Array code;
	Array values;
} Chunk;

void chunk_init(Chunk *);
void chunk_write(Chunk *, u8);
void chunk_free(Chunk *);

#endif

#ifndef __CHUNK_H_
#define __CHUNK_H_

#include "common.h"

typedef enum {
	OP_RETURN
} OpCode;

typedef struct {
	int length;
	int capacity;
	u8 *code;
} Chunk;

void chunk_init(Chunk *);
void chunk_write(Chunk *, u8);

#endif

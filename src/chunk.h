#ifndef __CHUNK_H_
#define __CHUNK_H_

#include "array.h"
#include "common.h"

typedef enum {
	OP_RETURN,
	OP_ADDI, OP_SUBI, OP_MULTI, OP_DIVI, OP_EQUI
} OpCode;

typedef struct {
	Array code;
	Array values;
} Chunk;

typedef struct {
	OpCode op;
	u8 a, b, c;
} Instruction;

void chunk_init(Chunk *);
void chunk_write(Chunk *, Instruction);
void chunk_free(Chunk *);

#endif

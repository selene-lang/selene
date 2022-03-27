#ifndef __CHUNK_H_
#define __CHUNK_H_

#include "array.h"
#include "common.h"

typedef enum {
	OP_RET, OP_VOID,
	OP_ADDI, OP_SUBI, OP_MULI, OP_DIVI, OP_EQUI,
	OP_CJMP, OP_NJMP, OP_UJMP,
	OP_CALL, OP_CCALL,
	OP_MOV
} OpCode;

typedef struct {
	Array code;
	u64 values[128];
} Chunk;

typedef struct {
	OpCode op;
	u8 a, b, c;
} Instruction;

void chunk_init(Chunk *);
void chunk_write(Chunk *, Instruction);
void chunk_leave_space(Chunk *, int, size_t);
void chunk_write_addr(Chunk *, int, u32);
void chunk_write_ptr(Chunk *, int, u64);
void chunk_free(Chunk *);

#endif

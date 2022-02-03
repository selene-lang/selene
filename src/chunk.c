#include <stdlib.h>

#include "common.h"

#include "array.h"
#include "chunk.h"
#include "memory.h"

void
chunk_init(Chunk *chunk)
{
	array_init(&chunk->code, sizeof(Instruction));
}

void
chunk_write(Chunk *chunk, Instruction i)
{
	array_write(&chunk->code, &i);
}

void
chunk_write_addr(Chunk *chunk, int i, u32 addr)
{
	((u32*)chunk->code.p)[i] = addr;
}

void
chunk_free(Chunk *chunk)
{
	array_free(&chunk->code);
	chunk_init(chunk);
}

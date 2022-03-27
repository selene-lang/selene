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
chunk_leave_space(Chunk *chunk, int i, size_t s)
{
	while ((long)(chunk->code.capacity - i) * (chunk->code.esize) < s)
		chunk_write(chunk, (Instruction){0});
}

void
chunk_write_addr(Chunk *chunk, int i, u32 addr)
{
	((u32*)chunk->code.p)[i] = addr;
}

void
chunk_write_ptr(Chunk *chunk, int i, u64 p)
{
	chunk_leave_space(chunk, i, sizeof(u64));
	((u64*)chunk->code.p)[i] = p;
}

void
chunk_free(Chunk *chunk)
{
	array_free(&chunk->code);
	chunk_init(chunk);
}

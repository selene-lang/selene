#include <stdlib.h>

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
chunk_free(Chunk *chunk)
{
	array_free(&chunk->code);
	array_free(&chunk->values);
	chunk_init(chunk);
}

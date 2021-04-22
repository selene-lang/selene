#include <stdlib.h>

#include "array.h"
#include "chunk.h"
#include "memory.h"
#include "value.h"

void
chunk_init(Chunk *chunk)
{
	array_init(&chunk->code, sizeof(u8));
	array_init(&chunk->values, sizeof(Value));
}

void
chunk_write(Chunk *chunk, u8 byte)
{

}

void
chunk_free(Chunk *chunk)
{
	array_free(&chunk->code);
	array_free(&chunk->values);
	chunk_init(chunk);
}

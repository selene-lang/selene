#include <stdlib.h>

#include "chunk.h"

void
chunk_init(Chunk *chunk)
{
	chunk->code = NULL;
	chunk->length = 0;
	chunk->capacity = 0;
}

#ifndef __COMPILE_H_
#define __COMPILE_H_

#include "common.h"
#include "chunk.h"

typedef struct {
	struct {
		int nreg;
		char *name;
	} var[128];
	u8 regs[128];
	Chunk *chunk;
} CompileContext;

#endif

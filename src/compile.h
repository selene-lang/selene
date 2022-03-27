#ifndef __COMPILE_H_
#define __COMPILE_H_

#include "common.h"
#include "chunk.h"
#include "syntax.h"

typedef struct {
	struct {
		int nreg;
		char *name;
	} var[128];
	u8 regs[128];
	int nconst;
	Chunk chunk;
} CompileContext;

typedef struct {
	Chunk *fun;
	FunPtr *ext;
	int nfun;
	int next;
} Program;

Program compile_program(Array p);

#endif /* __COMPILE_H_ */

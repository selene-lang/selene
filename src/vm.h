#ifndef __VM_H_
#define __VM_H_

#include "chunk.h"
#include "common.h"
#include "syntax.h"

typedef struct {
	Chunk *prog;
	Chunk code;
	int pc;
	u64 reg[128];
} VM;

void vm_init(VM *, Chunk *, Chunk);
u64 vm_run(VM);

void vm_run_program(Array);

#endif

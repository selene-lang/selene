#ifndef __VM_H_
#define __VM_H_

#include "chunk.h"
#include "common.h"
#include "syntax.h"

typedef struct {
	Chunk *prog;
	Chunk code;
	int pc;
	int reg[128];
} VM;

void vm_init(VM *, Chunk *, Chunk);
int vm_run(VM);

void vm_run_program(Array);

#endif

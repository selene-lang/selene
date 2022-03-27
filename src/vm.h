#ifndef __VM_H_
#define __VM_H_

#include "chunk.h"
#include "common.h"
#include "compile.h"
#include "syntax.h"

typedef struct {
	Program prog;
	Chunk code;
	int pc;
	u64 reg[128];
} VM;

void vm_init(VM *, Program, Chunk);
u64 vm_run(VM);

void vm_run_program(Array);

#endif

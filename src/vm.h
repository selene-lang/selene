#ifndef __VM_H_
#define __VM_H_

#include "chunk.h"
#include "common.h"

typedef struct {
	Chunk *code;
	int pc;
} VM;

void vm_init(void);
void vm_free(void);

#endif

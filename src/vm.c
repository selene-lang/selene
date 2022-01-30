#include "vm.h"

void
vm_init(VM *vm, Chunk c)
{
	vm->pc = 0;
	vm->code = c;
}

int
vm_run(VM vm)
{
	Instruction *p;

	p = (Instruction *)vm.code.code.p;
	for (;;) {
		switch (p[vm.pc].op) {
			
		}
	}
}

void
vm_free(VM *vm)
{
}

void
vm_run_program(Array p)
{
	
}

#include "vm.h"

static int load(u8 r, VM *vm);

static int
load(u8 r, VM *vm)
{
	if (r >= 128)
		return ((int *)vm->code.values.p)[r - 128];
	else
		return vm->reg[r];
}

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
		Instruction i = p[vm.pc];
		switch (i.op) {
		case OP_ADDI:
			vm.reg[i.a] = load(i.b, &vm) + load(i.c, &vm);
			break;
		case OP_SUBI:
			vm.reg[i.a] = load(i.b, &vm) - load(i.c, &vm);
			break;
		case OP_MULI:
			vm.reg[i.a] = load(i.b, &vm) * load(i.c, &vm);
			break;
		case OP_DIVI:
			vm.reg[i.a] = load(i.b, &vm) / load(i.c, &vm);
			break;
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

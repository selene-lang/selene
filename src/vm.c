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
vm_init(VM *vm, Chunk *prog, Chunk c)
{
	vm->pc = 0;
	vm->code = c;
	vm->prog = prog;
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
		case OP_EQUI:
			vm.reg[i.a] = (load(i.b, &vm) == load(i.c, &vm));
			break;
		case OP_CJMP:
			++vm.pc;
			if (load(i.a, &vm))
				vm.pc = *(int *)(p + vm.pc) - 1;
			break;
		case OP_UJMP:
			vm.pc = *(int *)(p + vm.pc + 1) - 1;
			break;
		case OP_NJMP:
			++vm.pc;
			if (load(i.a, &vm) == 0)
				vm.pc = *(int *)(p + vm.pc) - 1;
			break;
		case OP_MOV:
			vm.reg[i.a] = load(i.b, &vm);
			break;
		case OP_VOID:
			return 0;
		case OP_RET:
			return load(i.a, &vm);
		case OP_CALL: {
			VM f;
			vm_init(&f, vm.prog, vm.prog[load(i.b, &vm)]);
			for (int j = 0; j < i.c; ++j)
				f.reg[j] = load(p[vm.pc + j + 1].a, &vm);
			vm.reg[i.a] = vm_run(f);
			vm.pc += i.c;
			break;
		}
		}
		++vm.pc;
	}
}

void
vm_run_program(Array p)
{
	
}

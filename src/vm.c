#include "vm.h"

static u64 load(u8 r, VM *vm);
static long loadl(u8 r, VM *vm);

static u64
load(u8 r, VM *vm)
{
	if (r >= 128)
		return vm->code->values[r - 128];
	else
		return vm->reg[r];
}

static long
loadl(u8 r, VM *vm)
{
	return (long)load(r, vm);
}

void
vm_init(VM *vm, Chunk *c)
{
	vm->pc = 0;
	vm->code = c;
}

u64
vm_run(VM *vm)
{
	Instruction *p;

	p = (Instruction *)vm->code->code.p;
	for (;;) {
		Instruction i = p[vm->pc];
		switch (i.op) {
		case OP_ADDI:
			vm->reg[i.a] = (u64)(loadl(i.b, vm) + loadl(i.c, vm));
			break;
		case OP_SUBI:
			vm->reg[i.a] = (u64)(loadl(i.b, vm) - loadl(i.c, vm));
			break;
		case OP_MULI:
			vm->reg[i.a] = (u64)(loadl(i.b, vm) * loadl(i.c, vm));
			break;
		case OP_DIVI:
			vm->reg[i.a] = (u64)(loadl(i.b, vm) / loadl(i.c, vm));
			break;
		case OP_EQUI:
			vm->reg[i.a] = (u64)(long)(load(i.b, vm) == load(i.c, vm));
			break;
		case OP_GRTI:
			vm->reg[i.a] = (u64)(long)(load(i.b, vm) > load(i.c, vm));
			break;
		case OP_LWRI:
			vm->reg[i.a] = (u64)(long)(load(i.b, vm) < load(i.c, vm));
			break;
		case OP_CJMP:
			++vm->pc;
			if (load(i.a, vm))
				vm->pc = *(u32 *)(p + vm->pc) - 1;
			break;
		case OP_UJMP:
			vm->pc = *(int *)(p + vm->pc + 1) - 1;
			break;
		case OP_NJMP:
			++vm->pc;
			if (load(i.a, vm) == 0)
				vm->pc = *(int *)(p + vm->pc) - 1;
			break;
		case OP_MOV:
			vm->reg[i.a] = load(i.b, vm);
			break;
		case OP_VOID:
			return 0;
		case OP_RET:
			return load(i.a, vm);
		case OP_CALL: {
			VM f;
			vm_init(&f, vm_prog.fun + loadl(i.b, vm));
			for (int j = 0; j < i.c; ++j)
				f.reg[j] = load(p[vm->pc + j + 1].a, vm);
			vm->reg[i.a] = vm_run(&f);
			vm->pc += i.c;
			break;
		}
		case OP_CCALL: {
			u64 arg[i.c];
			for (int j = 0; j < i.c; ++j)
				arg[j] = load(p[vm->pc + j + 1].a, vm);
			vm->reg[i.a] = vm_prog.ext[loadl(i.b, vm)]((void *)arg);
			vm->pc += i.c;
			break;
		}
		}
		++vm->pc;
	}
}

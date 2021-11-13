#include "compile.h"

#include "chunk.h"
#include "syntax.h"

static u8 regs[128] = {0};

static void add_instruction(Instruction i, Chunk *c);
static u8 new_reg(void);
static void free_reg(u8 reg);
static u8 add_const_int(int n, Chunk *c);
static u8 compile_op(Expr *lhs, Expr *rhs, int op, Chunk *c);
static u8 compile_expr(Expr e, Chunk *c);

static void
add_instruction(Instruction i, Chunk *c)
{
	array_write(&c->code, &i);
}

static u8
new_reg(void)
{
	for (int i = 0; i < 128; ++i) {
		if (regs[i] == 0) {
			regs[i] = 1;
			return i;
		}
	}
	return -1;
}

static void
free_reg(u8 reg)
{
	regs[reg] = 0;
}

static u8
add_const_int(int n, Chunk *c)
{
	array_write(&c->values, &n);
	return c->values.length - 1;
}

static u8
compile_op(Expr *lhs, Expr *rhs, int op, Chunk *c)
{
	u8 l, r, o;
	Instruction i;

	l = compile_expr(*lhs, c);
	o = new_reg();
	if (rhs != NULL)
		r = compile_expr(*rhs, c);
	switch (op) {
	case O_PLUS:
		i.a = o;
		i.b = l;
		i.c = r;
		i.op = OP_ADDI;
		free_reg(l);
		free_reg(r);
		break;
	}
	add_instruction(i, c);
	return o;
}

static u8
compile_expr(Expr e, Chunk *c)
{
	switch (e.type) {
	case E_NUM:
		return add_const_int(e.number, c);
	case E_OP:
		break;
	}
}

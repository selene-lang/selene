#include "common.h"

#include "compile.h"

#include "chunk.h"
#include "syntax.h"

static void add_instruction(Instruction i, Chunk *c);
static u8 new_reg(CompileContext *c);
static void free_reg(u8 reg, CompileContext *c);
static u8 add_const_int(int n, CompileContext *c);
static u8 compile_op(Expr *lhs, Expr *rhs, int op, CompileContext *c);
static u8 compile_expr(Expr e, CompileContext *c);

static void
add_instruction(Instruction i, Chunk *c)
{
	array_write(&c->code, &i);
}

static u8
new_reg(CompileContext *c)
{
	for (int i = 0; i < 128; ++i) {
		if (c->regs[i] == 0) {
			c->regs[i] = 1;
			return i;
		}
	}
	return -1;
}

static void
free_reg(u8 reg, CompileContext *c)
{
	c->regs[reg] = 0;
}

static u8
add_const_int(int n, CompileContext *c)
{
	array_write(&c->chunk->values, &n);
	return c->chunk->values.length - 1;
}

static u8
compile_op(Expr *lhs, Expr *rhs, int op, CompileContext *c)
{
	u8 l, r, o;
	Instruction i;

	l = compile_expr(*lhs, c);
	o = new_reg(c);
	i.a = o;
	i.b = l;
	if (rhs != NULL) {
		r = compile_expr(*rhs, c);
		i.c = r;
	}
	switch (op) {
	case O_PLUS:
		i.op = OP_ADDI;
		break;
	case O_MINUS:
		i.op = OP_SUBI;
		break;
	}
	free_reg(l, c);
	if (rhs != NULL)
		free_reg(r, c);

	add_instruction(i, c->chunk);
	return o;
}

static u8
compile_expr(Expr e, CompileContext *c)
{
	switch (e.type) {
	case E_NUM:
		return add_const_int(e.number, c);
	case E_OP:
		return compile_op(e.left, e.right, e.op, c);
	default:
		return -1;
	}
}

#include <string.h>

#include "common.h"

#include "compile.h"

#include "chunk.h"
#include "syntax.h"

static u8 new_reg(CompileContext *c);
static void free_reg(u8 reg, CompileContext *c);
static u8 add_const_int(int n, CompileContext *c);
static int jmp(u8 r, OpCode op, CompileContext *c);

static u8 find_var(char *var, CompileContext *c);
static u8 compile_op(Expr *lhs, Expr *rhs, int op, CompileContext *c);
static u8 compile_expr(Expr e, CompileContext *c);
static void compile_statement(Statement s, CompileContext *c);
static void compile_body(Array a, CompileContext *c);
static Chunk *compile_function(Function f);

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

static int
jmp(u8 r, OpCode op, CompileContext *c)
{
	Instruction i;

	i.op = op;
	i.a = r;
	chunk_write(c->chunk, i);

	/* Leave room for the address. */
	chunk_write(c->chunk, (Instruction){0});
	chunk_write(c->chunk, (Instruction){0});

	return c->chunk->code.length - 2;
}

static u8
find_var(char *var, CompileContext *c)
{
	for (int i = 0; i < 128; ++i)
		if(!strcmp(var, c->var[i].name))
			return i + 128;
	exit(1);
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

	chunk_write(c->chunk, i);
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
	case E_VAR:
		return find_var(e.name, c);
	default:
		return -1;
	}
}

static void
compile_statement(Statement s, CompileContext *c)
{
	switch (s.type) {
	case S_IF: {
		u8 r = compile_expr(s.e, c);
		int if_addr = jmp(r, OP_CJMP, c);
		if (s.elseb.length != 0) {
			compile_body(s.elseb, c);
		}
		int end_addr = jmp(0, OP_UJMP, c);
		break;
	}
	default:
		break;
	}
}

static void
compile_body(Array a, CompileContext *c)
{
	for (int i = 0; i < a.length; ++i)
		compile_statement(((Statement *)a.p)[i], c);
}

static Chunk *
compile_function(Function f)
{
}

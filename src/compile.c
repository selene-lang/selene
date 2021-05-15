#include "compile.h"

#include "chunk.h"
#include "syntax.h"

static void add_instruction(Instruction i, Chunk *c);
static u8 new_reg(void);
static u8 add_const_int(int n, Chunk *c);
static u8 compile_op(Expr *lhs, Expr *rhs, int op, Chunk *c);
static u8 compile_expr(Expr e, Chunk *c);

static void
add_instruction(Instruction i, Chunk *c)
{
	array_write(&c->code, &i);
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
		break;
	}
	add_instruction(i, c);
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

#include <string.h>

#include "common.h"

#include "compile.h"

#include "chunk.h"
#include "syntax.h"

static u8 new_reg(CompileContext *c);
static void free_reg(u8 reg, CompileContext *c);
static u8 add_const_int(int n, CompileContext *c);
static int jmp(u8 r, OpCode op, CompileContext *c);
static int jmp_addr(u8 r, OpCode op, int addr, CompileContext *c);
static void add_var(char *var, CompileContext *c);
static u8 find_var(char *var, CompileContext *c);
static u8 move_no_dest(u8 r, int dest, CompileContext *c);

static u8 compile_var(char *var, int dest, CompileContext *c);
static u8 compile_int(int n, int dest, CompileContext *c);
static u8 compile_op(Expr *lhs, Expr *rhs, int op, int dest, CompileContext *c);
static u8 compile_fun_call(Expr f, Array args, int dest, CompileContext *c);

static u8 compile_expr(Expr e, int dest, CompileContext *c);
static void compile_statement(Statement s, CompileContext *c);
static void compile_body(Array a, CompileContext *c);
static Chunk compile_function(Function f);

Array fun_ctx;

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
	if (reg >= 128)
		return;

	if (c->regs[reg] == 1)
		c->regs[reg] = 0;
}

static u8
add_const_int(int n, CompileContext *c)
{
	if (c->nconst >= 128)
		exit(1);
	c->chunk.values[c->nconst++] = n;
	return 128 + c->nconst - 1;
}

static int
jmp(u8 r, OpCode op, CompileContext *c)
{
	Instruction i;
	int index;

	i.op = op;
	i.a = r;
	chunk_write(&c->chunk, i);

	/* Leave room for the address. */
	index = c->chunk.code.length - 1;
	chunk_write_addr(&c->chunk, index, 0);

	return index - 4;
}

static int
jmp_addr(u8 r, OpCode op, int addr, CompileContext *c)
{
	Instruction i;
	int index;

	i.op = op;
	i.a = r;
	chunk_write(&c->chunk, i);
	index = c->chunk.code.length - 1;
	chunk_write_addr(&c->chunk, index, addr);

	return index - 4;
}

static void
add_var(char *var, CompileContext *c)
{
	for (int i = 0; i < 128; ++i) {
		if (c->regs[i] == 0) {
			c->var[i].name = var;
			c->var[i].nreg = i;
			c->regs[i] = 2;
			return;
		}
	}
}

static u8
find_var(char *var, CompileContext *c)
{
	for (int i = 0; i < 128; ++i)
		if (c->var[i].name != NULL && !strcmp(var, c->var[i].name))
			return i;
	for (int i = 0; i < fun_ctx.length; ++i)
		if (!strcmp(var, ((char **)fun_ctx.p)[i]))
			return i;
	exit(1);
}

static u8
move_no_dest(u8 r, int dest, CompileContext *c)
{
	if (dest != -1) {
		Instruction i = {.op = OP_MOV, .a = (u8)dest, .b = r};
		chunk_write(&c->chunk, i);
		return (u8)dest;
	} else {
		return r;
	}
}

static u8
compile_var(char *var, int dest, CompileContext *c)
{
	return move_no_dest(find_var(var, c), dest, c);
}

static u8
compile_int(int n, int dest, CompileContext *c)
{
	return move_no_dest(add_const_int(n, c), dest, c);
}

static u8
compile_op(Expr *lhs, Expr *rhs, int op, int dest, CompileContext *c)
{
	Instruction i;

	if (dest == -1)
		dest = new_reg(c);
	i.a = dest;
	i.b = compile_expr(*lhs, -1, c);
	if (rhs != NULL)
		i.c = compile_expr(*rhs, -1, c);
	switch (op) {
	case O_PLUS:
		i.op = OP_ADDI;
		break;
	case O_MINUS:
		i.op = OP_SUBI;
		break;
	case O_MULT:
		i.op = OP_MULI;
		break;
	case O_DIV:
		i.op = OP_DIVI;
		break;
	case O_EQU:
		i.op = OP_EQUI;
		break;
	}

	free_reg(i.b, c);
	if (rhs != NULL)
		free_reg(i.c, c);
	chunk_write(&c->chunk, i);
	return (u8)dest;
}

static u8
compile_fun_call(Expr f, Array args, int dest, CompileContext *c)
{
	Instruction i;
	Expr *x;

	if (dest == -1)
		dest = new_reg(c);
	i.op = OP_CALL;
	i.a = dest;
	i.b = compile_expr(f, -1, c);
	i.c = args.length;
	chunk_write(&c->chunk, i);

	x = (Expr *)args.p;
	i = (Instruction){ 0 };
	for (int j = 0; j < args.length; ++j) {
		i.a = compile_expr(x[j], -1, c);
		chunk_write(&c->chunk, i);
	}
	return dest;
}

static u8
compile_expr(Expr e, int dest, CompileContext *c)
{
	switch (e.type) {
	case E_NUM:
		return compile_int(e.number, dest, c);
	case E_OP:
		return compile_op(e.left, e.right, e.op, dest, c);
	case E_VAR:
		return compile_var(e.name, dest, c);
	case E_FUNCALL:
		return compile_fun_call(*e.left, e.args, dest, c);
	}
}

static void
compile_statement(Statement s, CompileContext *c)
{
	switch (s.type) {
	case S_IF: {
		u8 r = compile_expr(s.e, -1, c);
		int if_addr = jmp(r, OP_CJMP, c);
		free_reg(r, c);
		if (s.elseb.length != 0) {
			compile_body(s.elseb, c);
		}
		int end_addr = jmp(0, OP_UJMP, c);
		chunk_write_addr(&c->chunk, if_addr, c->chunk.code.length);
		compile_body(s.body, c);
		chunk_write_addr(&c->chunk, end_addr, c->chunk.code.length);
		break;
	}
	case S_WHILE: {
		int begin = c->chunk.code.length;
		u8 r = compile_expr(s.e, -1, c);
		int end_addr = jmp(r, OP_NJMP, c);
		free_reg(r, c);
		compile_body(s.body, c);
		jmp_addr(0, OP_UJMP, begin, c);
		chunk_write_addr(&c->chunk, end_addr, c->chunk.code.length);
		break;
	}
	case S_EXPR: {
		free_reg(compile_expr(s.e, -1, c), c);
		break;
	}
	case S_RETURN: {
		Instruction i = {.a = compile_expr(s.e, -1, c), .op = OP_RET};
		free_reg(i.a, c);
		chunk_write(&c->chunk, i);
		break;
	}
	case S_VAR_DECL: {
		add_var(s.name, c);
		break;
	}
	}
}

static void
compile_body(Array a, CompileContext *c)
{
	for (int i = 0; i < a.length; ++i)
		compile_statement(((Statement *)a.p)[i], c);
}

static Chunk
compile_function(Function f)
{
	CompileContext c;

	memset(&c, 0, sizeof(CompileContext));
	chunk_init(&c.chunk);

	for (int i = 0; i < f.args.length; ++i)
		add_var(((char **)f.args.p)[i], &c);
	compile_body(f.body, &c);
	return c.chunk;
}

Array
compile_program(Array p)
{
	Array c;

	array_init(&fun_ctx, sizeof(char *));
	array_init(&c, sizeof(Chunk));
	for (int i = 0; i < p.length; ++i) {
		array_write(&fun_ctx, &((Function *)p.p)->name);
		Chunk chnk = compile_function(((Function *)p.p)[i]);
		array_write(&c, &chnk);
	}
	return c;
}

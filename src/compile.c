#include <string.h>

#include "common.h"

#include "compile.h"

#include "chunk.h"
#include "slnlib.h"
#include "syntax.h"

static u8 new_reg(CompileContext *c);
static void free_reg(u8 reg, CompileContext *c);
static u8 add_const_int(long n, CompileContext *c);
static int jmp(u8 r, OpCode op, CompileContext *c);
static int jmp_addr(u8 r, OpCode op, int addr, CompileContext *c);
static u8 add_var(char *var, CompileContext *c);
static u8 move_no_dest(u8 r, int dest, CompileContext *c);
static FunPtr find_extern(char *);

static u8 compile_int(int n, int dest, CompileContext *c);
static u8 compile_var(char *var, int dest, CompileContext *c);
static u8 compile_assign(u8 v, Expr e, int dest, CompileContext *c);
static u8 compile_op(Expr *lhs, Expr *rhs, int op, int dest, CompileContext *c);
static u8 compile_fun_call(Expr f, Array args, int dest, CompileContext *c);

static u8 compile_expr(Expr e, int dest, CompileContext *c);
static void compile_statement(Statement s, CompileContext *c);
static void compile_body(Array a, CompileContext *c);
static Chunk compile_function(Function f);
static FunPtr compile_extern(Extern e);

static struct ext {
	char *name;
	FunPtr f;
} slnlib_ext_dict[] = {
	{"c_print_int", c_print_int},
	{"c_print_char", c_print_char},
	{"c_print_bool", c_print_bool},
	{"c_print_newline", c_print_newline}
};

static Array fun_ctx = {
	.p = NULL,
	.esize = sizeof(char *),
	.length = 0,
	.capacity = 0
};

static Array ext_ctx = {
	.p = NULL,
	.esize = sizeof(char *),
	.length = 0,
	.capacity = 0
};

static Array ext_dict  = {
	.p = NULL,
	.esize = sizeof(struct ext),
	.length = 0,
	.capacity = 0
};

static int is_extern;

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
add_const_int(long n, CompileContext *c)
{
	if (c->nconst >= 128)
		exit(1);
	c->chunk.values[c->nconst++] = (u64)(n << 1);
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

	index = c->chunk.code.length;
	chunk_leave_space(&c->chunk, index, sizeof(u32));

	return index;
}

static int
jmp_addr(u8 r, OpCode op, int addr, CompileContext *c)
{
	Instruction i;
	int index;

	i.op = op;
	i.a = r;
	chunk_write(&c->chunk, i);
	index = c->chunk.code.length;
	chunk_leave_space(&c->chunk, index, sizeof(u32));
	chunk_write_addr(&c->chunk, index, addr);

	return index;
}

static u8
add_var(char *var, CompileContext *c)
{
	for (int i = 0; i < 128; ++i) {
		if (c->regs[i] == 0) {
			c->var[i].name = var;
			c->var[i].nreg = i;
			c->var[i].is_owned = 1;
			c->regs[i] = 2;
			return i;
		}
	}
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

static FunPtr
find_extern(char *s)
{
	for (int i = 0; i < ext_dict.length; ++i)
		if (!strcmp(s, ((struct ext *)ext_dict.p)[i].name))
			return ((struct ext *)ext_dict.p)[i].f;
	exit(1);
}

static u8
compile_int(int n, int dest, CompileContext *c)
{
	return move_no_dest(add_const_int(n, c), dest, c);
}

static u8
compile_var(char *var, int dest, CompileContext *c)
{

	for (int i = 0; i < 128; ++i) {
		if (c->var[i].name != NULL && !strcmp(var, c->var[i].name)) {
			if (c->var[i].is_owned)
				c->var[i].is_owned = 0;
			return move_no_dest(i, dest, c);
		}
	}

	for (int i = 0; i < fun_ctx.length; ++i) {
		if (!strcmp(var, ((char **)fun_ctx.p)[i])) {
			is_extern = 0;
			return compile_int(i, dest, c);
		}
	}

	for (int i = 0; i < ext_ctx.length; ++i) {
		if (!strcmp(var, ((char **)ext_ctx.p)[i])) {
			is_extern = 1;
			return compile_int(i, dest, c);
		}
	}

	exit(1);
}

static u8
compile_assign(u8 v, Expr e, int dest, CompileContext *c)
{
	u8 r;

	compile_expr(e, v, c);
	r = move_no_dest(v, dest, c);
	return r;
}

static u8
compile_op(Expr *lhs, Expr *rhs, int op, int dest, CompileContext *c)
{
	Instruction i;

	if (op == O_ASSGN) {
		u8 v = compile_expr(*lhs, -1, c);
		return compile_assign(v, *rhs, dest, c);
	}
	i.a = dest == -1 ? new_reg(c) : dest;
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
	case O_GRT:
		i.op = OP_GRTI;
		break;
	case O_LWR:
		i.op = OP_LWRI;
		break;
	}

	free_reg(i.b, c);
	if (rhs != NULL)
		free_reg(i.c, c);
	chunk_write(&c->chunk, i);
	return i.a;
}

static u8
compile_fun_call(Expr f, Array args, int dest, CompileContext *c)
{
	Instruction i;
	u8 cargs[args.length];
	Expr *x;

	x = (Expr *)args.p;
	for (int j = 0; j < args.length; ++j)
		cargs[j] = compile_expr(x[j], -1, c);
	if (dest == -1)
		dest = new_reg(c);
	i.a = dest;
	i.b = compile_expr(f, -1, c);
	i.op = is_extern ? OP_CCALL : OP_CALL;
	i.c = args.length;
	chunk_write(&c->chunk, i);

	x = (Expr *)args.p;
	i = (Instruction){ 0 };
	for (int j = 0; j < args.length; ++j) {
		i.a = cargs[j];
		free_reg(i.a, c);
		chunk_write(&c->chunk, i);
	}
	return dest;
}

static u8
compile_expr(Expr e, int dest, CompileContext *c)
{
	switch (e.type) {
	case E_INT:
		return compile_int(e.inumber, dest, c);
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
		if (s.elseb.length != 0)
			compile_body(s.elseb, c);
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
		u8 v = add_var(s.name, c);
		compile_assign(v, s.e, -1, c);
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

	array_write(&fun_ctx, &f.name);
	memset(&c, 0, sizeof(CompileContext));
	chunk_init(&c.chunk);

	for (int i = 0; i < f.args.length; ++i)
		(void)add_var(((char **)f.args.p)[i], &c);
	compile_body(f.body, &c);
	return c.chunk;
}

static FunPtr
compile_extern(Extern e)
{
	array_write(&ext_ctx, &e.name);
	return find_extern(e.name);
}

Program
compile_program(Array p)
{
	Array c;
	Array e;
	Program prog;
	TopLevel tl;

	array_init(&c, sizeof(Chunk));
	array_init(&e, sizeof(FunPtr));

	for (int i = 0; i < sizeof(slnlib_ext_dict) / sizeof(struct ext); ++i)
		array_write(&ext_dict, slnlib_ext_dict + i);

	for (int i = 0; i < p.length; ++i) {
		tl = ((TopLevel *)p.p)[i];
		if (tl.type == TL_FUN) {
			Chunk chnk = compile_function(tl.fun);
			array_write(&c, &chnk);
		} else if (tl.type == TL_EXT) {
			FunPtr ext = compile_extern(tl.ext);
			array_write(&e, &ext);
		}
	}

	prog.fun = (Chunk *)c.p;
	prog.nfun = c.length;
	prog.ext = (FunPtr *)e.p;
	prog.next = e.length;
	return prog;
}

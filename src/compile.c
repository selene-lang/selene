#include "compile.h"

#include "chunk.h"
#include "syntax.h"

static u8 add_const_int(int n, Chunk *c);
static void compile_expr(Expr e, u8 reg, Chunk *c);

static u8
add_const_int(int n, Chunk *c)
{
	
}

static void
compile_expr(Expr e, u8 reg, Chunk *c)
{
	switch (e.type) {
	case E_NUM:
		break;
	}
}

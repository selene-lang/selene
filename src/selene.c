#include <stdio.h>

#include "debug.h"
#include "lexer.h"
#include "parser.h"

char *program = "12 * 3 + 5";

int
main(int argc, char **argv)
{
	parser_init(program);
	print_expr(expr());
}

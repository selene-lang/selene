#include <stdio.h>
#include <locale.h>

#include "debug.h"
#include "lexer.h"
#include "parser.h"

char *program = "fun id(x){if 1 {return 1;}}";

int
main(int argc, char **argv)
{
	setlocale(LC_ALL, "C.UTF-8");
	parser_init(program);
	print_function(top_level());
}

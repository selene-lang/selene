#include <stdio.h>
#include <locale.h>

#include "debug.h"
#include "lexer.h"
#include "parser.h"

char *program = "var a;\n"
	"var b;";

int
main(int argc, char **argv)
{
	setlocale(LC_ALL, "C.UTF-8");
	parser_init(program);
	print_statement(statement());
}

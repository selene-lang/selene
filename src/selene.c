#include <stdio.h>
#include <locale.h>

#include "debug.h"
#include "lexer.h"
#include "parser.h"

char *program = "{var a;var b;a+b;a+1; if 1 {a+b;} else {a+b;} while 1 {a=a+1;} return 1;}";

int
main(int argc, char **argv)
{
	setlocale(LC_ALL, "C.UTF-8");
	parser_init(program);
	print_block(block());
}

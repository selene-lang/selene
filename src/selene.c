#include <stdio.h>
#include <locale.h>

#include "debug.h"
#include "lexer.h"
#include "parser.h"
#include "compile.h"

char *program = "fun add(a, b){return a + b;}fun main(){return add(1, 2);}";

int
main(int argc, char **argv)
{
	Array prog;
	Array f;

	setlocale(LC_ALL, "C.UTF-8");
	parser_init(program);
	prog = parser_program();
	print_program(prog);
	f = compile_program(prog);
	print_cporgram(f);
}

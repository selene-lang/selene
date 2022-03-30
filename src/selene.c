#include <stdio.h>
#include <locale.h>

#include "debug.h"
#include "lexer.h"
#include "parser.h"
#include "compile.h"
#include "vm.h"

char *program = "extern print_int : int -> void; fun main(){return print_int(1);}";

int
main(int argc, char **argv)
{
	Array prog;
	Program f;
	VM vm;

	setlocale(LC_ALL, "C.UTF-8");
	parser_init(program);
	prog = parser_program();
	f = compile_program(prog);
	vm_init(&vm, f, f.fun[0]);
	vm_run(vm);
}

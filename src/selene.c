#include <stdio.h>
#include <locale.h>

#include "debug.h"
#include "lexer.h"
#include "parser.h"
#include "compile.h"
#include "vm.h"

char *program = "extern print_int : int -> void;"
	"extern print_newline : () -> void;"
	"fun main(){print_int(1); print_newline(); print_int(2);}";

int
main(int argc, char **argv)
{
	Array tl;
	Program prog;
	VM vm;

	setlocale(LC_ALL, "C.UTF-8");
	parser_init(program);
	tl = parser_program();
	prog = compile_program(tl);
	vm_init(&vm, prog, prog.fun[0]);
	vm_run(vm);
}

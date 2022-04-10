#include <stdio.h>
#include <locale.h>

#include "debug.h"
#include "lexer.h"
#include "parser.h"
#include "compile.h"
#include "vm.h"

char *program = "extern c_print_int : int -> void;"
	"extern c_print_newline : () -> void;"
	"-- We need this wrapper because of the way extern functions are called.\n"
	"fun print_int(n){c_print_int(n);}"
	"fun main(){let i = 10; while i > 0 {print_int(i); i = i - 1;}}";

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
	vm_init(&vm, prog, prog.fun[1]);
	vm_run(vm);
}

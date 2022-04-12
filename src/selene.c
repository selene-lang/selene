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
	"fun fact(n){if n == 1 {return 1;}else{return n * fact(n-1);}}"
	"fun main(){print_int(fact(6));}";

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
	vm_init(&vm, prog, prog.fun[2]);
	vm_run(vm);
}

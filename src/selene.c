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
	"fun fib(n){if n < 2 {return n;}else{return fib(n-1) + fib(n-2);}}"
	"fun main(){print_int(fib(30));}";

int
main(int argc, char **argv)
{
	Array tl;
	VM vm;

	setlocale(LC_ALL, "C.UTF-8");
	parser_init(program);
	tl = parser_program();
	vm_prog = compile_program(tl);
	vm_init(&vm, vm_prog.fun + 2);
	vm_run(&vm);
}

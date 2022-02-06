#include <stdio.h>
#include <locale.h>

#include "debug.h"
#include "lexer.h"
#include "parser.h"
#include "compile.h"
#include "vm.h"

char *program = "fun add(a, b){return a + b;}fun main(){return add(1, 2);}";

int
main(int argc, char **argv)
{
	Array prog;
	Array f;
	VM vm;

	setlocale(LC_ALL, "C.UTF-8");
	parser_init(program);
	prog = parser_program();
	print_program(prog);
	f = compile_program(prog);
	print_cporgram(f);
	puts("ee");
	vm_init(&vm, (Chunk *)f.p, ((Chunk *)f.p)[1]);
	printf("%d\n", vm_run(vm));
}

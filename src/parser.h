#ifndef __PARSER_H_
#define __PARSER_H_

#include "common.h"
#include "syntax.h"

Expr expr(void);
Statement statement(void);
Array block(void);
void parser_init(char *program);

#endif

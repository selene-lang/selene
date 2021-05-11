#ifndef __PARSER_H_
#define __PARSER_H_

#include "common.h"
#include "syntax.h"

void parser_init(char *program);

Array parser_program(void);

#endif

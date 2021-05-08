#ifndef __PARSER_H_
#define __PARSER_H_

#include "common.h"
#include "syntax.h"

void parser_init(char *program);

Function top_level(void);

#endif

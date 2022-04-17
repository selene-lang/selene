#ifndef __SLNLIB_H_
#define __SLNLIB_H_

#include "common.h"

u64 c_print_int(void *p);
u64 c_print_newline(void *p);
u64 c_print_char(void *p);
u64 c_print_bool(void *p);

u64 c_addi(void *p);
u64 c_subi(void *p);
u64 c_muli(void *p);
u64 c_divi(void *p);

u64 c_addf(void *p);
u64 c_subf(void *p);
u64 c_mulf(void *p);
u64 c_divf(void *p);

#endif /* __SLNLIB_H_ */

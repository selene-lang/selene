#ifndef __SLNLIB_H_
#define __SLNLIB_H_

#include "common.h"

u64 print_int(void *p);
u64 print_newline(void *p);

u64 addi(void *p);
u64 subi(void *p);
u64 muli(void *p);
u64 divi(void *p);

u64 addf(void *p);
u64 subf(void *p);
u64 mulf(void *p);
u64 divf(void *p);

#endif /* __SLNLIB_H_ */

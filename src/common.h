#ifndef __COMMON_H_
#define __COMMON_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef void *u64;

typedef u64 (*FunPtr)(void *);

void *memcpy(void *, const void *, size_t);

#endif /* __COMMON_H_ */

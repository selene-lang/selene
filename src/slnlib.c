#include <stdio.h>

#include "common.h"

#include "slnlib.h"

static u64 tagl(long l);
static long untagl(u64 v);

static u64
tagl(long l)
{
	return (u64)((l << 1) | 1);
}

static long
untagl(u64 v)
{
	return (long)v >> 1;
}

u64
c_print_int(void *p)
{
	printf("%ld", untagl(*(u64 *)p));
	return tagl(0);
}

u64
c_print_newline(void *p)
{
	printf("\n");
	return tagl(0);
}

u64
c_print_char(void *p)
{
	printf("%c", (char)untagl(*(u64 *)p));
	return tagl(0);
}

u64
c_print_bool(void *p)
{
	if (untagl(*(u64 *)p))
		printf("true");
	else
		printf("false");
	return tagl(0);
}

u64
c_addi(void *p)
{
	return tagl(untagl(((u64 *)p)[0]) + untagl(((u64 *)p)[1]));
}

u64
c_subi(void *p)
{
	return tagl(untagl(((u64 *)p)[0]) - untagl(((u64 *)p)[1]));
}

u64
c_muli(void *p)
{
	return tagl(untagl(((u64 *)p)[0]) * untagl(((u64 *)p)[1]));
}

u64
c_divi(void *p)
{
	return tagl(untagl(((u64 *)p)[0]) / untagl(((u64 *)p)[1]));
}

u64
c_addf(void *p)
{
	double d;

	d = ((double *)p)[0] + ((double *)p)[1];
	return ((u64 *)&d)[0];
}

u64
c_subf(void *p)
{
	double d;

	d = ((double *)p)[0] + ((double *)p)[1];
	return ((u64 *)&d)[0];
}

u64
c_mulf(void *p)
{
	double d;

	d = ((double *)p)[0] + ((double *)p)[1];
	return ((u64 *)&d)[0];
}

u64
c_divf(void *p)
{
	double d;

	d = ((double *)p)[0] + ((double *)p)[1];
	return ((u64 *)&d)[0];
}

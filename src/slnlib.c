#include <stdio.h>

#include "common.h"

#include "slnlib.h"

u64
c_print_int(void *p)
{
	printf("%d", *(int *)p);
	return 0;
}

u64
c_print_newline(void *p)
{
	printf("\n");
	return 0;
}

u64
c_addi(void *p)
{
	return (u64)(((long *)p)[0] + ((long *)p)[1]);
}

u64
c_subi(void *p)
{
	return (u64)(((long *)p)[0] - ((long *)p)[1]);
}

u64
c_muli(void *p)
{
	return (u64)(((long *)p)[0] * ((long *)p)[1]);
}

u64
c_divi(void *p)
{
	return (u64)(((long *)p)[0] / ((long *)p)[1]);
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

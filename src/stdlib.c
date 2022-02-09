#include <stdio.h>

#include "common.h"

u64
print_int(void *p)
{
	printf("%d", *(int *)p);
	return 0;
}

u64
print_newline(void *p)
{
	printf("\n");
	return 0;
}

u64
addi(void *p)
{
	return (u64)(((long *)p)[0] + ((long *)p)[1]);
}

u64
subi(void *p)
{
	return (u64)(((long *)p)[0] - ((long *)p)[1]);
}

u64
muli(void *p)
{
	return (u64)(((long *)p)[0] * ((long *)p)[1]);
}

u64
divi(void *p)
{
	return (u64)(((long *)p)[0] / ((long *)p)[1]);
}

u64
addf(void *p)
{
	double d;

	d = ((double *)p)[0] + ((double *)p)[1];
	return ((u64 *)&d)[0];
}

u64
subf(void *p)
{
	double d;

	d = ((double *)p)[0] + ((double *)p)[1];
	return ((u64 *)&d)[0];
}

u64
mulf(void *p)
{
	double d;

	d = ((double *)p)[0] + ((double *)p)[1];
	return ((u64 *)&d)[0];
}

u64
divf(void *p)
{
	double d;

	d = ((double *)p)[0] + ((double *)p)[1];
	return ((u64 *)&d)[0];
}

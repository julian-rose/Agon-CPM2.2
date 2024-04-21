#include "cdb2.h"

/* Copyright (c) 1982-1986 by J. David Kirkland, Jr. */

ucase (s) char *s; {

	/* convert a null-terminated string to upper case */

	while (*s = toupper(*s)) 
		s++;
}

str70cpy(d,s) char *d, *s; {

	/* copy s, a 7-terminated string, to d, a null-terminated string */

	while (0 == ((*d = *s++) &0x80))
		d++;
	*d = 0x7f & *d;
	*++d = 0;
}

str7eq (s1, s2) char *s1, *s2; {

   	/* test two bit-7-terminated strings for equality */

	for (; *s1 == *s2; s2++)
		if (0x80 & *s1++) return 1;

	return 0;
}

str07cpy(d,s) char *d, *s; {

	/* copy s, a null-terminated string, to d, a 7-terminated string */

	while (*s)
		*d++ = *s++;
	*--d |= 0x80;
}

struct fntentry *addr2fnt (addr) char *addr; {

	/* given an address, addr, find which function the address is
	 * within and return the address of the proper fn table entry.
	 */

	/*
	struct fntentry *a2f_p;   /* now global for speed  */
	char *a2f_addr;
	*/

	a2f_p = fntab;
	a2f_addr = addr;
	while ( a2f_addr > (a2f_p--)->fntaddr )
		;

	return a2f_p + 2;
}

struct fntentry *name2fnt(name) char *name; {

	/* find the fn table entry for the function with name "name" (a null-
	 * terminated string).  ERROR returned if no such function exists.
	 */
	struct fntentry *f;
	char n[9];

	movmem(name,n,8);	/* truncate to 8 chars, upper case */
	n[8] = 0;
	ucase(n);

	for (f = fntab; f->fntaddr != 0xffff; f--)
		if (!strcmp(n,f->fntname))
			return f;

	return ERROR;
}


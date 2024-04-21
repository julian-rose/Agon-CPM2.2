#include "cdb2.h"

/* Copyright (c) 1982-1986 by J. David Kirkland, Jr. */

interprete(c) char c; {

	if (c<' ' || c>127)
		putchar('.');
	else
		putchar(c);
}

printp(p) char **p; {

	printf("[%04x] = ", p);
	if (*p==NIL) printf("NIL\n");
	else  	     printf("%04x\n",*p);
}

printc(p) char *p; {

	printf("[%04x] = %02x = '", p,*p);
	interprete(*p);
	printf("'\n");
}

printw(p) int *p; {

	printf("[%04x] = %04x = %6d  '", p,*p,*p);
	interprete(*p&255);
	interprete(*p>>8);
	printf("'\n");
}

/*
printu(p) unsigned *p; {

	printf("[%04x] = %04x = %6u  '", p,*p,*p);
	interprete(*p&255);
	interprete(*p>>8);
	printf("'\n");
}
*/

cancelhit() {

	if (kbhit())
		cancel_char = bdos(6,0xff);

	return cancel_char;
}

sprint (s) char *s; {

	for (; *s; s++)
		if (cancelhit())
			break;
		else if (*s<' ' || *s>127 || *s == '\\')
			switch (*s)  {
			    case '\n': puts("\\n"); break;
			    case '\r': puts("\\r"); break;
			    case '\t': puts("\\t"); break;
			    case '\\': puts("\\\\"); break;
			    default: printf("\\%o",*s); break;
			    }
		else
			putchar(*s);
}

prints(s) char *s; {

	printf("%04x (len %d): \"", s, strlen(s));
	sprint(s);
	printf("\"\n");
}

printca(array, len) char array[][16]; int len; {
	int i, j, m, n;

	n = (len+15)/16;
	for (i=0; i<n; i++) {
		if (cancelhit()) break;
		printf("%04x [%3d]",&array[i][0],i*16);
		m = min(16, len - i*16);
		for (j=0; j<16; j++) {
			if (!(j&7)) putchar(' ');
			if (j<m)
				printf("%02x ",array[i][j]);
			else if (i)
				printf("   ");
			}
		printf("'");
		for (j=0; j<m; j++)
			interprete(array[i][j]);
		printf("'\n");
		}
}

printwa(array, mul) int array[]; int mul; {
	int i, j, m, n;

	for (i=0; i<mul; i+=4) {
		if (cancelhit()) break;
		printf("%04x [%3d] ",&array[i],i);
		for (j=0; j<4; j++)
			if (i+j<mul) printf("%04x ", array[i+j]);
			else if (i) printf("     ");
		printf("= ");
		for (j=0; j<4; j++)
			if (i+j<mul) printf("%6d ",array[i+j]);
			else if (i) printf("       ");
		printf("'");
		for (j=0; j<4; j++) {
			if (i+j>=mul) break;
			interprete(array[i+j]&255);
			interprete(array[i+j]>>8);
			}
		printf("'\n");
		}
}

printpa(array, mul) int array[]; int mul; {
	int i, j, m, n;

	for (i=0; i<mul; i+=4) {
		if (cancelhit()) break;
		printf("%04x [%3d] ",&array[i],i);
		for (j=0; j<4; j++)
			if (i+j<mul) 
				if (array[i+j]==NIL)
					printf(" NIL ");
				else
					printf("%04x ", array[i+j]);
			else if (i) printf("     ");
		printf("'");
		for (j=0; j<4; j++) {
			if (i+j>=mul) break;
			interprete(array[i+j]&255);
			interprete(array[i+j]>>8);
			}
		printf("'\n");
		}
}

printvar (s, sbase) struct stentry *s; unsigned sbase; {

	struct sttype vartype;
	char pa[9], *addr;

	str70cpy(pa,s->stname);
	printf("%-11s",pa);
	addr = do_stentry(s,vartype);
	printexp(sbase+addr,vartype,0,0);
}

printstruct (addr, type, mul) char *addr; struct sttype *type; int mul; {

	/* print "mul" occurrences of structures described by "type"
	 * starting at address "addr".
	 * If possible, we go through the symbol table and print each element
	 * of the structure in the proper format.
	 */

	struct stentry *s;
	unsigned i, size;

	size = sizeelt(type);

	for (i=0; i<mul; i++, addr += size) {
		if (cancelhit() || cancel_char) break;
		if (mul>1) printf("[%u]",i);
		putchar('\n');
		if (type->tsptr) {
			s = type->tsize.p;
			while( (--s)->stname[7]!=0xff && STELT(*s) )
				if (WHAT(*s)==VARIABLE)
					printvar(s, addr);
			}
		else
			printca(addr,type->tsize.u);
		}
}

printlg(s) struct stentry *s; {

	for (; s->stname[7]!=0xff; s--) {
		cancel_char = 0;
		if (WHAT(*s)==VARIABLE && !STELT(*s) )
			printvar(s, 0);
		cancelhit();
		if (cancel_char && cancel_char!=NEXTKEY) break;
		}
}

printargs() {
	struct stentry *s;

	s = curfnt->fntst;
	for (; s->stname[7]!=0xff && FORML(*s) && !cancelhit(); s--)
		if (!STELT(*s))
			printvar(s, 0);
}

printtype(type) struct sttype *type; {
	char pa[9], i;

	if (type->tptfnf)
		printf("pointer to a function returning ");

	for (i=0; i<type->tlind; i++)
		putchar('*');

	switch (type->ttype) {
	    case INT: 	   printf("int"); break;
	    case CHAR:	   printf("char"); break;
	    case UNSIGNED: printf("unsigned"); break;
	    case STRUCT: 
		if (type->tsptr) {
			str70cpy(pa,type->tsize.p->stname);
			printf("struct %s",pa);
			}
		else
			printf("(%d byte structure)",type->tsize.u);
		break;
	    }
}

printexp(addr,type,mul,option)
char *addr; struct sttype *type; int mul; char option; {

	/* Print "mul" occurences of an item with address "addr" described
	 * by "type".  If option == 0, printexp decides the proper print
	 * format based on *type; for non-zero options, the specified option
	 * is used (p for pointer, c for char, w for word, s for string). 
	 * Also, if mul is specified as zero, *type is used to provide the
	 * correct number of elements to print.
	 */

	unsigned i, nrows, ncols, scale;

	if (mul==0 && option==0 && type->tdimsz && !type->tforml)
		if (type->tdimsz == 0xff00) {
			printf("a %d element array of ",type->tmul);
			printtype(type);
			putchar('\n');
			printsexp(addr,type,mul,option);
			}
		else {
			ncols = type->tdimsz;
			nrows = type->tmul/ncols;
			printf("a %d x %d array of ",nrows, ncols);
			printtype(type);
			putchar('\n');
			scale = type->tdimsz*sizeelt(type);
			for (i=0; i<nrows; i++) {
				if (cancelhit() || cancel_char) break;
				printf("\nrow [ %u ]\n",i);
				printsexp(addr+i*scale,type,ncols,option);
				}
			}
	else {
		if (option==0 && type->ttype==STRUCT && !type->tlind) {
			printf("a ");
			printtype(type);
			}
		printsexp(addr,type,mul,option);
		}
}

printsexp(addr,type,mul,option)
char *addr; struct sttype *type; int mul; char option; {

	if (type->ttype == BAD)
		return;

	if (option == 0) 
		if (type->tlind || type->tforml)
			option = 'p';
		else if (type->ttype==CHAR)
			option = 'c';
		else if (type->ttype==STRUCT)
			option = 't';
		else
			option = 'w';

	if (mul == 0)
		mul = type->tmul;

	switch (tolower(option)) {
	    case 'p':
		if (mul>1)
			printpa(addr,mul);
		else
			printp(addr);
		break;

	    case 'c':
		if (mul>1)
			printca(addr,mul);
		else
			printc(addr);
		break;

	    case 's':
		prints(addr);
		break;

	    case 't':
		printstruct(addr,type,mul);
		break;

	    case 'i': case 'w': default: 
		if (mul>1)
			printwa(addr,mul);
		else
			printw(addr);
		break;
	    }
}

print(l) char *l; {
	char *addr, aonly, option, token[10], *ll, c;
	int mul, value;
	struct sttype type;

	ll = l;
	if (!(aonly = (c = get_token(&l, token, &value)) == '&'))
		if (c==BAD)
			return;
		else
			l = ll;

	addr = exp(&l, 0, type);

	if (type.ttype == BAD)
		return;

	if (aonly) {
		printf(" %04x\n", addr);
		return;
		}

	mul = 0;
	option = 0;

	while (c = get_token(&l, token, &value))
		if (c==ID)
			option = *token;
		else if (c==INTEGER)
			mul = value;

	printexp(addr,type,mul,option);
}

set(l) char *l; {

	/* set processes the set command, which sets a byte or a word
	 * in memory to the value specified by the user.  l is a 
	 * pointer to the input command line, which contains
	 * an expression (which determines the address of the item to
	 * be set) and a value (either an integer or a single quoted
	 * character).  An optional c operand flag is allowed, which 
	 * specifies that only a single byte is to be stored; if
	 * this option is not specified, set() determines the length of
	 * the target item by determining its type.
	 */

	char token[10], c;
	int i, value, *addr;
	struct sttype type;

	addr = exp(&l, 0, type);
	if (type.ttype==BAD)
		return;

	c = get_token(&l, token, &i);

	if (c=='\'') {
		c = get_token(&l, token, &i);
		value = *token;
		get_token(&l, token, &i);
		}
	else if (c==INTEGER)
		value = i;
	else {
		printf("invalid new value");
		return;
		}

	if ( (type.tlind == 0 && type.ttype == CHAR && 
	      !type.tforml && !type.tptfnf) ||
	     (get_token(&l, token, &i) && tolower(*token) == 'c') )
		poke(addr, value);
	else
		*addr = value;
}

#include "cdb1.h"

/* Copyright (c) 1982-1986 by J. David Kirkland, Jr. */

openit (fn) char *fn; { 
	unsigned num, i, max;

	inbufp = CDB2ADDR;

	if (*fn) {
		if (ERROR==(fd=open(fn,0))) {
			printf("can't open %s\n",fn);
			return ERROR;
			}
		if (1!=read(fd,inbufp,1)) {
			printf("read error on %s\n",fn);
			return ERROR;
			}
		}

	num = *inbufp++;
	num += *inbufp++<<8;

	i = (num-2+127)/128 - 1;
	if (num > (max=topofmem()-CDB2ADDR-1000) ) {
		printf("%s is too large (%d bytes); max is %d bytes\n",
			fn, num, max);
		return ERROR;
		}

	if (*fn && i && i!=read(fd,inbufp+126,i)) {
		printf("read error on %s\n",fn);
		return ERROR;
		}

	return num;
}

struct fstentry *getst(s) char *s; {
	char i, name[8];

	for (i=8; i<16; i++)
		s[i] = *inbufp++;
	for (i=0; i<8; i++)
		s[i] = *inbufp++;

	return inbufp - 16;
}

str70cpy(d,s) char *d, *s; {

	while (0 == ((*d = *s++) &0x80))
		d++;
	*d = 0x7f & *d;
	*++d = 0;
}

str7cmp (s1, s2) char *s1, *s2; {

   	/* compare two strings, either bit-7-terminated or null-terminated */

	char end1, end2;

	for (; *s1 == *s2; s1++, s2++)
		if ( (0x80 & *s1) || !*s1) return 0;

	if ((0x7F & *s1) < (0x7F & *s2)) return -1;
	if ((0x7F & *s1) > (0x7F & *s2)) return  1;

	end1 = (*s1 & 0x80) || !*(s1+1);
	end2 = (*s2 & 0x80) || !*(s2+1);
	if (end2  &&  !end1) return 1;
	if (end1  &&  !end2) return -1;
	/* if (end1  &&  end2) */ return 0;
}

struct fntentry *name2fnt(name) char *name; {
	struct fntentry *f;
	char n[9];

	movmem(name,n,8);
	n[8] = 0;
	ucase(n);

	for (f = fntab; f->fntaddr != 0xffff; f--)
		if (!strcmp(n,f->fntname))
			return f;

	return ERROR;
}

symb (s,e) char *s; struct stentry *e; {
	struct stentry *p;

	/* * 
	printf("%02x %02x %02x %02x %02x %02x %02x %02x\n",
		s[0], s[1], s[2],
		s[3], s[4], s[5],
		s[6], s[7]);
	 * */
	for (p = globalp; p>=e; p--) {
		/* * 
		printf("%02x %02x %02x %02x %02x %02x %02x %02x ",
			p->stname[0], p->stname[1], p->stname[2],
			p->stname[3], p->stname[4], p->stname[5],
			p->stname[6], p->stname[7]);
		printf("%s\n",p->stname);
		 *  */
		if (0==str7cmp(s,p->stname))
			return p;
		}

	return ERROR;
}

addst(p,s,f) struct stentry *p, *s; struct fstentry *f; {
	struct fstentry *q;

	/* * 
	char name[10]; 
	str70cpy(name, s->stname);
	printf("%-8s %4d %5d %5d %4d %4d %4d %04x %04x %04x\n",name,
		TYPE(*s), STELT(*s), FORML(*s), WHAT(*s),
		LIND(*s), CLEV(*s),
		s->stadrs, s->stsize, s->stdimsz);
	 * */

	s->stb2 = (LIND(*s)<<6) + ((CLEV(*s)!=0)<<1);
	movmem (s, --p, 16);

	if (WHAT(*s)==STRUCTDEF) {
		f->fstn1 = 0xff;
		f->fstp  = p;
		}

	return p;
}

deffn(p,s,size) char *p; struct stentry *s; unsigned size; {
	char name[10]; 
	struct fntentry *fnt;
	unsigned *u;

	*--p = 0xff;
	u = p -= 2;
	*u = size;

	str70cpy(name, s->stname);
	/* * 
	printf("function %s, frame size %d\n",name,s->stadrs);
	 * */
	if (ERROR!=(fnt = name2fnt(name))) {
		fnt->fntst = p - sizeof *s;
		fnt->fntfsize = s->stadrs;
		}

	return p;
}

char *build1glob(p,num) struct stentry *p; unsigned num; {
	struct stentry s, *b;
	struct fstentry *f;
	unsigned i;
	/* 
	char name[10]; 
	   */

	num /= 16;

	b = p;
	for (i=0; i<num; i++) {
		f = getst(s);
		if (CLEV(s) == 0 && (WHAT(s)==VARIABLE || WHAT(s)==STRUCTDEF) )
			p = addst(p,s,f);
		}

	for (--b; b>=p; --b)
		if (WHAT(*b)==VARIABLE && TYPE(*b)==STRUCT) {
			f = CDB2ADDR+2;
			b->stsize = (f + b->stsize)->fstp;
			b->stb2 |= 1;
		/* * 
	str70cpy(name, b->stname);
	printf("%-8s %4d %5d %5d %4d %4d %4d %04x %04x %04x ** new **\n",name,
		TYPE(*b), STELT(*b), FORML(*b), WHAT(*b), LIND(*b), CLEV(*b),
		b->stadrs, b->stsize, b->stdimsz);
		 * */
			}
	return p;
}

fakes(s) struct stentry *s; {

	s->stname[0] = 0;
	s->stb1 = 0x01;
	s->stb2 = 0x3f;
}

char *build1loc(p,num) struct stentry *p; unsigned num; {
	struct stentry s, *b, *q, *t;
	struct fstentry *f;
	char stpflag;
	unsigned i, size;
	/* * 
	char name[10]; 
	 * */

	num /= 16;

	b = p;
	for (i=0; i<=num; i++) {
		if (i==num)
			fakes(s);
		else
			f = getst(s);
		if (WHAT(s) == FUNCDEF) {
			for (q = --b; q>=p; --q)
			     if (WHAT(*q)==VARIABLE && TYPE(*q)==STRUCT) {
				  f = CDB2ADDR+2;
				  stpflag = 1;
				  if ((f + q->stsize)->fstn1 == 0xff)
				       q->stsize = (f + q->stsize)->fstp;
				  else if (ERROR!=(t=
					   symb(&(f + q->stsize)->fstn1,p)))
				       q->stsize = t;
				  else {
				       q->stsize = (f + q->stsize)->fstsize;
				       stpflag = 0;
				       }
				  q->stb2 |= stpflag;
		/* * 
	str70cpy(name, q->stname);
	printf("%-8s %4d %5d %5d %4d %4d %4d %04x %04x %04x ** new **\n",name,
		TYPE(*q), STELT(*q), FORML(*q), WHAT(*q), LIND(*q), CLEV(*q),
		q->stadrs, q->stsize, q->stdimsz);
		 * */
				  }
			p = deffn(p,s,size);
			size = s.stadrs;
			b = p;
			}
		else if (CLEV(s) && (WHAT(s) == VARIABLE || WHAT(s)==STRUCTDEF)) 
			p = addst(p,s,f);
		}

	return p;
}

char *buildglob(p) struct stentry *p; {
	int i, num;

	globalp = p-1;

	for (i=0; i<globals.nfiles || (i==0 && globals.nfiles==0); i++) {
		if (ERROR==(num=openit(globals.files[i])))
			return ERROR;
		p = build1glob(p, num);
		close(fd);
		}

	return p;
}

char *buildloc(p) struct stentry *p; {
	int i, num;
	char *q;

	localp = p-1;

	for (i=0; i<locals.nfiles || (i==0 && locals.nfiles==0); i++) {
		if (globals.nfiles==0 && locals.nfiles==0)
		        locals.files[0][0] = 0;
		if (ERROR==(num=openit(locals.files[i])))
			return ERROR;
		p = build1loc(p, num);
		close(fd);
		}

	return p;
}



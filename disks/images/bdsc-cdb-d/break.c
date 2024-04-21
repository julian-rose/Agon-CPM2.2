#include "cdb2.h"

/* Copyright (c) 1982-1986 by J. David Kirkland, Jr. */

insertbreak(fnt,sn,count) struct fntentry *fnt; unsigned sn, count; {

	/* insert a breakpoint at the function referenced by fnt and the
	 * statment number "sn".  Set the bcount of the breakpoint to count.
	 * N.B.:  this routine must update curbreak and all the breakindex
	 * bytes in the fn table.
	 * Return 0 or ERROR.
	 */

	struct breakentry *q;
	struct fntentry *f;
	int i;

	if (nbreaks>=MAXBREAKS) {
		printf("too many breakpoints (max is %d)\n",MAXBREAKS);
		return ERROR;
		}

	if (fnt->fntbreakindex!=NOBREAK)
		i = fnt->fntbreakindex;
	else
		i = 0;

	for ( q = breaktab[i]; i<nbreaks; q++, i++)
		if (fnt < q->bfnt || (fnt==q->bfnt && sn <= q->bsn))
			break;

	if (fnt!=q->bfnt || sn != q->bsn)
		movmem( q, q+1, (sizeof *q) * (nbreaks++ - i) );

	q->bfnt = fnt;
	q->bsn = sn;
	q->bcount = count;

	for (f = fntab; f->fntaddr != 0xffff; f--)
		if (f->fntbreakindex!=NOBREAK && f->fntbreakindex >= i)
			f->fntbreakindex++;

	if (fnt->fntbreakindex==NOBREAK || fnt->fntbreakindex>i)
		fnt->fntbreakindex = i;

	breaktab[nbreaks].bfnt = 0xffff;

	curbreak = curfnt->fntbreakindex;
	return 0;
}

deletebreak(fnt,sn) struct fntentry *fnt; unsigned sn; {

	/* delete the breakpoint at the function referenced by fnt and the
	 * statment number "sn", if it exists.
	 * N.B.:  this routine must update curbreak and all the breakindex
	 * bytes in the fn table.
	 * Return 0 or ERROR.
	 */

	struct breakentry *q;
	struct fntentry *f;
	int i;

	if (fnt->fntbreakindex!=NOBREAK)
		i = fnt->fntbreakindex;
	else
		i = 0;

	for ( q = breaktab[i]; i<nbreaks; q++, i++)
	        if (fnt < q->bfnt || (fnt==q->bfnt && sn <= q->bsn))
			break;

	if (fnt!=q->bfnt || sn!=q->bsn) {
		printf("%s %d is not a breakpoint\n", fnt->fntname, sn);
		return ERROR;
		}

	movmem( q+1, q, (sizeof *q) * (nbreaks-- - i) );

	if (fnt->fntbreakindex==i && q->bfnt!=fnt)
		fnt->fntbreakindex = NOBREAK;

	for (f = fntab; f->fntaddr != 0xffff; f--)
		if (f->fntbreakindex!=NOBREAK && f->fntbreakindex > i)
			f->fntbreakindex--;

	breaktab[nbreaks].bfnt = 0xffff;

	curbreak = curfnt->fntbreakindex;
	return 0;
}

zapbreaks () {

	/* eliminate all breakpoints.
	 */

	struct fntentry *f;

 	nbreaks = 0;
	breaktab[0].bfnt = 0xffff;

	for (f = fntab; f->fntaddr != 0xffff; f--)
		f->fntbreakindex = NOBREAK;
}

clearbreaks () {

	/* eliminate all breakpoints except MAIN -1
	 */

	struct fntentry *f;
	zapbreaks();

	if (ERROR != (f = name2fnt("MAIN")))
		insertbreak(f,-1,1);
}



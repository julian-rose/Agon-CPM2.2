#include "cdb2.h"

/* Copyright (c) 1982-1986 by J. David Kirkland, Jr. */

isbreak() {
	/* isbreak() determines whether cursn and curfnt are a breakpoint.
	 * The CALLER must check that curbreak != 0xff
	 */

	/* struct breakentry *isb_q;	    */

	isb_q = breaktab[curbreak];

	/* first, try to find an entry in breaktab */
	while (curfnt==isb_q->bfnt && cursn > isb_q->bsn)
	      	 isb_q++;

	/* did we find one? if so, decrement bcount and check for zero */
	if (curfnt!=isb_q->bfnt || cursn!=isb_q->bsn || --(isb_q->bcount) )
		return 0;
	else {
		isb_q->bcount = 1;
		return 1;
		}
}

atbreak (sp) struct savearea *sp; {

	/* the assembly package calls atbreak whenever a RST is encountered.
	 * A pointer to the savearea is passed 
	 */

	unsigned sn, *resume, atret();

	if (0==(sn = *(resume = sp->statement++))) {
		/* sn == 0 means entry to a new function; stack on tracetab */
		if (ntraces>=MAXTRACES) {
			if (!trace_full) {
				printf(
				  "fn trace table full, fn tracing inactive\n");
				trace_full = 1;
				}
			}
		else {
			tracetab[ntraces].taddr = sp->caller_return;
			tracetab[ntraces++].tsn = cursn;
			sp->caller_return = atret;
			}
	   	curbreak = (curfnt = addr2fnt(resume))->fntbreakindex;
		curargs = (sp+1);
		}

	cursn = sn;

	if (trace_display && walkcount!=1) {
		printf("  trace:  %s ",curfnt->fntname);
		printasn(sn);
		printf(" [%04x]\n",sp->statement);
		}       
	if ((walkcount && !--walkcount) || (kbhit() && kbint) ||
	    (curbreak!=NOBREAK && isbreak()) ) {
		/* at a breakpoint */
		walkcount = 0;
		trace_display = 0;
		printf("\nbreak at %s ",curfnt->fntname);
		printasn(sn);
		printf(" [%04x]\n",sp->statement);
		cursave = sp;
		command();
		}

	resumeit(sp);
}

atreturn (h) int h; {

  	/* when a function returns, it returns here because atbreak() changed
	 * the stacked return address.  atreturn() checks for a breakpoint
	 * at sn == -1 (i.e., function return) and acts accordingly.
	 */

	char *t, stop;

	if (trace_full) {
		printf("fn tracing restored\n");
		trace_full = 0;
		}
	cursn = -1;

	if ( (stop =  (curbreak!=NOBREAK && isbreak()) || 
		(walkcount && !--walkcount)  || (kbhit() && kbint) ) 
		|| trace_display ) {
		printf(stop ? "\n" : "  trace:  ");
		printf("%s returning %04x = %d = '",curfnt->fntname,h,h);
		interprete(h>>8);
		interprete(h&255);
		printf("'\n");
		if (stop)
			command();
		}
	t = tracetab[--ntraces].taddr;
	curbreak = (curfnt = addr2fnt(t))->fntbreakindex;
	curargs = NIL;
	return t;
}

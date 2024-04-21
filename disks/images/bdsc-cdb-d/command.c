#include "cdb2.h"							        

/* Copyright (c) 1982-1986 by J. David Kirkland, Jr. */

help() {

	printf("debugger commands are:\n");
	printf("b[reak] [fn] [sn [count]]\n");
	printf("clear\t\t\tall breakpoints\n");
	printf("d[ump]\t\t\tcontents of memory\n");
	printf("g[o]\n");
	printf("l[ist]\n");
	printf("l[ist] a[rgs]\n");
	printf("l[ist] b[reaks]\n");
	printf("l[ist] g[lobals]\n");
	printf("l[ist] l[ocals]\n");
	printf("l[ist] m[ap]\n");
	printf("l[ist] t[raceback]\n");
	printf("quit\t\t\texit to CP/M\n");
	printf("r[eset] [fn] [sn]\ta breakpoint\n");
	printf("run\t\t\tstart execution, debugger off\n");
	printf("s[et] var val [c]\ta variable\n");
	printf("t[race] [num]\t\texecute num statements, displaying sn's\n");
	printf("u[ntrace] [num]\t\texecute num statements, silently\n");
}

printasn(n) unsigned n; {

	/* print a statement number.  If 4 high order bits are on, then
	 * use decimal format (low order 12 bits . high order 4).
	 */

	if (n==0xffff)
		printf("-1");
	else {
		printf("%d",n&0x0fff);
		if (n&0xf000)
			printf(".%d",n>>12);
		}
}

listbreaks () {
	int i;

	for (i=0; i<nbreaks; i++) {
		printf("%s ",breaktab[i].bfnt->fntname);
		printasn(breaktab[i].bsn);
	        if (breaktab[i].bcount!=1)
			printf("   (%d)\n",breaktab[i].bcount);
		else
			putchar('\n');
		}
}

listit(l) char *l; {
	/* process a list command */

	char subcmd[10], *garbage;

	get_token(&l, subcmd, garbage);

	switch (tolower(*subcmd)) {
	    case 0:
		printf("current location: %s ",curfnt->fntname);
		printasn(cursn);
		putchar('\n');
		break;
	    case 'a':
		if (curfnt->fntst!=NIL)
			printargs();
		else if (curargs == NIL)
			printf("can't list args to %s because it has called a function\n",
			       curfnt->fntname);
		else {
			printf("first argument address is %04x\n",curargs);
			printf("[1] = %04x = %d, [2] = %04x = %d, [3] = %04x = %d\n",
			        *curargs, *curargs, *(curargs+1), *(curargs+1),
			        *(curargs+2), *(curargs+2) );
			printf("[4] = %04x = %d, [5] = %04x = %d, [6] = %04x = %d\n",
			        *(curargs+3), *(curargs+3), *(curargs+4),
			        *(curargs+4), *(curargs+5), *(curargs+5) );
			}
		break;
	    case 'g':
		printlg(globalp);
	        break;
	    case 'l':
		if (curfnt->fntst!=NIL)
			printlg(curfnt->fntst);
		else
 			printf("local symbols not available\n");
	        break;
	    case 'm':
		map();
		break;
	    case 't':
		trace();
	        break;
	    case 'b':
		listbreaks();
		break;
	    default:
		printf("illegal subcommand\n");
	    }
}

trace () {
	int i;

	for (i=1; i<ntraces; i++)
		if (tracetab[i].tsn!=0xffff) {
			printf("%s ",addr2fnt(tracetab[i].taddr)->fntname);
			printasn(tracetab[i].tsn);
			printf(" called\n");
			}
		else
			printf("%s called\n",
				addr2fnt(tracetab[i].taddr)->fntname);

	printf("%s\n",curfnt->fntname );
}

compare(s, pat) char *s, *pat; {

	while (*pat && tolower(*s++) == tolower(*pat)) 
		pat++;

	return !*pat;
}

breakargs(l, fnt, sn) char **l; struct fntentry **fnt; unsigned *sn; {

	/* Process arguments to break and reset commands.  Determines
	 * function (and sets fnt) and statement number (sets sn).
	 * l is pointer to pointer to input line 
	 *
	 * syntax: 
	 * 	[function name] [line[.statement within line]]
	 * if function name is ommitted, current function assumed.
	 * if line ommitted, zero (i.e., function entry) assumed.
	 */

	char name[10], class;
	int high;

	if (0 == (class = get_token(l, name, sn))) {
		printf("arguments missing\n");
		return ERROR;
		}
	else if (class==INTEGER)
		*fnt = curfnt;
	else {
		if (ERROR == (*fnt = name2fnt(name))) {
			printf("%s does not exist\n",name);
			return ERROR;
			}
		if (INTEGER != get_token(l, name, sn))
			*sn = 0;
		}

	if ('.' == **l) {
		(*l)++;
		if (INTEGER != get_token(l, name, &high)) {
			printf("invalid statement number %d.%s\n",*sn,name);
			return ERROR;
			}
		*sn |= high<<12;
		}
}

setbreak(l) char *l; {
	unsigned sn, count;
	struct fntentry *fnt;
	char name[20];

	if (ERROR!=breakargs(&l,&fnt,&sn)) {
		if (INTEGER != get_token(&l, name, &count))
			count = 1;
		insertbreak(fnt,sn,count);
		}
}

resetbreak(l) char *l; {
	unsigned sn;
	struct fntentry *fnt;

	if (ERROR!=breakargs(&l,&fnt,&sn))
		deletebreak(fnt,sn);
}

map() {
	/* print a load module map of the target program.
	 */

	struct fntentry *q;
	int i;

	for (i = 0, q = fntab-1; (q-1) > fntabend; q--, i++) {
		if (!(i&3)) putchar('\n');
		printf("%-8s  %04x     ", q->fntname, q->fntaddr);
		}
	putchar('\n');
}

command() {

	/* get and process commands.
	 */

	char *l, line[128], cmd[10];
	int garbage;

	walkcount = trace_display = 0;
	while (1) {
		cancel_char = 0;
		putchar('>');
		l = gets(line);
		get_token(&l, cmd, &garbage);
		switch (tolower(*cmd)) {
		    case 'b':
		        setbreak(l);
			break;
		    case 'c':
			if (compare(cmd,"clear"))
				clearbreaks();
			else 
				goto wrong;
			break;
		    case 'l':
			listit(l);
			break;
		    case 'q':
			if (compare(cmd,"quit"))
				exit();
			else 
				goto wrong;
			break;
		    case 'r':
			if (!compare(cmd,"run")) {
			        resetbreak(l);
				break;
				}
			movmem("\341\043\043\351",8*RST_NUM,4);
			/*    POP H INX H x2  PCHL  */
			zapbreaks();	      /* fall through */
		    case 'g':
		        return;
			break;
		    case 't':
			trace_display = 1;    /* fall through */
		    case 'w': case 'u':
		        if (INTEGER!=get_token(&l, cmd, &walkcount))
			        walkcount = 1;
			return;
			break;
		    case 's':
		        set(l);
			break;
		    case '?': case 'h':
		        help();
			break;
		    case ',':  case 'd': case 'p':
		        print(l);
			break;
#ifdef DEBUG
		    case 'z':
		        debug = !debug;
			printf("debug %s\n", debug ? "on" : "off");
			break;
#endif
	wrong:	    default:
			printf("illegal command\n");
		    }
		}    

}


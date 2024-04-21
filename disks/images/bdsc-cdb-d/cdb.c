#include "cdb1.h"

/* Copyright (c) 1982-1986 by J. David Kirkland, Jr. */

ucase (s) char *s; {

	while (*s = toupper(*s)) 
		s++;
}

makeext (fname, ext) char *fname, *ext;	 {
	/* uppercase filename and force extension to ext */

	while (*fname && (*fname != '.')) {
		*fname = toupper (*fname);
		++fname;
		}
	*fname++ = '.';
	strcpy (fname, ext);
}

buildfnt (addr, name) struct fntentry *addr; char *name; {
	FILE *sym;
	char afntname[4][20];
	char *afntaddr[4];
	struct fntentry *p;
	int i, n;

	strcpy(afntname, name);
	makeext(afntname,"SYM");
	if (ERROR==(sym=fopen(afntname,"r"))) {
		printf("Can't open %s\n",afntname);
		return ERROR;
		}
	p = addr;
	(--p)->fntaddr = 0x0000;
	fntab = p;
	strcpy(p->fntname, "C.CCC");
	while (1) {
		n = fscanf(sym,"%x %s\t%x %s\t%x %s\t%x %s\r", 
			   &afntaddr[0], afntname[0],
			   &afntaddr[1], afntname[1], &afntaddr[2], afntname[2], 
			   &afntaddr[3], afntname[3]);
		n /= 2;
		for (i=0; i<n; i++) {
			(--p)->fntaddr = afntaddr[i];
			strcpy(p->fntname, afntname[i]);
			p->fntbreakindex = NOBREAK;
			p->fntst = NIL;
			p->fntfsize = -1;
			}
		if (n<4) break;
		}
	(--p)->fntaddr = 0xffff;
	strcpy(p->fntname, "NEVER");

	return p;
}

doargs(i, ac, av, arg) int i, ac; char **av; struct argb *arg; {
	char line[80];

	strcpy(av[i], &av[i][2]);

	for (arg->nfiles = 0; i < ac; i++)
		if (*av[i]=='-' || *av[i]=='%') {
			i--;
			break;
			}
		else if (0==strcmp(av[i],"0")) {
			arg->nfiles = -1;
			break;
			}
		else if (*av[i]) {
			strcpy (arg->files[arg->nfiles], av[i]);
			makeext(arg->files[arg->nfiles++], CSYMEXT);
			}

	if (arg->nfiles==0) {
		printf("Enter %s symbol files, one per line (null line to end)\n*",
			(arg==locals) ? "local" : "global" );
		for (; arg->nfiles<NFILES && getline(line,70); ) {
			strcpy (arg->files[arg->nfiles], line);
			makeext(arg->files[arg->nfiles++], CSYMEXT);
			putchar('*');
			}
		if (arg->nfiles==NFILES)
			printf("Error:  too many files\n");
		else if (arg->nfiles==0)
			arg->nfiles = -1;
		}

	return i;
}

validdrive (d) char *d; {

	if (isdigit(*d))
		if ((isdigit(*++d) && *++d=='/') || *d=='/')
			d++;
		else 
			return 0;

	return !*d || (isalpha(*d) && (!*++d || *d==':'));
}

processargs(argc, argv, drive, kbint) 
int argc; char **argv, **drive, *kbint; {

	int i;

	locals.nfiles = globals.nfiles = 0;
	strcpy (globals.files[0], argv[1]);
	makeext(globals.files[0], CSYMEXT);
	strcpy (locals.files[0], globals.files[0]);

	*drive = CDB2_DRIVE;
	*kbint = KBINT_DEF;

	for (i=2; i<argc; i++)
		if (argv[i][0]=='%')
			break;
		else if (argv[i][0]=='-')
			if (argv[i][1]=='L')
				i = doargs(i, argc, argv, locals);
			else if (argv[i][1]=='G')
				i = doargs(i, argc, argv, globals);
			else if (argv[i][1]=='D') {
				if (!*(*drive = &argv[i][2]))
					if (++i>=argc)
						printf("-d operand omitted\n");
					else
						*drive = argv[i];
				if (**drive && !validdrive(*drive)){
					printf("Invalid -d option %s\n",*drive);
					*drive = CDB2_DRIVE;
					}
				}
			else if (argv[i][1]=='K')
				*kbint = !KBINT_DEF;
			else
				printf("invalid argument %s\n",argv[i]);
		else
			printf("invalid argument %s\n",argv[i]);

	if (**drive && isalpha( (*drive)[strlen(*drive)-1]) )
		strcat(*drive,":");
}

cpmtinker(stack) char *stack; {
	int i, *a;
	char *p;

	movmem(ORIGIN + 0x0005, stack-=3, 3); /* fix BDOS jumps so 0x0006 has */
	a = ORIGIN + 0x0006;		      /* a good stack pointer 	      */
	*a = stack;

	p = ORIGIN + 0x0080;		/* patch the arguments to the target */
	i = *p++;
	while (--i && *++p!='%') ;	/* find 'em */

	if (i>1)
		movmem(p+1,ORIGIN + 0x0082,i);
	else if (i==0)
		poke(ORIGIN + 0x0081,0);
	else  {
		printf("Enter command line args: ");
		i = 1 + getline(ORIGIN + 0x0082,126);
		ucase(ORIGIN + 0x0082);
		}

	poke(ORIGIN + 0x0080,i);

	return stack;
}

statusmsg(p, glob, loc) char *p, *glob, *loc; {

	printf("top of target stack is %04x, cdb2 is at %04x\n",p,CDB2ADDR);
	printf("globals use %04x bytes, locals use %04x bytes\n",
		glob-loc, loc-p);
}

main (argc, argv) int argc; char **argv; {
	int i;
	char (*f)(), *p, *drive, name[20], ovlname[20], kbint;

	COPYRIGHT;

	printf("c debugger ver 1.60\n");

	if (argc<2) {
		printf("usage: cdb target_name [-g [global_fns | 0]]");
		printf(" [-l [local_fns | 0] ]\n");
		printf("	   [-d [usr/][drive] ] [-k]");
		printf(" [ %% [target arguments] ]\n");
		exit();
		}

	strcpy(name,argv[1]);

	if (ERROR==(p = buildfnt(CDB2ADDR, name)))
		exit();

	processargs(argc, argv, &drive, &kbint);

	if (ERROR==(p = buildglob(p)))
		exit();
	if (ERROR==(p = buildloc(p)))
		exit();

	p = cpmtinker(p);

	statusmsg(p, globalp+1, localp+1);

	strcpy(ovlname,drive);
	strcat(ovlname,CDB2_FILE);

	if (ERROR==swapin(ovlname,CDB2ADDR)) {
		printf("Couldn't swapin %s\n",ovlname);
		exit();
		}

	makeext(name,"COM");
	f = CDB2ADDR;
	(*f)(name,globalp,localp,p,kbint);
}


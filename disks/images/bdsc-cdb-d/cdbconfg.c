/*
	CDBCONFIG.C
	cdb v1.6 Automated Configuration Program
	Written 8/86 by David Kirkland. 

	Usage:
		cdbconfg [bdos]

	cdbconfg creates a submit file called MAKECDB.SUB in the current
	user number on the currently logged disk that, when submitted,
	will compile and link CDB.COM and CDB2.OVL.  cdbconfg also
	edits the CDB.H file to set the #define for CDB2ADDR
	to the appropriate address for the target system.

	If bdos is given (as a hex number with no leading 0x), 
	CDB is configured for a system with a BDOS beginning at the specified
	address.  If no parameter is given, cdbconfg uses the current
	BDOS location of the system on which cdbconfg is being run.

	Compile & Link:
		cc cdbconfg
		clink cdbconfg

	For more details, see the chapter on CDB in the BDS C User's Guide.
*/

#include <stdio.h>

#define CDB2CODE 0x4900		/* approx size of cdb2 code */
#define EXTERNS  0x0e00		/* approx size of externals plus stack */

#define MAX	 150		/* max line length in cdb.h file */

void error (format, a1, a2, a3, a4) char *format; int a1, a2, a3, a4; {
	fprintf(stderr, format, a1, a2, a3, a4);
	exit(1);
}

int strncmp(s1, s2, n)
char *s1, *s2; int n;
{
	while (*s1 == *s2++)
		if (--n == 0 || *s1++ == '\0')
			return 0;
	return (*s1 - *--s2);
}

main(argc, argv) int argc; char **argv;
{
	char **ptr;
	char *cdb2addr, *cdbexterns, *bdos;
	FILE *in, *out;
	char line[MAX+1], *p;
	char done;

	/* find BDOS location and compute cdb2 location and location of 
	   externals */

	if (argc == 2)
		sscanf( argv[1], "%4x", &bdos);
	else {
		ptr = 0x0006;
		bdos = *ptr;
		}
	cdbexterns = bdos - (EXTERNS + 0x0006);
	cdb2addr = cdbexterns - CDB2CODE;

	printf("BDOS is at %04x, cdb2addr is %04x and cdb externals are at %04x\n",
		bdos, cdb2addr, cdbexterns);

	/* create makecdb.sub as a submit file with necessary commands to 
	   compile and link cdb.com and cdb2.ovl */

	if (NULL == (out = fopen("makecdb.sub", "w")))
		error("can't open makecdb.sub\n");

	fprintf(out, "; BDOS is at %04x, cdb2addr is %04x and cdb externals are at %04x\n",
		     bdos, cdb2addr, cdbexterns);
	fprintf(out,"; compile and link cdb.com\n");
	fprintf(out,"cc cdb.c -e3c00 -x\n");
	fprintf(out,"cc build.c -e3c00 -x\n");
	fprintf(out,"l2 cdb build\n");
	fprintf(out,"; compile files for cdb2\n");
	fprintf(out,"cc cdb2.c -e%04x -x\n", cdbexterns);
	fprintf(out,"cc atbreak.c -e%04x -x\n", cdbexterns);
	fprintf(out,"cc break.c -e%04x -x\n", cdbexterns);
	fprintf(out,"cc command.c -e%04x -x\n", cdbexterns);
	fprintf(out,"cc print.c -e%04x -x\n", cdbexterns);
	fprintf(out,"cc parse.c -e%04x -x\n", cdbexterns);
	fprintf(out,"cc util.c -e%04x -x\n", cdbexterns);
	fprintf(out,"; link cdb2.ovl\n");
	fprintf(out,"l2 cdb2 dasm atbreak command break print parse util -ovl null %04x -wa\n", cdb2addr);

	if (ERROR == fclose(out))
		error("can't close makecdb.sub\n");

	/* now edit cdb.h to change the #define for CDB2ADDR to the correct
	   value.  Do the edit by copying from the existing cdb.h to a new
	   file cdb.$$$.  Once the copy is successful, we rename cdb.h to 
	   cdb.bak and rename the new cdb.$$$ to cdb.h */

	if (NULL == (in = fopen("cdb.h", "r"))) 
		error("can't open cdb.h\n");

	if (NULL == (out = fopen("cdb.$$$", "w"))) 
		error("can't open cdb.$$$ as output file\n");

	done = 0;
	while (fgets(line, MAX, in)) {
		if (!done && !strncmp(line, "#define", 7)) {
			p = &line[7];
			while(isspace(*p)) p++; /* skip white space */
			if (!strncmp(p, "CDB2ADDR", 8)) {
				sprintf(p+8, "\t0x%04x\n", cdb2addr);
				done = 1;
				}
			}
		fputs(line, out);
		}

	if (!done)
		error("Couldn't find a #define for CDB2ADDR in cdb.h\n");

	if (ERROR == fclose(out)) 
		error("can't close cdb.$$$ (temporary output file)\n");

	if (ERROR == fclose(in)) 
		error("can't close cdb.h\n");

	unlink("cdb.bak");
	if (ERROR == rename("cdb.h", "cdb.bak")) 
		error("can't rename cdb.h to cdb.bak");
	if (ERROR == rename("cdb.$$$", "cdb.h")) 
		error("can't rename cdb.$$$ to cdb.h");

	fprintf(stderr, "Now enter the  CP/M command:\n");
	fprintf(stderr, "     submit makecdb\n");
	fprintf(stderr, "to compile and link the cdb system.\n");
}

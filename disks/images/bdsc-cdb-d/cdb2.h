#include "cdb.h"

/* Copyright (c) 1982-1986 by J. David Kirkland, Jr. */

#define CCC_KHACK  0x013e

#define NEXTKEY ('M'&0x1f)	/* used by printlg to skip to next variable */  
char cancel_char;

/*  the restart-trapping routine pushes the registers at time of restart
 *  and passes a pointer to the block of registers to atbreak.  The
 *  registers end up as follows:
 */
struct savearea {
	int  psw;
	int  bc;
	int  de;
	int  hl;
	unsigned *statement;	/* -> the statement number after the RST */
	char *caller_return;	/* iff the RST was one l2 put in at the start
				 * of a function, this is the return address
				 * that the called function will return to */
	} ;

/* The breaktable "breaktab" records all breakpoints
 * The table is sorted by bfnt (address of function table entry) and bsn 
 * (statement number).  Both sorts are ascending.
 */
struct breakentry {
	struct fntentry *bfnt;
	unsigned bsn;
	unsigned bcount;	/* each time through a breakpoint, decrement
				 * this count; only stop when count == 0  */
	int bpad;
	};

#define MAXBREAKS 40
int nbreaks;
struct breakentry breaktab[MAXBREAKS];

struct savearea *cursave;	/* points to current save area */
struct fntentry *curfnt;	/* fntab entry of current function */

int *curargs;		/* address of first arg to most recently invoked fn */
char curbreak;		/* curfnt->fntbreakindex; 0xff means no breaks in cur
			 * function, else index of first break in this fn */
unsigned cursn;		/* current statement number */

unsigned walkcount;	/* if !=0, decrement at each RST and break when zero */
char trace_display;	/* if !=0, display current location at each RST */
char kbint;		/* if !=0, break when user types any char */


struct fntentry *fntab;		/* address of function table */
struct fntentry *fntabend;	/* address of last entry in function table */

struct stentry *globalp;	/* address of first global symbol table entry */
struct stentry *localp;		/* address of first local symbol table entry */
int globbase;			/* pointer to start of externs [ externs()] */

struct fntentry *a2f_p; 	/* used in addr2fnt; now global for speed  */
char *a2f_addr;
struct breakentry *isb_q; 	/* now global for speed  */

/* these defines are for the "class" returned by get_token */

#define ID 'a'
#define INTEGER '0'
#define HEX 'x'
#define POINT '>'		/* "->" operand */

/* an sttype structure is used by the symbol/expression parsing routines
 * to keep track of the current attributes of the expression.  It is based
 * upon the stentry for the base symbo, as modified by indirection, indexing,
 * etc.
 */
struct sttype {
	char tptfnf;		/* ptr to fnunction flag      */
	char ttype;		/* same type codes as stentry */
	char tsptr;		/* same meaning as stentry for next items */
	char tlind;
	char tforml;
	unsigned tadrs;
	union {
		unsigned u;
		struct stentry *p;
		} tsize;
	unsigned tdimsz;
	unsigned tmul;		/* "multiplicity"--how many occurrences of
				 * the item in question are in this array 
				 * (if it is an array) */
	};

/* types in addition to those listed in cdb.h */

#define VALUE           8	/* the returned value is the actual result,
				 * not the address of the desired result */
#define BAD           255	/* invalid in some way */

/* *
#define DEBUG 0
char debug;
 * */


/* To allow breakpoints at the return point from a function, atbreak changes
 * the actual return address on the target stack.  The trace table "tracetab"
 * keeps track of where all the active functions really need to return.
 */

struct traceentry {
	char *taddr;	/* return address */
	unsigned tsn;	/* statment number within caller -- used only by
			 * list trace command */
	};

char trace_full;	/* true iff we're more than MAXTRACES deep into
			 * subroutine nesting and tracing is turned off */
int ntraces;
#define MAXTRACES 512
struct traceentry tracetab[MAXTRACES];



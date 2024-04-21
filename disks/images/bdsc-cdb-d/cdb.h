#include <stdio.h>

/* Copyright (c) 1982-1986 by J. David Kirkland, Jr. */

/* distribution configuration:
 *   CDB2ADDR      	7900
 *   externs	        c200
 *   BDOS     >=   	d006
 */

/* CDB2ADDR is where cdb2.ovl code sits; run-time tables grow down from it */
/* The cdbconfg utility edits this cdb.h file to change this address */

#define CDB2ADDR	0x8E00

#define RST_NUM		6	/* restart number for debugger breaks */

#define CDB2_FILE  	"CDB2.OVL"   /* name of CDB2 overlay file  */
#define CDB2_DRIVE	""	     /* null default drive for CDB2.OVL */
				     /* (i.e., use CP/M default drive)  */
				     /* use "A:" for drive A, etc.   	*/

#define KBINT_DEF	1	/* user may stop target by typing any key */

/* end of #defines for user customization */

#define COPYRIGHT "(c) 1982-86 by David Kirkland"

/*  each function in the target program has an entry in the function
 *  name table.  The primary source of this information is the .SYM file
 *  produced by l2.  Each entry in the table is a struct fntentry.
 *
 */

#define NOBREAK 255

struct fntentry {
	char *fntaddr;		/* -> first code byte (the RST n 0x0000)
				 * in the function */
	char fntname[9];	/* function name, null terminated */
	char fntbreakindex;	/* index into the breaktab array;
				 * if any breakpoints are set in this function,
				 * breaktab[fntbreakindex] is the first break-
				 * point; if none, value is NOBREAK */
	struct stentry *fntst;	/* -> first symbol table entry for local symbols
				 * of this function or NIL if none */
	unsigned fntfsize;	/* number of bytes in stack frame  */
	};

#define ORIGIN		0x0000  /* CP/M base page */
#define NIL		0	/* NIL pointer    */

/*  every symbol table entry corresponds to either a variable (be it a local,
 *  a global, a formal parameter, or a structure element) or a structure.  
 *  The format is basically that of the CC1 produced symbol table, except:
 *      the name is the LAST 8 bytes, not the first
 *	the "stsize" field has been massaged to some extent
 *  The name is last to enable cdb2, when searching for a local symbol,
 *  to scan through the symbol table until a symbol with a name ending with
 *  an 0xff byte is found (this marks the end of the locals for a given 
 *  function); because the symbol table is built down from the end of the
 *  fn table, and I wanted to avoid wasting a whole record at the end
 *  of each function, the name had to be at the end.  (unclear, but 
 *  think for a while).
 */
struct stentry {
	char stb1, stb2;	/* descriptor bytes */
	unsigned stadrs;	/* relative address of variable */
	unsigned stsize;	/* if struct def:  size of struct  
				 * if struct ref:  -> struct def */
	unsigned stdimsz;	/* 2d dimension of array, or 0xff00 if 1 dim */
	char stname[8];		/* str7 */
	};

/* macros to extract information for descriptor bytes */

#define PTFNF(s)  ((s).stb1&0x80)

#define TYPE(s)  (((s).stb1&0x70)>>4)

#define CHAR 		0
#define INT	     	1
#define UNSIGNED        2
#define LONG            3
#define FLOAT		4
#define DOUBLE		5
#define STRUCT		6
#define FUNCTION        7

#define STELT(s) (((s).stb1&0x08))   	/* 3  is it a structure element ? */
#define FORML(s) (((s).stb1&0x04))   	/* 2  is it a formal parameter ? */
#define WHAT(s)  (((s).stb1&0x03))

#define VARIABLE 	0
#define FUNCDEF		1
#define STRUCTDEF	2
#define FUNCREF		3

#define LIND(s)  (((s).stb2&0xc0)>>6)	/* levels of indirection */
#define CLEV(s)  (((s).stb2&0x3f))	/* "defining block" -- only before
					 * build has processed the entry */

/* after symbol table is built, CLEV is replaced by : */
#define LOCAL(s) (((s).stb2&0x02))	/* 1  is it local ? */
					/* only applies if a structure ref: */
#define SPTR(s)  (((s).stb2&0x01))	/* TRUE  -> stsize is a pointer to
					 * 	    structure definition
					 * FALSE -> stsize is size, in bytes,
					 *          of structure 
					 */

 TYPE(s)  (((s).stb1&0x70)>>4)

#define CHAR 		0
#define INT	     	1
#define UNSIGNED        2
#define LONG         
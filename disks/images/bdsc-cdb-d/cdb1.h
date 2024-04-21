#include "cdb.h"

/* Copyright (c) 1982-1986 by J. David Kirkland, Jr. */

#define CSYMEXT "CDB"		/* extent for debug symbol tables */
#define NFILES  16		/* max number of files for -l or -g options */

/* argb structures are used to define the files specified to -l or -g 
 * options */

struct argb {
	int nfiles;
	char files[NFILES][16];
	};

struct argb locals, globals;

struct fntentry *fntab;		/* function table */

struct stentry *globalp;	/* -> first global st entry */
struct stentry *localp;		/* -> first local st entry */

/* fstentry is like stentry, but in the order it appears in the CDB file */

struct fstentry {
				/* while processing the in-memory CDB file,
				 * build replaces the name field in the 
				 * entry with other information IFF the entry
				 * is a structure definition */
	char fstn1;		/* if struct def && struct is in cdb2 symbol
				 * table, byte is 0xff */
	struct stentry *fstp;	/* if fstn1 == 0xff, fstp points to cdb2 symbol
				 * table entry for the struct def */
	char fstname[5];	/* pad */

	char fstb1, fstb2;	/* see stentry ..... */
	int  fstadrs;
	int  fstsize;
	unsigned fstdimsz;
	};

int  fd;			/* stuff for build */
char *inbufp;

/*
#define TYPE(s)  (((s).stb1&0x70)>>4)
#define STELT(s) (((s).stb1&0x08)>>3)	/* is it a structure element ? */
#define FORML(s) (((s).stb1&0x04)>>2)	/* is it a formal parameter ? */
#define WHAT(s)  (((s).stb1&0x03))
#define LIND(s)  (((s).stb2&0xc0)>>6)	/* levels of indirection */
#define CLEV(s)  (((s).stb2&0x3f))	/* "defining block" -- only before
					 * build has processed the entry */

/* after symbol table is built, CLEV is replaced by : */
#define LOCAL(s) (((s).stb2&0x02)>>1)	/* is it local ? */
					/* only applies if a structure ref: */
#define SPTR(s)  (((s).stb2&0x01))	/* TRUE  -> stsize is a pointer to
					 * 	    structure definition
					 * FALSE -> stsize is size, in bytes,
					 *          of structure 
					 */
*/



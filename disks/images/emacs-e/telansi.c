#if 0
# include "stdio.h"         /* just not needed */
# include "ed.h"            /* presumably prototype for conout */
#endif

#define	NROW	24			/* Screen size.			*/
#define	NCOL	80			/* Edit if you want to.		*/
#define	BEL	0x07			/* BEL character.		*/
#define	ESC	0x1B			/* ESC character.		*/
int ansiterm;

int conout( int );          /* assume */

/* termctrl contains 3 screen control sequences, preceded by their length in bytes; 
   These original sequences were possibly for the Commodore C128; but they are not ansi.
   termctrl[0] is the sequence to move the cursor to POS(x,y);
   termctrl[1]   to clear from the cursor position to EOL;
   termctrl[2]   to clear the screen.
   You are supposed to be able to re-program these sequences using METERM.COM;
     but you'll need to change ansimove() too */
#if 0
	/* C128? */
char termctrl[3][10] =
{	4, ESC, '=', ' ', ' ', 0,0,0,0,0,   /* POS - ansimove() */
	2, ESC, 'T', 0,0, 0,0,0,0,0,        /* clear to EOL - ansieeol() */
	1, 26,     0,0,0, 0,0,0,0,0         /* CLS - ansiclear() */
};

ansimove(row, col)
{	register char * cp;
	register int i;

	cp = &termctrl[0][0];
	i = *cp++;
	conout( *cp++ ); --i;
	conout( *cp++ ); --i;
	if ( ! ansiterm )
	{	conout( row + *cp++ ); ++i;
		conout( col + *cp++ ); ++i;
	}
	else
	{	conout( ( row / 10 ) + *cp++ ); ++i;
		conout( ( row % 10 ) + *cp++ ); ++i;
		conout( *cp++ ); --i;
		conout( ( row / 10 ) + *cp++ ); ++i;   /* I suspect this is supposed to be col not row */
		conout( ( row % 10 ) + *cp++ ); ++i;   /*    ditto    */
	}
	while ( --i >= 0 ) conout( *cp++ );        /*    er    */
}
#else
	/* ANSI */                                  /* https://en.wikipedia.org/wiki/ANSI_escape_code */
char termctrl[3][10] =
{	8, ESC, '[', '0', '0', ';', '0', '0', 'H', 0,   /* POS - ansimove() */
	4, ESC, '[', '0', 'K', 0,0,0,0,0,               /* clear to EOL - ansieeol() */
	4, ESC, '[', '2', 'J', 0,0,0,0,0                /* CLS - ansiclear() */
};

ansimove(row, col)
{	register char * cp;
	register int i;

	cp = &termctrl[0][0];
	i = *cp++;                                  /* get length of escape sequence */
	  /* Agon CP/M sets VDP into terminal mode, so we will use ANSI escape sewuences */
	conout( *cp++ ); --i;
	conout( *cp++ ); --i;
	conout((( row + 1 )/ 10 )+ *cp++ ); ++i;
	conout((( row + 1 )% 10 )+ *cp++ ); ++i;
	conout( *cp++ ); --i;
	conout((( col + 1 )/ 10 )+ *cp++ ); ++i;
	conout((( col + 1 )% 10 )+ *cp++ ); ++i;
	conout( *cp++ ); --i;
}
#endif

ansiclear()
{	ctlout( 2 );
}

ansieeol()
{	ctlout( 1 );
}

ctlout( x )
{	register char * cp;
	register int i;

	cp = &termctrl[x][0];
	i = *cp++;
	while ( --i >= 0 ) conout( *cp++ );
}

ansibeep()
{
	conout( BEL );
}

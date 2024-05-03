
/*
RED terminal output module
Source:  red6.c
This file was created by the configuration program:
February 6, 1985

Modified 19.April.2024 for Agon Light terminal commands
*/

#include "red.h"


/*
Define the current coordinates of the cursor.
*/

#ifdef HAS_STATIC
static
int outx;
static
int outy;
#endif


/*
Return the current coordinates of the cursor.
*/

outgetx()
{
	return outx;
}

outgety()
{
	return outy;
}


/*
Initialize the globals which describe the terminal.
*/

outinit()
{
	hasdn   = NO;
	hasup   = NO;
	hasins  = NO;
	hasdel  = NO;
	hasint  = NO;
	hascol  = NO;
	haswrap = NO;
}


/*
Output one printable character to the screen.
*/

outchar(c)
char c;
{
	syscout(c);
	outx++;
	return c;
}


/*
Position cursor to position x,y on screen.
0,0 is the top left corner.
*/

outxy(x,y)
int x, y;
{
	outx = x;
	outy = y;
	syscout(27);
	syscout('[');
	syscout(((outy+1)/10)+48);
	syscout(((outy+1)%10)+48);
	syscout(';');
	syscout(((outx+1)/10)+48);
	syscout(((outx+1)%10)+48);
	syscout('H');
}


/*
Erase the entire screen.
Make sure the rightmost column is erased.
*/

outclr()
{
	syscout(27);
	syscout('[');
	syscout('2');
	syscout('J');
	
	outxy(0,0);
}


/*
Delete the line on which the cursor rests.
Leave the cursor at the left margin.
*/

outdelln()
{
	outxy(0,outy);
	outdeol( );
}


/*
Delete to end of line.
Assume the last column is blank.
*/

outdeol()
{
	syscout(27);
	syscout('[');
	syscout('0');
	syscout('K');
}


/*
Hardware insert line.
*/

outins()
{
	syscout(27);
	syscout('L');
}


/*
Hardware delete line.
*/

outdel()
{
	syscout(27);
	syscout('M');
}


/*
Scroll the screen up.
Assume the cursor is on the bottom line.
*/

outsup()
{
	/* auto scroll */
	outxy(0,SCRNL1);
	syscout(10);
}


/*
Scroll screen down.
Assume the cursor is on the top line.
*/

outsdn()
{
}


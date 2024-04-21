#include "cdb2.h"

/* Copyright (c) 1982-1986 by J. David Kirkland, Jr. */

union valun {
	int intval;
	int *ptrval;
	};

ishex(c) char c; {

	return isdigit(c) || ('a'<=c&&c<='f') || ('A'<=c&&c<='F');
}

struct stentry *findsym (symbol) char *symbol; {

	/* Look up a symbol in the symbol table.  The null-terminated string
	 * "symbol" is searched for in the table.  If it is present, findsym
	 * returns a pointer to the correct symbol table entry; else, a 
	 * diagnostic is printed and ERROR is returned.
	 * Normally, first the local symbols for the current function are
	 * searched, followed by the globals; but if the symbol is preceeded
	 * by a backslash, only the global symbols are searched.
	 */

	char s[8], t[9], globflag;
	struct stentry *p;

	globflag = '\\' == *symbol;
	movmem(&symbol[globflag],t,8);
	t[8] = 0;
	str07cpy(s,t);

	if ((!globflag) && NIL!=(p=curfnt->fntst))
		for (; p->stname[7]!=0xff; p--) {
			if (str7eq(s,p->stname))
				return p;
			}

	for (p=globalp; p>localp; p--) {
		if (str7eq(s,p->stname))
			return p;
		}

	printf("no symbol %s\n",symbol);
	return ERROR;
}

char get_token(sp,t,valp) char **sp, *t; int *valp; {

	/* get_token isolates the next token on the input line.
	 * sp is a pointer to a pointer to an array of characters forming the
	 * line to be scanned.  *sp is updated to point to the next unprocessed
	 * character once get_token has gotten its token.
	 * The token scanned is copied into t; valp is set to the integer value
	 * of the token iff the token is an integer.
	 * get_token returns the "class" of the token, which can be
	 * (i) INTEGER (a decimal or hex integer), (ii) ID (a sequence of
	 * _, alpha, or digits that doesn't start with a digit), (iii) POINT
	 * (the sequence "->"), or (iv) any special character.
	 * 
	 */

	char class, *s, *tt;

	tt = t;
	s = *sp;
	while(isspace(*s))
		++s;
	if (isalpha(*s) || *s=='_' || *s=='\\') {         /* identifier */
		class = ID;
		do
			if (t-tt<=8)
				*t++ = *s;
		while (isdigit(*++s) || isalpha(*s) || *s == '_');
		}
	else if (isdigit(*s) || ((*s=='-' || *s=='+') && isdigit(*(s+1)) ) ) {
		class = INTEGER;
		do
			if ('x' == tolower(*s)) {
				class = HEX;
				t = tt;
				}
			else if (t-tt<=8)
				*t++ = *s;
			else if (class!=BAD) {
				printf("integer too long\n");
				class = BAD;
				}
		while (ishex(*++s) || *s == 'x');

		*t = 0;
		if (class!=BAD) {
			sscanf(tt,(class==INTEGER) ? "%d" : "%x",valp);
			class = INTEGER;
			}
		}
	else {
		if (class = *s) s++;
		if (class == '-' && *s == '>') {
			class = POINT;
			++s;
			}
		*t++ = class;
		}

	*t = 0;
	/* * 
	if (debug) printf("get_token returning class = %c, token = <%s>, value = %d\n",
		class, tt, *valp);
	 * */
	*sp = s;
	return class;
}

invalid(t) char t; {

	if (t>=0x20 && t<0x80)
		printf("'%c'",t);
	else
		printf("'\\%o'",t);
}

sizeelt(type) struct sttype *type; {

	/* compute the size, in bytes, of a single element of 
	 * the object described by "type".
	 */

	if (type->tlind!=0 || type->tptfnf)
		return 2;
	else if (type->ttype==STRUCT)
		if (type->tsptr)
			return type->tsize.p->stsize;
		else
			return type->tsize.u;
	else if (type->ttype==CHAR)
		return 1;
	else 
		return 2;
}

sizeary(s) struct stentry *s; {

	/* Compute the size, in bytes, of the array with the symbol table entry
	 * pointed to by s.
	 * Method: find the variable that follows this variable, and compute
	 * the difference in adrs fields.  Special cases occur when the
	 * array in question is (i) the last element in a structure, 
	 * (ii) the last local variable in a function, or (iii) the last
	 * global variable. (This is the order in which we treat them below.)
	 */

	struct stentry *next;
	unsigned naddr, *up;

	next = s-1;

	if (STELT(*s))
		if (STELT(*next))
			naddr = next->stadrs;
		else {
			for (next = s; WHAT(*++next)!=2; ) ;
			naddr = next->stsize;
			}
	else {
	        while (next->stname[7]!=0xff && (WHAT(*next)!=0 || STELT(*next)) )
			next--;
		if (next->stname[7] == 0xff)
			if (LOCAL(*s)) {
				up = &(next->stname[5]);
				naddr = *up;
#ifdef DEBUG
 				if (debug) printf("   local sfsize = %04x\n",
						  naddr);
#endif
				}
			else 
				naddr = endext() - externs();
		else
			naddr = next->stadrs;
		}

	return naddr - s->stadrs;
}

gettt(s,t) struct stentry *s; struct sttype *t; {

	/* Copy information from a symbol table entry into a sttype structure.
	 * Most of this is done solely to make it easier to change the data 
	 * so copied when exp or primary needs to do so.
	 */

	t->tptfnf = PTFNF(*s);
	t->ttype = TYPE(*s);
	t->tsptr = SPTR(*s);
	t->tlind = LIND(*s);
	t->tforml = FORML(*s);
	t->tadrs = s->stadrs;
	t->tsize.u = s->stsize;
	t->tdimsz = s->stdimsz;
	if (t->tdimsz && !t->tforml)
	        t->tmul = sizeary(s)/sizeelt(t);
	else
		t->tmul = 1;
}

exp (s, term, type) char **s, term; struct sttype *type; {

	/* exp() processes expressions.
	 * expression := *expression
	 *		 primary
	 *
	 * s is the command line pointer-to-pointer; term is the character
	 * that marks the end of the expression; it is usually either
	 * '\0', ']', or ')'.
	 * exp returns the value computed for the expression (i.e., the
	 * address defined by the symbolic reference or the actual value
	 * of the integer entered), and sets *type as appropriate.
	 */

	char token[10], class, *ss;
	union valun aval;

#ifdef DEBUG
	if (debug) printf("exp( %s, '%c')\n",*s,term);
#endif
	ss = *s;
	class = get_token(s, token, aval);

	if (class=='*') {
		aval.intval = exp(s,term,type);
		aval.intval = *aval.ptrval;
		if (type->tlind)
			type->tlind--;
		else if (type->tdimsz!=0xff00) {
		        if (type->ttype==CHAR)
				aval.intval &= 0xff;
			type->ttype = INT;
			}
#ifdef DEBUG
		if (debug) printf("exp [*] returning %04x = %d\n",aval.intval,aval.intval);
#endif
		return aval.intval;
		}
	else if (class==term)  {
		printf("empty expression at ");
		invalid(term);
		putchar('\n');
		type->ttype = ERROR;
		return;
		}
	else if (class==BAD) {
		type->ttype = ERROR;
		return;
		}

	*s = ss;
	return primary(s,term,type);
}

/*
unsigned baseaddr(stab) struct stentry *stab; {
	unsigned result;

	if (LOCAL(*stab)) {
		if (cursn==0) {
			result = &(cursave->caller_return);
			result -= curfnt->fntfsize + 2;
			}
		else
			result = cursave->bc;
		if (FORML(*stab))
			result += 4 + curfnt->fntfsize;
		}
	else {
#ifdef DEBUG
		if (debug) printf("globbase = %04x\n",globbase);
#endif
		result = globbase;
		}

	return result;
}
*/

unsigned do_stentry(stab,type) struct stentry *stab; struct sttype *type; {
	unsigned base;

	gettt(stab,type);

	if (STELT(*stab))
		base = 0; 
	else if (LOCAL(*stab)) {
		if (cursn==0) {
			base = &(cursave->caller_return);
			base -= curfnt->fntfsize + 2;
			}
		else
			base = cursave->bc;
		if (FORML(*stab))
			base += 4 + curfnt->fntfsize;
		}
	else
 		base = globbase;

#ifdef DEBUG
	if (debug) printf("adrs = %04x, base = %04x\n",type->tadrs,base);
#endif
	return base + type->tadrs;
}

do_id (ident, type) char *ident; struct sttype *type; {

	/* do_id processes an identifier by looking up the symbol given
	 * in "ident" in the symbol table and computing the absolute 
	 * memory address of that symbol. This address is returned, and
	 * type is set.
	 */

	struct stentry *stab;

	if (ERROR==(stab = findsym(ident)))
		type->ttype = BAD;
	else
		return do_stentry(stab,type);
}

do_index(s, aval, type) char **s; union valun *aval; struct sttype *type; {

	/* do_index processes a single subscript to an already-processed  
	 * primary.  aval and type contain the address and other information
	 * derived from the primary; do_index updates all this information
	 * to take the indexing into account.
	 * s is pointer-to-pointer to input line.
	 */

	union valun bval;
	struct sttype type2;
	char token[10];
	int i, scale;

	if (type->tforml) {
		aval->intval = *aval->ptrval;
		type->tforml = 0;	/* only do indirection once for */
		}			/* 2-dim arrays */

	bval.intval = exp(s,']', type2 );

	if (type2.ttype==BAD) {
		type->ttype = BAD;
		return;
		}

	get_token(s, token, aval);
	i = (type2.ttype==VALUE) ? bval.intval : *bval.ptrval;
	if (type2.tlind==0 && type2.ttype==CHAR)
		i &= 0xff;

	scale = sizeelt(type);
#ifdef DEBUG
	if (debug) printf("   scale = %d, i = %d, dimsz = %04x\n",scale,i,type->tdimsz);
#endif
	if (type->tdimsz!=0xff00) {
		scale *= type->tdimsz;
		type->tmul = type->tdimsz;
		type->tdimsz = 0xff00;
#ifdef DEBUG
		if (debug) printf("   new scale = %d, mul = %04x\n",scale,type->tmul);
#endif
		}
	else {
		type->tmul = 1;
		type->tdimsz = 0;
		}

	aval->intval += scale * i; 
}

do_struct(class, s, aval, type) 
char class; char **s; union valun *aval; struct sttype *type; {

	/* do_struct processes a structure reference to an already-processed  
	 * primary.  aval and type contain the address and other information
	 * derived from the primary; do_struct updates all this information
	 * to take the structure reference into account.
	 * class contains either '.' or POINT and tells do_struct whether
	 * aval contains the address of the structure or the address of a
	 * pointer to the structure.
	 * s is pointer-to-pointer to input line.
	 */

	char token[10];
	union valun bval;
	struct stentry *stab;

	if (class==POINT)
		aval->intval = *aval->ptrval;

	get_token(s, token, bval);

	if (ERROR==(stab = findsym(token)))
		type->ttype = BAD;
	else if (!STELT(*stab)) {
 		printf("%s is not a structure element\n", token);
		type->ttype = BAD;
		}
	else {
		aval->intval += stab->stadrs;
		gettt(stab,type);
		}
}

primary (s, term, type) char **s, term; struct sttype *type; {

	/* primary() processes primaries.
	 * primary  :=  (expression)
	 *		primary[expression]
	 *		primary->indentifier
	 *		primary.indentifier
	 *
	 * s is the command line pointer-to-pointer; term is the character
	 * that marks the end of the expression; it is usually either
	 * '\0', ']', or ')'.
	 * primary returns the value computed for the expression (i.e., the
	 * address defined by the symbolic reference or the actual value
	 * of the integer entered), and sets *type as appropriate.
	 */

	char token[10], class, *ss;
	union valun aval, bval;

#ifdef DEBUG
	if (debug) printf("primary( %s, '%c' )\n", *s, term);
#endif
	class = get_token(s, token, aval);

	if (class=='(') {
		aval.intval = exp(s,')',type);
	        get_token(s, token, aval);
		}
	else if (class==ID)
		aval.intval = do_id(token,type);
	else if (class==INTEGER) {
		type->ttype = VALUE;
		type->tdimsz = 0;
		type->tmul = 1;
		type->tptfnf = 0;
		type->tlind = 0;
		}
	else {
		if (class!=BAD) {
			printf("invalid primary at token %s", token);
			printf(", identifier or integer expected\n");
			}
		type->ttype = BAD;
		}

	ss = *s;
	while (type->ttype!=BAD && term != (class=get_token(s, token, bval))) {
		if (class=='[')
			do_index(s, aval, type);
		else if (class==POINT || class=='.')
			do_struct(class, s, aval, type);
		else
			if (class==INTEGER || class=='\'' || class==ID)
				break;
			else {
				if (class!=BAD) {
					printf("invalid primary at token ");
					invalid(class);
					putchar('\n');
					}
				type->ttype = BAD;
				}
		ss = *s;
		}

	*s = ss;

#ifdef DEBUG
	if (debug) printf("primary returning type = %d, value = %04x = %d\n",
		type->ttype, aval.intval, aval.intval);
	if (debug) printf("  lind = %d, size = %d, mul = %04x, dimsz = %04x\n",
		type->tlind, type->tsize, type->tmul, type->tdimsz);
#endif

	return aval.intval;
}

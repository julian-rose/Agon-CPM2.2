! VE - Visual Editor
! A very simple VI surrogate for CP/M
! Nils M Holm, 2020, 2024
! In the public domain

! Todo: tab length in GETSTR

use t3x: t;
use console: con;

const	SIZE	= 27000;
const	XMAX	= 80;

const	MAXLINE	= XMAX;
const	TABSIZE	= 8;
const	YMAX	= 23;
const	YBUFL	= 256;

var	Iobuf::128;
var	Tmpfile;

var	Cmd, Cmds, Mmap;
var	File::14;
var	Buffer::SIZE;
var	Gap, Egap, Point;
var	Line;
var	Block, Eblock, Bline, Eline;

var	Xpos, Ypos, Head;
var	Stx;

var	Ybuf::YBUFL;
var	Ylen, Yline;

var	Marker;
var	Nodraw;
var	Rdonly;

var	Msg, Modified;

str_length(s) return t.memscan(s, 0, 32767);

str_copy(a, b) t.memcopy(a, b, str_length(b)+1);

reset() do
	Gap := 0;
	Egap := SIZE;
	Point := 0;
	Line := 0;
	Block := 0;
	Eblock := 0;
	Xpos := 0;
	Ypos := 0;
	Head := 0;
	Stx := 0;
	Msg := 0;
	Ylen := %1;
	Yline := 0;
	Modified := 0;
	Marker := %1;
	Nodraw := 0;
	Rdonly := 0;
end

!For Agon we can define values for con.clreol and con.clrscr (see init() below);
! but we cannot define con.move as a constant string. To keep it simplest, we will
! just write a function called conMove
conMove(x,y) do var lx, ly;
	lx := x + 1;
	ly := y + 1;
	con.wrch('\e');
	con.wrch('[');
	con.wrch((ly / 10)+'0');
	con.wrch((ly mod 10)+'0');
	con.wrch(';');
	con.wrch((lx / 10)+'0');
	con.wrch((lx mod 10)+'0');
	con.wrch('H');
end

post(s) do
	conMove(0, YMAX);
	con.clreol();
	con.writes(s);
	Msg := s;
end

prompt(s) do
	post(s);
	Msg := 0;
end

clearmsg() prompt("");

huh() post("HUH?");

movegap(p) do
	ie (Gap < p) do
		t.memcopy(@Buffer::Gap, @Buffer::Egap, p-Gap);
		Egap := Egap + p - Gap;
	end
	else if (Gap > p) do
		Egap := Egap + p - Gap;
		t.memcopy(@Buffer::Egap, @Buffer::p, Gap-p);
	end
	Gap := p;
end

textsize() return SIZE-Egap+Gap;

normalize() movegap(textsize());

pos(n) return n<Gap-> n: n+Egap-Gap;

textinfo(sv) do var i, n, m, lim;
	normalize();
	n := 0;
	m := 1;
	lim := textsize();
	for (i=0, lim) do
		if (Buffer::i = '\n') do
			m := m+1;
			if (i < Point) n := n+1;
		end
	end
	sv[0] := n;
	sv[1] := m;
	return lim;
end

var ntoab::10;

ntoa(n) do var i;
	if (\n) return "0";
	i := 9;
	ntoab::9 := 0;
	while (n) do
		i := i-1;
		ntoab::i := n mod 10 + '0';
		n := n / 10;
	end
	return @ntoab::i;
end

report() do var v[2];
	conMove(0, YMAX);
	con.clreol();
	con.writes(ntoa(textinfo(v)));
	con.writes("C ");
	con.writes(ntoa(v[1]-1));
	con.writes("L ");
	con.writes(ntoa(v[0]+1));
	con.writes(" ");
	con.writes(File);
	if (Modified) con.writes(" *");
	Msg := 1;
end

moveto(p) do
	Point := p;
	for (Line = Buffer::pos(Point) = '\n'-> Point-1: Point, %1, %1)
		if (Buffer::pos(Line) = '\n') leave;
	Line := Line+1;
end

advance(n) do var m, lim;
	if (Point + n > textsize()) return %1;
	m := 0;
	lim := Point+n;
	for (Point = Point, lim)
		if (Buffer::pos(Point) = '\n') m := m+1;
	moveto(Point);
	return m;
end

goback() do var m;
	if (Point < 1) return %1;
	Point := Point - 1;
	m := 0;
	if (Buffer::pos(Point) = '\n') m := 1;
	moveto(Point);
	return m;
end

next(n) do var i, lim;
	normalize();
	lim := textsize();
	for (i=Point, lim) do
		if (\n) leave;
		if (Buffer::i = '\n') n := n-1;
	end
	if (n) return %1;
	moveto(i);
end

prev(n) do var i;
	normalize();
	if (\Line) return %1;
	for (i=Line-1, %1, %1) do
		if (Buffer::i = '\n') n := n-1;
		if (\n) leave;
	end
	if (n) return %1;
	while (i > 0 /\ Buffer::(i-1) \= '\n')
		i := i-1;
	moveto(i);
end

eol() do var lim;
	lim := textsize();
	for (Point=Point, lim)
		if (Buffer::pos(Point) = '\n')
			leave;
	return Point-Line;
end

insertblock(s, k) do var lim;
	Modified := %1;
	Marker := %1;
	if (Gap+k >= Egap) return %1;
	movegap(Point);
	t.memcopy(@Buffer::Gap, s, k);
	Gap := Gap+k;
end

efind(s, start, lim) do var k, i, c;
	k := str_length(s);
	normalize();
	lim := (lim-> lim: Gap) - k;
	c := s::0;
	for (i=start, lim) do
		if (	Buffer::i = c /\
			\t.memcomp(@Buffer::i, s, k)
		)
			return i;
	end
	return %1;
end

erfind(s, start, lim) do var k, i, c;
	k := str_length(s);
	normalize();
	lim := (lim-> lim: Gap) - k;
	c := s::0;
	start := start-1;
	for (i=lim-1, start, %1) do
		if (	Buffer::i = c /\
			\t.memcomp(@Buffer::i, s, k)
		)
			return i;
	end
	return %1;
end

mark() do
	Block := Point;
	Bline := Line;
end

setblock() do var x;
	Eblock := Point;
	if (Eblock <= Block) do
		x := Eblock;
		Eblock := Block;
		Block := x;
	end
	Eline := Line;
end

nospace() post("NO SPACE");

readblock(name) do var fd, k, j, c;
	fd := t.open(name, T3X.OREAD);
	if (fd < 0) return %1;
	movegap(Point);
	while (Gap < Egap) do
		k := t.read(fd, Iobuf, 128);
		j := 0;
		while (j < k /\ Gap < Egap) do
			c := Iobuf::j;
			j := j+1;
			Buffer::Gap := c;
			if (c = 26) leave;
			if (c \= '\r') Gap := Gap+1;
		end
		if (k < 128 \/ c = 26) leave;
	end
	if (Gap = Egap) do
		nospace();
		Rdonly := 1;
	end
	t.close(fd);
end

load(name) do
	reset();
	prompt("READING");
	readblock(name);
	if (Gap = 0) do
		Buffer::0 := '\n';
		Gap := 1;
	end
	if (\Msg) report();
end

writeblock(name, n0, nn) do var fd, k, len, lim, i, j1, j2, c;
	normalize();
	fd := t.open(name, T3X.OWRITE);
	if (fd < 0) return %1;
	len := nn-n0;
	for (i=n0, nn, 64) do
		j1 := 0;
		j2 := 0;
		lim := len<64-> len: 64;
		while (j1 < lim) do
			c := Buffer::(i+j1);
			j1 := j1+1;
			if (c = '\n') do
				Iobuf::j2 := '\r';
				j2 := j2+1;
			end
			Iobuf::j2 := c;
			j2 := j2+1;
		end
		k := t.write(fd, Iobuf, j2);
		if (k \= j2) do
			t.close(fd);
			return %1;
		end
		len := len - lim;
	end
	t.close(fd);
end

save(name) do
	if (rdonly) return post("READ ONLY");
	post("WRITING");
	ie (writeblock(name, 0, textsize()) < 0)
		post("WRITE ERROR");
	else do
		Modified := 0;
		report();
	end
end

edeleteblock() do
	Modified := %1;
	Marker := %1;
	moveto(Block);
	movegap(Block);
	Egap := Egap+Eblock-Block;
end

flatpos(x) do var rx, i, c, lim;
	lim := textsize();
	rx := 0;
	i := Line;
	for (x=x, 0, %1) do
		c := Buffer::pos(i);
		if (c = '\n' \/ i > lim) leave;
		rx := rx + (c = '\t'->
			TABSIZE - (rx+TABSIZE) mod TABSIZE: 1);
		i := i+1;
	end
	return rx;
end

adjust() do var rx, k, i, c, lim;
	rx := 0;
	lim := textsize();
	for (i=0, MAXLINE) do
		if (Point+i >= lim) leave;
		c := Buffer::pos(Point+i);
		if (rx >= Stx \/ c = '\n') leave;
		rx := rx + (c = '\t'->
			TABSIZE - (rx+TABSIZE) mod TABSIZE: 1);
	end
	Xpos := i;
	advance(i);
end

println(x) do var i, c, co, lim;
	co := flatpos(x);
	con.clreol();
	lim := textsize();
	for (i=Line, lim) do
		if (co >= XMAX) leave;
		c := Buffer::pos(x+i);
		if (c = '\n') leave;
		ie (c = '\t') do
			while (1) do
				con.wrch('\s');
				co := co+1;
				if (co mod TABSIZE = 0) leave;
			end
		end
		else do
			con.wrch(c);
			co := co+1;
		end
	end
end

refresh(first, last) do var i, k, pt;
	if (Nodraw) return;
	pt := Point;
	moveto(Head);
	if (first) next(first);
	for (i=first, last+1) do
		conMove(0, i);
		println(0);
		if (next(1)) leave;
	end
	while (i <= last) do
		conMove(0, i);
		con.clreol();
		con.wrch('~');
		i := i+1;
	end
	moveto(pt);
end

display() refresh(0, YMAX-1);

disprest() refresh(Ypos, YMAX-1);

displine() refresh(Ypos, Ypos);

sync() conMove(flatpos(Xpos), Ypos);

goto(p) do var i, r, rest;
	moveto(Head);
	for (i=0, YMAX) if (next(1)) leave;
	rest := Line;
	ie (p >= Head /\ p < rest) do
		moveto(Head);
		r := 0;
	end
	else do
		moveto(p);
		for (i=0, YMAX/2-1) if (prev(1)) leave;
		Head := Line;
		moveto(Head);
		display();
		r := 1;
	end
	Ypos := advance(p-Head);
	Xpos := p - Line;
	Stx := flatpos(Xpos);
	return r;
end

newtop() do
	Head := Line;
	Ypos := 0;
	display();
end

key() do var k;
	k := con.getkey();
	if (Msg) clearmsg();
	return k & 127;
end

_down() do var x, pt;
	if (next(1)) return %1;
	Ypos := Ypos + 1;
	if (Ypos >= YMAX) do
		con.scroll();
		pt := Point;
		moveto(Head);
		next(1);
		Head := Line;
		moveto(pt);
		Ypos := YMAX-1;
		refresh(YMAX-1, YMAX-1);
	end
	Xpos := 0;
end

_up() do var x, pt;
	if (prev(1)) return %1;
	Ypos := Ypos-1;
	if (Ypos < 0) do
		con.rscroll();
		pt := Point;
		goto(Head);
		prev(1);
		Head:= Line;
		goto(pt);
		Ypos := 0;
		refresh(0, 0);
	end
	Xpos := 0;
end

var Linemo;

down() do var r;
	r := _down();
	Linemo := 1;
	if (\r) adjust();
	return r;
end

up() do var r;
	r := _up();
	Linemo := 1;
	if (\r) adjust();
	return r;
end

right() do var nl;
	nl := advance(1);
	if (nl < 0) return %1;
	ie (nl) do
		goback();
	end
	else do
		Xpos := Xpos+1;
		Stx := flatpos(Xpos);
	end
end

left() do var nl;
	nl := goback();
	if (nl < 0) return %1;
	ie (nl) do
		advance(1);
	end
	else do
		Xpos := Xpos-1;
		Stx := flatpos(Xpos);
	end
end

home() do
	Point := Line;
	Xpos := 0;
	Stx := 0;
end

toeol() do
	Xpos := eol();
	Stx := flatpos(Xpos);
end

toeoscr() do var i;
	goto(Head);
	for (i=1, YMAX)
		if (_down())
			leave;
end

top() do var hd;
	hd := head;
	Head := 0;
	goto(0);
	Xpos := 0;
	Ypos := 0;
	Stx := 0;
	if (hd) display();
end

bottom() do var pt;
	moveto(textsize());
	pt := Line;
	if (prev(YMAX-1)) do
		top();
		while (\next(1)) Ypos := Ypos+1;
		return;
	end
	Head := Line;
	Ypos := YMAX-1;
	moveto(pt);
	display();
	Xpos := 0;
	Stx := 0;
end

iswhite(c) return c = '\s' \/ c = '\t';

skipword(lim) do
	while (\iswhite(Buffer::pos(Point)) /\ Point < lim) do
		if (Buffer::pos(Point) = '\n') leave;
		Point := Point+1;
	end
end

wordright() do var lim, p;
	p := Point;
	lim := textsize();
	skipword(lim);
	while (iswhite(Buffer::pos(Point)) /\ Point < lim)
		Point := Point+1;
	Xpos := Xpos + Point - p;
	Stx := flatpos(Xpos);
end

eoword(chg) do var lim, p;
	p := Point;
	lim := textsize();
	if ((Buffer::pos(Point) \= '\n')) advance(1);
	while (iswhite(Buffer::pos(Point)) /\ Point < lim)
		Point := Point+1;
	skipword(lim);
	if (\chg /\ Point \= Line) goback();
	Xpos := Xpos + Point - p;
	Stx := flatpos(Xpos);
end

wordleft() do var p;
	p := Point;
	while (\iswhite(Buffer::pos(Point)) /\ Point > Line)
		Point := Point-1;
	while (iswhite(Buffer::pos(Point)) /\ Point > Line)
		Point := Point-1;
	while (\iswhite(Buffer::pos(Point)) /\ Point > Line)
		Point := Point-1;
	if (iswhite(Buffer::pos(Point))) Point := Point+1;
	Xpos := Xpos + Point - p;
	Stx := flatpos(Xpos);
end

findchar(ch, onto) do var lim, p;
	p := Point;
	lim := textsize();
	if (Buffer::pos(Point) = '\n') return;
	Point := Point+1;
	while (Buffer::pos(Point) \= ch /\ Point < lim) do
		if (Buffer::pos(Point) = '\n') leave;
		Point := Point+1;
	end
	if (Buffer::pos(Point) \= ch) do
		Point := p;
		return;
	end
	if (Cmd = 'd' \/ Cmd = 'c' \/ Cmd = 'y') Point := Point+1;
	if (\onto) Point := Point-1;
	Xpos := Xpos + Point - p;
	Stx := flatpos(Xpos);
end

nextpage() do
	goto(Head);
	ie (\next(YMAX)) do
		prev(1);
		Head := Line;
		Xpos := 0;
		Stx := 0;
		Ypos := 0;
		display();
	end
	else
		bottom();
end

prevpage() do
	goto(Head);
	ie (\prev(YMAX-1)) do
		Head := Line;
		Xpos := 0;
		Stx := 0;
		Ypos := 0;
		display();
	end
	else
		top();
end

find(s, rs, re) do var p;
	p := efind(s, rs, re);
	if (p < 0) return %1;
	goto(p);
end

rfind(s, rs, re) do var p;
	p := erfind(s, rs, re);
	if (p < 0) return %1;
	goto(p);
end

gotoln(n) do var i, lim;
	normalize();
	lim := textsize();
	for (i=0, lim) do
		if (\n) leave;
		if (Buffer::i = '\n') n := n-1;
	end
	if (n) return %1;
	Line := i;
	goto(i);
end

indent() do var k, c, i, n, pl;
	if (prev(1)) return 0;
	pl := Line;
	next(1);
	Xpos := 0;
	Stx := 0;
	n := 0;
	movegap(Point);
	while (1) do
		c := Buffer::pos(pl);
		pl := pl+1;
		ie (c = '\s' \/ c = '\t') do
			if (Gap >= Egap) return %1;
			Buffer::Gap := c;
			Gap := Gap+1;
			n := n+1;
			right();
		end
		else do
			leave;
		end
	end
	conMove(0, Ypos);
	println(0);
	return n;
end

deleteblock() do
	if (Block = Eblock) return;
	goto(Block);
	edeleteblock();
	ie (Bline = Eline)
		displine();
	else
		disprest();
end

getstr(b) do var k, n, i, j;
	n := 0;
	while (1) do
		k := con.getkey();
		if (k = '\e' \/ k = '\r' \/ k = 3) do
			b::n := 0;
			return k;
		end
		ie (k = '\b' /\ n) do
			n := n-1;
			j := b::n = '\t'-> TABSIZE: 1;
			for (i=0, j) con.writes("\b\s\b");
		end
		else ie (k = 21) do
			while (n) do
				con.writes("\b\s\b");
				n := n-1;
			end
		end
		else ie (k = '\t' /\ n < XMAX) do
			for (i=0, TABSIZE)
				con.wrch('\s');
			b::n := k;
			n := n+1;
		end
		else if ('\s' <= k /\ k <= '~' /\ n < XMAX) do
			con.wrch(k);
			b::n := k;
			n := n+1;
		end
	end
end

var	Findbuf::XMAX+1;
var	Replbuf::XMAX+1;

nomatch() post("NO MATCH");

findnext()
	if (find(Findbuf, Point+1, textsize()) < 0)
		nomatch();

findprev()
	if (rfind(Findbuf, 0, Point-1) < 0)
		nomatch();

dosearch(rev) do var k;
	conMove(0, YMAX);
	con.wrch(rev-> '?': '/');
	Msg := 1;
	k := getstr(Iobuf);
	if (k = 3) return;
	if (str_length(Iobuf))
		str_copy(Findbuf, Iobuf);
	ie (rev)
		findprev();
	else
		findnext();
end

search() dosearch(0);
rsearch() dosearch(1);

numeric(k) return '0' <= k /\ k <= '9';

gotoline() do var k, n;
	Linemo := 1;
	n := 0;
	k := Cmd;
	while (numeric(k)) do
		n := n*10 + k - '0';
		k := con.getkey();
	end
	if (k = 'G') gotoln(n-1);
end

chword1(cm) do
	ie (cm = 'c') do
		eoword(1);
		sync();
		con.wrch('$');
		return 2;
	end
	else do
		wordright();
		return 1;
	end
end

gotomark(ln) do
	con.getkey();
	if (Marker = %1) return huh();
	goto(Marker);
	if (ln) do
		home();
		Linemo := 1;
	end
end

tomarker() gotomark(0);
tomarkln() gotomark(1);

inserts(s) return insertblock(s, str_length(s));

killblank() do var i, p, lim;
	p := Point;
	if (prev(1)) return;
	lim := textsize();
	for (i=Point, lim) do
		if (Buffer::pos(i) = '\n')
			leave;
		if (\iswhite(Buffer::pos(i))) do
			goto(p);
			return;
		end
	end
	mark();
	eol();
	setblock();
	p := p - Eblock + Block;
	edeleteblock();
	goto(p);
end

insmode(open) do var n, k, ks, hd, y0, i;
	if (Point >= textsize()) return huh();
	hd := Head;
	y0 := Ypos;
	n := 0;
	while (1) do
		killblank();
		sync();
		ie (open) do
			k := '\r';
			Iobuf::0 := 0;
			open := 0;
			y0 := y0+1;
		end
		else
			k := getstr(Iobuf);
		if (k = 3) leave;
		ks := str_length(Iobuf);
		if (ks /\ inserts(Iobuf)) do
			nospace();
			leave;
		end
		for (i=0, ks) right();
		if (k \= '\r') leave;
		n := n+1;
		if (inserts("\n")) do
			nospace();
			leave;
		end
		displine();
		_down();
		indent();
	end
	ie (n) do
		ie (hd = Head)
			refresh(y0, YMAX-1);
		else
			display();
	end
	else
		displine();
end

insbol() do
	home();
	if (iswhite(Buffer::pos(Point))) wordright();
	insmode(0);
end

delete() do
	Modified := %1;
	Marker := %1;
	movegap(Point);
	if (Egap >= SIZE) return;
	if (Buffer::pos(Point) = '\n') return;
	Egap := Egap+1;
	sync();
	println(Xpos);
end

yank(lnmode) do var k;
	k := Eblock - Block;
	ie (\k)
		return;
	else ie (k > YBUFL) do
		writeblock(Tmpfile, Block, Eblock);
	end
	else do
		movegap(Block);
		t.memcopy(Ybuf, @Buffer::Egap, k);
	end
	Ylen := Eblock - Block;
	Yline := lnmode;
end

paste(r) do
	if (Ylen = %1) return huh();
	Modified := %1;
	Marker := %1;
	if (Yline) home();
	if (r) do
		ie (Yline)
			down();
		else
			right();
	end
	ie (Ylen > YBUFL) do
		readblock(Tmpfile);
		disprest();
	end
	else do
		insertblock(Ybuf, Ylen);
		ie (Bline = Eline)
			displine();
		else
			disprest();
	end
end

deleteln(del) do var p;
	home();
	p := Point;
	mark();
	if (next(1) < 0) eol();
	setblock();
	goto(p);
	yank(1);
	if (del) do
		edeleteblock();
		disprest();
	end
end

deltoeol() do
	mark();
	eol();
	setblock();
	yank(0);
	edeleteblock();
	displine();
end

lnadjust() do var p;
	if (Linemo) do
		p := Point;
		moveto(Block);
		Block := Line;
		moveto(Eblock);
		if (next(1) = %1) eol();
		Eblock := Line;
		moveto(p);
	end
end

motion(cm, k) do var p, r;
	Linemo := 0;
	if (Mmap::k) do
		Cmd := cm;
		p := Cmds[k];
		r := call p();
		return r-> r: 1;
	end
end

delmode(cm, del) do var k, yp, hd, m;
	mark();
	k := key();
	if (del /\ k = 'd') return deleteln(del);
	if (\del /\ k = 'y') return deleteln(del);
	Nodraw := %1;
	yp := Ypos;
	hd := Head;
	m := motion(cm, k);
	if (\m) do
		Nodraw := 0;
		return huh();
	end
	setblock();
	lnadjust();
	goto(Block);
	Ypos := yp;
	Head := hd;
	Nodraw := 0;
	goto(Block);
	yank(0);
	if (del)
		ie (m > 1)
			edeleteblock();
		else
			deleteblock();
end

joinlns() do
	toeol();
	movegap(Point);
	Buffer::pos(Point) := '\s';
	right();
	mark();
	if (Buffer::pos(Point) = '\s' \/ Buffer::pos(Point) = '\t')
		wordright();
	setblock();
	Eline := Bline+1;
	deleteblock();
	disprest();
end

replace() do var k;
	sync();
	k := con.getkey();
	movegap(Point);
	Modified := %1;
	ie (k >= '\s' /\ k <= '~') do
		Buffer::Egap := k;
		con.wrch(k);
	end
	else ie (k = '\r') do
		Buffer::Egap := '\n';
		disprest();
		down();
		home();
	end
	else
		huh();
end

quit() do var k;
	if (\Modified) return %1;
	post("*!");
	return con.getkey() = '!';
end

saveblock() do var k, fd;
	if (Marker = %1) return huh();
	prompt(":W ");
	k := getstr(Iobuf);
	if (k = 3) return;
	fd := t.open(Iobuf, T3X.OREAD);
	if (fd >= 0) do
		t.close(fd);
		post("EXISTS");
		return;
	end
	ie (Marker < Point)
		writeblock(Iobuf, Marker, Point);
	else
		writeblock(Iobuf, Point, Marker);
end

loadblock() do var k, fd;
	prompt(":r ");
	k := getstr(Iobuf);
	if (k = 3) return;
	down();
	home();
	readblock(Iobuf);
	display();
end

subst(all) do var k, p, lim, pt, n, ks, kr, len;
	prompt(all-> ":%s ": ":.,'ms ");
	k := getstr(Findbuf);
	if (k = 3) return;
	ks := str_length(Findbuf);
	if (ks < 1) return huh();
	prompt("R: ");
	k := getstr(Replbuf);
	if (k = 3) return;
	kr := str_length(Replbuf);
	p := all-> 0: Point;
	len := textsize();
	lim := all-> len: Marker;
	pt := Point;
	n := 0;
	Nodraw := %1;
	while (1) do
		p := efind(Findbuf, p, lim);
		if (p < 0) leave;
		len := len + kr - ks;
		if (len >= SIZE) leave;
		pt := p;
		Modified := %1;
		movegap(p);
		Egap := Egap+ks;
		t.memcopy(@Buffer::Gap, Replbuf, kr);
		Gap := Gap+kr;
		p := p + kr - ks;
		lim := lim + kr - ks;
		n := n+1;
	end
	Nodraw := 0;
	ie (n = 0) do
		goto(pt);
		nomatch();
	end
	else do
		if (len >= SIZE) post("NO SPACE");
		if (\goto(pt)) display();
	end
end

colon() do var k;
	post(":");
	k := con.getkey();
	ie (k = 'q') return quit();
	else ie (k = 'r') loadblock();
	else ie (k = 's') subst(0);
	else ie (k = '%') subst(1);
	else ie (k = 'w') save(File);
	else ie (k = 'W') saveblock();
	else huh();
end

command(k) do var p;
	Cmd := k;
	ie (Cmds[k]) do
		p := Cmds[k];
		return call p();
	end
	else
		huh();
end

edit() do var k, q;
	q := 0;
	while (\q) do
		sync();
		k := key();
		if (\motion(k, k)) q := command(k);
	end
end

! More commands

append()   do right(); insmode(0); end
appndeol() do toeol(); insmode(0); end
change()   do delmode('c', 1); insmode(0); end
chgeol()   do deltoeol(); insmode(0); end
delback()  if (Xpos) do left(); delete(); end
delblock() delmode('d', 1);
insert()   insmode(0);
openln()   do toeol(); insmode(1); end
paste0()   paste(0);
paste1()   paste(1);
redisp()   do con.clrscr(); display(); end
setmark()  do con.getkey(); Marker := Point; end
tochar()   do sync(); findchar(con.getkey(), 0); end
ontochar() do sync(); findchar(con.getkey(), 1); end
yankblk()  delmode('y', 0);

! More motion commands

endwrd()   eoword(Cmd='c');
firstcol() do home(); wordright(); end
toeof()    do Linemo := 1; bottom(); end
topscr()   do Linemo := 1; goto(Head); end
wordfwd()  return chword1(Cmd);

init(vt52) do const _ = 0;
	Cmds := [
	! ^@        ^A        ^B        ^C
	  _,        _,        @prevpage,_,

	! ^D        ^E        ^F        ^G
	  _,        _,        @nextpage,@report,

	! ^H        ^I        ^J        ^K
	 @left,     _,        @down,    _,

	! ^L        ^M        ^N        ^O
	 @redisp,   @down,    @down,    _,

	! ^P        ^Q        ^R        ^S
	 @up,       _,        @redisp,  _,

	! ^T        ^U        ^V        ^W
	  _,        _,        _,        _,

	! ^X        ^Y        ^Z        ^[
	  _,        _,        _,        _,

	! ^\        ^]        ^^        ^_
	  _,        _,        _,        _,

	!           !         "         #
	  @right,   _,        _,        _,

	! $         %         &         '
	  @toeol,   _,        _,        @tomarkln,

	! (         )         *         +
	  _,        _,        _,        @down,

	!  ,        -         .         /
	  _,        @up,      _,        @search,

	! 0         1         2         3
	 @home,     @gotoline,@gotoline,@gotoline,

	! 4         5         6         7
	  @gotoline,@gotoline,@gotoline,@gotoline,

	! 8         9         :         ;
	  @gotoline,@gotoline,@colon,   _,

	! <         =         >         ?
	  _,        _,        _,        @rsearch,

	! @         A         B         C
	  _,        @appndeol,@wordleft,@chgeol,

	! D         E         F         G
	  @deltoeol,@endwrd,  _,        @toeof,

	! H         I         J         K
	  @topscr,  @insbol,  @joinlns, _,

	! L         M         N         O
	 @toeoscr,   _,      @findprev,  _,

 	! P         Q         R         S
	  @paste0,  _,        _,        _,

	! T         U         V         W
	  _,        _,        _,        @wordfwd,

	! X         Y         Z         [
	  @delback, _,        _,        _,

	! \         ]         ^         _
	  _,        _,        @firstcol,_,

	! `         a         b         c
	 @tomarker, @append,  @wordleft,@change,

	! d         e         f         g
	  @delblock,@endwrd,  @ontochar,_,

	! h         i         j         k
	  @left,    @insert,  @down,    @up,

	! l         m         n         o
	  @right,   @setmark, @findnext,@openln,

 	! p         q         r         s
	 @paste1,    _,      @replace,   _,

	! t         u         v         w
	  @tochar,  _,        _,        @wordfwd,

	! x         y         z         {
	  @delete,  @yankblk, @newtop,  _,

	! |         }         ~         ^?
	  _,        _,        _,        _
	];
	Mmap := packed [
	!^@  ^A  ^B  ^C  ^D  ^E  ^F  ^G  ^H  ^I  ^J  ^K  ^L  ^M  ^N  ^O
	  _,  _,  _,  _,  _,  _,  _,  _,255,  _,255,  _,  _,255,255,  _,

	!^P  ^Q  ^R  ^S  ^T  ^U  ^V  ^W  ^X  ^Y  ^Z  ^[  ^\  ^]  ^^  ^_
	255,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,  _,

	!     !   "   #   $   %   &   '   (   )   *   +   ,   -   .   /
	255,  _,  _,  _,255,  _,  _,255,  _,  _,  _,255,  _,255,  _,255,

	! 0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ?
	255,255,255,255,255,255,255,255,255,255,  _,  _,  _,  _,  _,255,

	! @   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O
	  _,  _,255,  _,  _,255,  _,255,255,  _,  _,  _,255,  _,255,  _,

 	! P   Q   R   S   T   U   V   W   X   Y   Z   [   \   ]   ^   _
	  _,  _,  _,  _,  _,  _,  _,255,  _,  _,  _,  _,  _,  _,255,  _,

	! `   a   b   c   d   e   f   g   h   i   j   k   l   m   n   o
	255,  _,255,  _,  _,255,255,  _,255,  _,255,255,255,  _,255,  _,

 	! p   q   r   s   t   u   v   w   x   y   z   {   |   }   ~  ^?
	  _,  _,  _,  _,255,  _,  _,255,  _,  _,  _,  _,  _,  _,  _,  _
	];
	Tmpfile := "ve.$$$";
	reset();

	! 			Lines,	clrscr,		clreol,		move,			rscroll,	csroff,	csron
	!			-----	------		------		----			-------		------	-----
	!Agon Light - ANSI
	con.setup([	24,		"\e[2J",	"\e[0K",	"",				"",			"",		"" ]);
	!
	!VT52/8	
	!con.setup([ 8, 	"\eH\eJ", 	"\eK", 	"\eY%yc\s%xc\s",	"\eI", 		"", 	""]);
	!
	!ie (vt52)
	!con.setup([24, 	"\eH\eJ", 	"\eK", "\eY%yc\s%xc\s", 	"\eI",		"", 	""]);
	!
	!else
	!con.setup([24, 	"\e[H\e[J", "\e[K", "\e[%yd1;%xd1H", 	"\eM", 	"\e[?25l", 	"\e[?25h"]);
end

fini() do
	t.remove(Tmpfile);
	clearmsg();
	conMove(0, YMAX-1);
	con.clreol();
	con.shutdown();
	halt 0;
end

do var b::3;
	if (t.getarg(1, File, 14) < 1) do
		t.write(T3X.SYSOUT, "Missing file name\r\n", 19);
		halt 1;
	end
	ie (t.getarg(2, b, 3) = 2 /\ \t.memcomp(b, "/5", 1))
		init(1);
	else
		init(0);
	load(File);
	display();
	edit();
	fini();
end

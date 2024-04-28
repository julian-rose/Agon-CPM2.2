T3X programming language and the VE editor
==========================================
This disc image is a small clone of VI called VE.
VE is found at: http://t3x.org/#VE

Running VE.COM from the download bundle on Agon results in control strings 
and incorrect cursor location. The terminal sequences in this binary are not 
ansi ones (they may be VT52 ones). So we need to modify the ve.t source for 
the correct terminal type and recompile it.

VE source
---------
There are two versions of VE, version 12 and version 14.
Note the VE source, VE.T, makes direct BDOS calls.
In line 56: t.bdos(6, c); which calls the t object built-in BDOS function 6 
with a parameter.
W.r.t. DRI CP/M O/S Manual v2, ch:5, Function 6 is Direct Console I/O, output 
if c<255, or input if c==255.


T3X
---
VE is written in a language called T3X, at: http://t3x.org/t3x/index.html
T3X is a family of languages. There are several language versions and tool 
versions. The nomenclature may seem confusing.

T3X/0 20a
---------
T3X0 is the newest T3X language family member.
The tool version 20a dates from 2022.
A copy is in the folder t3x0-20a.

'make' will create the T3X tcode (VM) compiler okay under cygwin.
'make triple test' as per the README results in '"Looks good!"
And then './tcvm programs/apfel' builds the apfel program. 
Similar for hello. 

tx-cpm.com included in ./t3x0-20a/bin/ [renamed T3X0.COM] runs on Agon.
(Though 'make tx-cpm' results in errors from gcc on #error definitions, so I 
can't rebuild tx-cpm.)

Building VE with T3X0-20a
-------------------------
Running './tcvm txtrm ./ve' on cygwin results in errors in line 9 module and 
(if I comment that out, then) line 12 object.
T3X0 doesn't know about the object language of T3X9 (in which VE version 12 
is written).

T3X/9 and T3X/Z
---------------
In the meantime, T3X/Z is language version T3X9 for the Z80. 
This is language version T3X9 which does know about objects and modules.
Just what we need to build VE version 12. 

The t3xz19 folder is a download of the Z80 CP/M version of the object-oriented
T3X9 version of the language, T3X/Z version 1.9. 

We can use the ./t3xz19/t.bin executable [renamed T3X9.COM] on Agon.
We can use it to buuld VE.t. 
We can build the compiler itself on Agon with 't3x9 t3x9-src', having renamed 
t.t to t3x9-src.t. So we can make changes and try them out.


Compiling VE version 12
=======================
Running T3X9.COM to compile VE.t on Agon crashes out with CP/M BDOS errors. 
(It also requires the board to be power-cycled, not just reset!)
This is due to the lack of an EOF character, at the end of VE.t

EOF ctrl-Z character
--------------------
Several tools fail to read EOF when the file just ends.
This includes the T3X compilers, and I noticed it earlier with HiTech.
The solution is to place a CTRL-Z (EOF) character after the last line.
(We need to check editors do this in evaluating them.)

In the meantime I wrote the du2c.c program to convert Dow/UNIX files to CP/M 
files by appending ctrl-z, as an exercise in porting cygwin sources to BDSC
on CP/M. The du2c.c source is included in the bdsc-work-h disc image.

Compiling VE12agon with T3X9
----------------------------
I edited a version of the VE 12 source in ve-agon.t, replacing the VT52 
escape sequences with ANSI ones.
And then used du2c to append a ctrl-z. 
Run the compiler using 't3x9 ve-agon /v' for verbose output.

Problems with VE (version 12)
-----------------------------
Now I can build ve-agon.t into VE-AGON.COM.
And we get output on the screen - which means the BDOS calls must be working.
But, the program crashes out, sometimes back to CP/M, sometimes like Z80 mode 
with AgDev.
This always happens on return from goto() function back to killblank.
Just couldn't resolve this...
Never found out why...need a debugger...need symbolic information in the 
ve12 build.


T3X/0 21b
=========
In communication with Nils, as per docs/t3x.txt, the "new" T3X/0 language as 
defined in September 2022 with Objects and Modules.
Nils put up a new version of T3X0, version 21b.
This is now in the folder t3x0-21b (TX30 2024).

Trying VE version 14 with T3X021b
---------------------------------
I can compile version 14 of VE using this tool version t3x021b.

VE14agon.t
----------
I made a version of VE14 with the Agon / ANSI escape sequences for display.
Type 't3x0v21 ve14agon.t /v' to generate ve14agon.com.
This is the version of ve that runs on the Agon Light.
For convenience, I made a copy of ve14agon.com in vi.com.

BDSC Debugger (CDB)
===================
This disc image contains the BDS C v1.6 Debugger (CDB). It is found at 
http://www.cpm.z80.de/develop.htm, under ./BDS-C/extra/CDEBUG.LBR as a 
library archive. 

(Neither the RED editor nor the CDB debugger are included on Nihirash's BDSC 
cpmc.dsk)

CDB is authored by David Kirkland. CDB is an interactive symbolic debugger
for programs written using the CP/M BD Software C Compiler version 1.6


Extracting
----------
Unpack CDEBUG.LBR on CP/M:
  i/ run 'LBREXT CDEBUG.LBR *.*'
  ii/ then 'UNCRUNCH *.cz', 'UNCRUNCH *.hz', 'UNCRUNCH *.czm', 'UNCRUNCH *.ozl', 'UNCRUNCH *.dzc'

(You can also use lar to view/unpack the content of CDEBUG.LBR, on windows/nix)

Note in addition to CDEBUG.LBR, the L2 Linker is part of the CDB release.
This consists of a single source file, l2.c, which is found in the 
separate ./BDS-C/extra/SOURCES.LBR
The file l2.c can be extracted following similar steps to i and ii above.


Manual
------
See ch:6 of BDS-C User Manual.
You can find the manual here: https://www.bdsoft.com/resources/bdsc.html

Refer to section 6.2 of the BDS C User Guide.
There are three parts to CDB. These are CDB, CDB2 and the L2 linker.


Building L2
-----------
W.r.t BDS-C User Manual section 6.2.1
To build L2 you will need chario.c
Build the L2 linker:
	cc l2
	cc chario
	clink l2 chario


Configuring CDB and CDB2
------------------------
CDB, the user interface, runs as an ordinary CP/M TPA program.
CDB2, the functional debugger, runs as an overlay in high memory; co-
resident with the TPA program being debugged. 

We can just use the extracted CDB.COM. 
This is configured assuming the CP/M BDOS starts at D000.
Agon CP/M actually starts at DCFD (or DD00-3).
So we can free-up an extra 0xC00 (3K) bytes of program space by re-
configuring CDB for a BDOS base of DC00 (and still leave a small margin).
If you are happy to used the extracted default CDB, or your programs to be 
debugged are not so large, then stop here and just use the supplied CDB.COM

Continue here to build a re-configured CDB for Agon.
To configure the high memory address for CDB2, we need to calculate CDB2ADDR.

You are supposed to compile and link the extracted CDBCONFG.C file into
CDBCONFG.COM, which is used to configure the build for CDB and CDB2.
Refer to section 6.2.2.
	cc cdbconfg
	clink cdbconfg   (or) l2 cdbconfg

CDBCONFIG configures CDB for runtime memory usage, directly modifying 
CDB.H by #defining CDB2ADDR, and outputting a MAKECDB.SUB batch file which 
can be used to compile and link the CDB executable.
  To run cdbconfig:
	cdbconfig

Calculating CDB2ADDR
--------------------
Refer to 6.2.2.2 how to calculate CDB2ADDR.
We don't need to do this; we can just take the output of running CDBCONFG.

To calculate CDB2ADDR We need to find the code size, the externals size, and 
the stack size for CDB2.
To find the code size, we can do a first compilation of CDB2 - see Building 
CDB2 below. From this, the linker reports the last program address.
Let's say the code size reported is 0x4900.
The same method is used to find the externals size.
Let's say the reported externals size is 0x980. 
David Kirkland arrives at 0x480 as a good stack size - we can make this
bigger, or reduce it at some risk. The latter might be worth trying if our
programs to be debugged will be large.
So the total CDB2 size is 0x4900 + 0x980 + 0x480 = 0x5700.
Therefore, the CDB2 overlay must origin 0x5700 bytes below the start of CP/M 
BDOS (at E500 for Agon).

For example, if BDOS starts at 0xE500,
then CDB2 must origin at 0xE500 - 0x5700 = 0x8E00.
So we calculate CDB2ADDR as 0x8E00.

Configuring Breakpoint Restart
------------------------------
CDB uses RESTART 0x6 for breakpointing by default.
You can opt to change this in CDB.H by redefining RST_NUM.
The same change must be made in L2.C.
And in DASM.CSM, at EQU RstNum.
(Make a note of the new number as you will need to supply it when compiling
programs to be debugged with -k)


Building CDB and CDB2
---------------------
On CP/M, after running cdbconfig.com, 'type makecdb.sub' to see the sequence 
of commands to compile the sources.

Building CDB
------------
Your first option is to run this batch file by typing 'a:submit makecdb.sub'
(using 'a:submit' assumes you have Nihirash's cpma.dsk, or similar, in a:)

Your second option is to type in each command line by hand (which is what 
I did):
  After defining CDB2ADDR in CDB.H, to build CDB:
	cc cdb.c -e3c00
	cc build.c -e3c00
	l2 cdb build
The value for the cdb external variables page, 3c00, is obtained by doing a 
first compilation and link of cdb, so that the linker reports the highest 
program address (say 3bf0). Then we choose the next 0x100 boundary above, to
arrive at 0x3c00.

Building CDB2
-------------
The value for the cdb2 external variables location, D700, is obtained by 
first compiling and linking the sources so that the linker reports the 
highest program addess (say 4900). The CDB2 code size is then added to 
CDB2ADDR (8E00), to give 0x4900 + 0x8E00 = 0xD700.
  To build CDB2 (refer to MAKECDB.SUB generated by CDBCONFIG):
	cc cdb2 -eD700
	cc atbreak -eD700
	cc break -eD700
	cc command -eD700
	cc print -eD700
	cc parse -eD700
	cc util -eD700
  You may optionally compile the one assembly file dasm.c into dasm.crl:
	casm dasm
	asm dasm
	cload dasm
  Or just use the dasm.crl included in CDEBUG.LBR when you unpacked it.

  To link all these together:
	l2 cdb2 dasm atbreak command break print parse util -ovl null <cdb2addr> -wa
  for <cdb2addr> = 8E00

You should end up with an executable program, CDB.COM
I have built and included a configured copy called CDB-AGON.COM
I preserved the original executable in CDB-D000.COM


Running
-------
Refer to the BDS C User Guide sections 6.4 through 6.6

Undocumented features
---------------------
To have visibility of function local variables and arguments, do not declare
the function prototypes in the source file. If you do that, the symbol table 
information is built from those prototypes instead of the actual function 
definitions. This is a bug in the compiler. The symptom is you will only
see locals and arguments in main.

Remember after you finish debugging a program, to recompile it without -k and
relink it; this removes the debug information (and insertion of rst calls).
If you try to run a debug-build program from CP/M, rather than from CDB, it
will lock up.


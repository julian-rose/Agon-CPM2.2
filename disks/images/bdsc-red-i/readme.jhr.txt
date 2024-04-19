This disc image contains the Ream Editor (RED) for BDS C v1.6,
as found in ./BDS-C/extra/RED.LBR, 
as extracted from http://www.cpm.z80.de/develop.htm
(The RED editor is not included on Nihirash's cpmc.dsk)

RED for CP/M is updated from the original Edward K. Ream program first 
published in Dr. Dobbs Journal #81 & #82, JUL/AUG 1983. The source code for 
this version of RED is part of BDS-C v1.6, released into the public domain, 
September 20, 2002.


Extracting
----------
You unpack it on CP/M,
  first using 'LBREXT RED.LBR *.*'
  then 'UNCRUNCH *.cz', 'UNCRUNCH *.hz', 'UNCRUNCH *.czm'

(You can also use lar to view the content of RED.LBR, on windows/nix)


Configuring
-----------
You are supposed to first use the extracted RCONFIG.COM to configure RED.
Refer to section 5.1.1 of the BDS C User Guide.
This sets editing preferences and hardware support for terminal functions.
RCONFIG directly modifies the .c and .h files.

I modified red6.c by hand to work on Agon terminal (ansi.sys) functions;
a copy of this modified version is named red6agon.c.
If you don't want to run RCONFIG, or don't complete it (like me),
you can just replace red6.c by my red6agon.c
This is what I built, with no other changes.
(If you run RCONFIG and overwrite red6.c, you can still copy back red6agon.c)


Building
--------
On CP/M, 'type cred.sub' to see the sequence of commands to compile the sources.
You can run this batch file using 'a:submit cred.sub',
or you can type in each compilation line by hand (which is what I did).
(I refer to 'a:submit' assuming you have Nihirash's cpma.dsk in a:)

You can 'type lred.sub' to see the commands to link the compiled sources,
using the CLINK linker.
You can run this batch file using 'a:submit lred.sub',
or type the command in by hand (as I did).


Manual
------
See ch:5 of BDS-C User Manual.
You can find this here: https://www.bdsoft.com/resources/bdsc.html

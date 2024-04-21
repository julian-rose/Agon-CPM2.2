BDSC-RED editor
===============
This disc image contains the Ream Editor (RED) customised for BDS C v1.6.
It is found at http://www.cpm.z80.de/develop.htm, in ./BDS-C/extra/RED.LBR
as a library archive. 

(Neither the RED editor nor the CDB debugger are included on Nihirash's BDSC 
cpmc.dsk)

RED for CP/M is updated from the original Edward K. Ream program first 
published in Dr. Dobbs Journal #81 & #82, JUL/AUG 1983. The source code for 
this version of RED is customised for BDS-C v1.6, and was released into the 
public domain on September 20, 2002.


Extracting
----------
Unpack it on CP/M:
  i/ run 'LBREXT RED.LBR *.*'
  ii/ then 'UNCRUNCH *.cz', 'UNCRUNCH *.hz', 'UNCRUNCH *.czm'

(You can also use lar to view/unpack the content of RED.LBR, on windows/nix)


Manual
------
See ch:5 of BDS-C User Manual.
You can find this here: https://www.bdsoft.com/resources/bdsc.html
Refer to section 5.1.1 of the BDS C User Guide.


Configuring
-----------
You are supposed to first use the extracted RCONFIG.COM to configure RED.
RCONFIG sets editing preferences and hardware support for terminal functions,
by directly modifying .c and .h source files (in particular red1.h and red6.c)

I modified red6.c by hand for the Agon terminal (ansi.sys) functions;
a copy of this modified version is named red6agon.c.
You can achieve a similar result by using RCONFIG.
If you don't want to run RCONFIG, or don't complete it (like me),
you can just replace red6.c by my red6agon.c and build with that.
This is what I built, with no other changes.
(If you run RCONFIG and overwrite red6.c, you can still copy back red6agon.c
if your configuration doesn't work.)


Building
--------
On CP/M, 'type cred.sub' to see the sequence of commands to compile the sources.
Your first option is to run this batch file by typing 'a:submit cred.sub'; but 
I found this doesn't work very well on Agon CP/M
(using 'a:submit' assumes you have Nihirash's cpma.dsk, or similar, in a:)
Your second option is to type in each compilation line by hand (which is what 
I did).

You can 'type lred.sub' to see the commands to link the compiled sources,
using the CLINK linker. Or similarly 'type l2red.sub' for the L2 linker.
You can run either batch file using 'a:submit lred.sub' or 'a:submit l2red,sub'
or you can enter the commands by hand in sequence (as I did).

You should end up with an executable program, RED.COM

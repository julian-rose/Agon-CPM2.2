WordMaster 1.07 for CP/M-80 
Released in 1981 by MicroPro
For CP/M-80

WordMaster is a text editor for CP/M-80. 
It pre-dates WordStar, and is designed as a text editor rather than a full blown word processor. 
It has three user modes, Video Mode, Command Mode and Insert Mode.
See sheets 105-107 in MicroPro Word-Master Operators Manual for the mode and commands summary.


How to build the patch version for Agon
---------------------------------------
WM-1.07.COM is generic for CP/M-80 systems.
This is found at: http://www.s100computers.com/Software%20Folder/Assembler%20Collection/Assembler%20Collection.htm
The generic program uses a video screen mode which is incompatible with Agon.
The file WM3.HEX is used to patch the generic image to one with a (vt100) screen mode which works (ish) with Agon.
This patch is found at: http://canal.chez.com/CPM/wm3.htm

I applied the patch as per the Micropro Word Master Operators Manual, sheet 107 (just after the command summary).
In summary:
 - a:stat wm.com   and note the number of records, so you know how much needs saving
 - a:zsid wm.com   start the symbolic debugger on wm.com (wm-107.com)
   i wm3.hex       name the input file wm3.hex
   r               read the input file into memory (executes record reads to overwrite the wm.com image in RAM)
   g0              return to CP/M
 - save 38 wm-agon.com   save the image in RAM to file (38 is the hex value for the number of records)
 - a:stat wm-agon.com    to check the requisite number of records have been saved

A copy of zsid.com is found on my updated cpm+utils-a disk image; you could use DDT instead, supplied by Nihirash in cpma.dsk

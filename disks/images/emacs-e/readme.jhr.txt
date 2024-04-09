This is uEmacs 3.6 for CPM-80.
The base of this port is by Ralph Betza.
It is found at: http://zimmers.net/anonftp/pub/cpm/editors/
However, this code is not Ralph's, but an interim modified version, perhaps 
by Zimmer, targetting the Commodore 128.

This base is patched by me to the Agon Light, using ZSID to edit the COM file 
directly. On my disk image you will find a ready-to-run editor named ME.COM 
(identical to the file named ME-AGON.COM). You will also find a copy of the 
Commodore 128 version in ME-ORIG.COM (although, I changed it using METERM, to 
no use).

If you just want to use it, you can find the key mappings in either of the
files:
	- mapping.txt
	- zmecmds.lst


Working from the base (to retrace what I did)
---------------------------------------------
Extract the Z80emacs.pma archive using PMARC.
See the readmeb.1st.

When CP/M for Agon starts, it sets the VDP to terminal mode (VDU 23, 0, FF).
So we must configure z80emacs for an ansi terminal.
This requires character code sequences for Delete to EOL, CLS, and POS to 
locate the cursor.

To configure Z80emacs, first run MECONFIG.COM; this generates a maptable.zme 
file.
Then use METERM.COM (which reads maptable.zme) to configure uEmacs.
For Agon we choose ANSI terminal.
The ANSI escape sequences here: https://en.wikipedia.org/wiki/ANSI_escape_code.
They are: EOL = ESC [ 0 K            -- that is 4 byte values
          CLS = ESC [ 2 J            -- also 4 bytes
          POS = ESC [ 0 0 ; 0 0 H    -- 8 byte values, the zeros will become 
                                        row tens / units, and col tens / units

However, this is not enough, and why I patched ME.COM using ZSID. The 
ansimove() code for POS needed reworking for Agon (and any modern ANSI 
terminal). This is done on CP/M using ZSID to modify the ME.COM assembly in-
place. To see the changes I made, you can:
	a/ use a hex file editor (on your PC) to compare ME-AGON.COM with ME-ORIG.COM
 	   But in essence the changes are:
	   - bashing out the correct POS sequence
	   - offsetting the uEmacs (0,0) origin to the Agon VDP (1,1) origin
        b/ Examine the edited telansi.c, to see what is needed. This served as my
           basis for editing ME.COM file in-place using ZSID.

Bugs
----
1. Naming an existing file to edit from the CP/M command line, ME <file>,
   results in the first line being displayed in the status area. This is
   a shortcoming in the uEmacs program which assumes the cursor is HOMEd
   before the file is loaded. The code should in fact HOME the cursor on
   startup. This may be due to differences in either the CP/M code or the
   display processor on the Commodore 128 versus Agon.
   Workaround: 1/ Start ME without naming a file, then issue a file read
   command; 2/ Try scrolling down then up again to refresh the editor display.

2. METERM reports the address of ansimove incorrectly. This probably comes from
   the Zimmer changes. You can find the correct memory locations of symbols in
   the file me.sym, which is the linker output from compiling ME-ORIG.COM.

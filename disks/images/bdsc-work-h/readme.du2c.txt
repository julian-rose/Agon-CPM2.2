du2c
====
Convert a DOS format (CR LF) or a UNIX format (LF) file to a CP/M one.
CP/M format retains the CR LF or LF combination as is, along with all other 
characters, and appends a CTRL-Z character at the file end.

This serves several purposes:
1/ Several CP/M editors (like microEmacs) need a ctrl-Z EOF marker to work 
   correctly.
2/ Likewise several CP/M tools require an EOF marker to read files correctly.
3/ It serves to see the conversion from an original cygwin source into a BDSC 
   source.
4/ It serves as a testbed for the CDB CP/M debugger. 

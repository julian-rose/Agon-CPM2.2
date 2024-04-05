This disc contains an unpacked and configured zde16 editor for the Agon Light2.
I still am looking for vi for CP/M, since my fingers don't remember ED or EDLIN (on drive A:)
I also find a:te.com doesn't work that well for me.
zde16 uses a small number of wordstary / emacsy commands, which are picked up quickly enough.

If you prefer to start from scratch, all you need is the PMARC utility and zda16.pma
zda16.pma is an PMARC archive
You can use the PMARC utilities to extract it
Use: pmext zde16.pma <drive>   where <drive> might be e:

Once unpacked, configure zde16 for the correct terminal.
Use: zdenst16 zde16
     t   to configure the terminal type
     l   to list available terminal types
         I chose ansi standard which works with my Agon Light2
     ESC+s to save
     ESC+q to quit

You are now ready to use the configured editor, zde16.com

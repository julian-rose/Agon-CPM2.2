! T3X core module for CP/M on the Z80
! Nils M Holm, 2022, 2023
! Public Domain / 0BSD license

module t3x;

 public const	SYSIN = 0,
		SYSOUT = 1,
		SYSERR = 2;

 public const	OREAD = 0,
		OWRITE = 1;
		! ORDWR = 2,
		! OAPPND = 3;

 ! public const	SEEK_SET = 0,
 !		SEEK_FWD = 1,
 !		SEEK_END = 2,
 !		SEEK_BCK = 3;

 public inline	bpw(0) =	[ 0xc3, 0x1d, 0x01 ],	! jp 0x011d
		newline(1) =	[ 0xc3, 0x20, 0x01 ],	! jp 0x0120
		memcomp(3) =	[ 0xc3, 0x23, 0x01 ],	! jp 0x0123
		memcopy(3) =	[ 0xc3, 0x26, 0x01 ],	! jp 0x0126
		memfill(3) =	[ 0xc3, 0x29, 0x01 ],	! jp 0x0129
		memscan(3) =	[ 0xc3, 0x2c, 0x01 ],	! jp 0x012c
		getarg(3) =	[ 0xc3, 0x2f, 0x01 ],	! jp 0x012f
		open(2) =	[ 0xc3, 0x32, 0x01 ],	! jp 0x0132
		close(1) =	[ 0xc3, 0x35, 0x01 ],	! jp 0x0135
		read(3) =	[ 0xc3, 0x38, 0x01 ],	! jp 0x0138
		write(3) =	[ 0xc3, 0x3b, 0x01 ],	! jp 0x013b
		! seek(3) =	[ 0 ]
		rename(2) =	[ 0xc3, 0x3e, 0x01 ],	! jp 0x013e
		remove(1) =	[ 0xc3, 0x41, 0x01 ],	! jp 0x0141
		bdos(2) =	[ 0xc3, 0x44, 0x01 ],	! jp 0x0144
		bdoshl(2) =	[ 0xc3, 0x47, 0x01 ];	! jp 0x0147
		! trunc(1) =	[ 0 ]
		! break(1) =	[ 0 ]

 public create(s) return open(s, OWRITE);

end
 =	[ 0xc3, 0x47, 0x01 ];	! jp 0x0147
		! trunc(1) =	[ 0 ]
		! break(1) =	[ 0 ]

 public create(s) return open(s, OWRITE);


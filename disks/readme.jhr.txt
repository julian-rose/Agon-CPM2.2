1. Adding disk images to Agon CP/M
----------------------------------
Images a-g and p are downloaded from nihirash github
I created images/j/ to contain scott adams adventure games

Included is a test program, RNDTST.ASM compiled to RNDTST.COM.  It
reads the R register and displays the raw contents as a decimal
number.  If the results don't change at all (typically always zero)
or are above 128 all the time, then this system will not be able to
run ADVENTUR.COM.

I downloaded cpmtools from https://github.com/lipro-cpm4l/cpmtools
I built and installed cpmtools under cygwin, and added cygrive/c/use/local/bin to the windows path.

Run ./build.sh from a cygwin shell to create the dsk images (from blank nihirash format template.dsk).


2. Sorcerer images
------------------
To create a new raw blank disk for the Exidy Sorcerer:
	dd if=/dev/zero of=sorcerer.dsk bs=308k count=1
where 308k is the product of diskdefs cda1 seclen * tracks * sectrk.
Then format the disk:
	mkfs.cpm -fcpt1 sorcerer.dsk
This reads the cpt1 configuration data from diskdefs to format a blank disk (which serves as a template in ./build.sh).
To include the cpm/m bootable files use the -b option to name them.
As yet I cannot find Sorcerer cp/m binaries to use with -b.

[I copied the first blocks from C:\Users\jrose\tech_stuff\retro\Exidy\disks\liaquay exidy-sorcerer\CPMdisk1\CPMdisk1.raw to create sorcercpm.dsk. That should be bootable on the Sorcerer, but won't work with the emulator - see C:\Users\jrose\tech_stuff\retro\Exidy\disks\liaquay exidy-sorcerer\README.1st]

You can check the new file system using:
	fsck.cpm -fcpt1 sorcerer.dsk
Then based on ./build.sh, do
	cp sorcerer.dsk cpmh.dsk to create a blank formatted disk image
	cpmcp -f cpt1 cpmh.dsk images/h/*.* 0:
to add files from images/h/ to the blank disk cpmh.dsk

Refer to: https://www.mankier.com/1/mkfs.cpm or https://sizeof.cat/post/cp-m-development-setup/


3. cpmcp incantation
--------------------
To copy files from SD-card (mounted on cygwin /cygdrive/d/) to my PC working directory:
cpmcp -fnihirash /cygdrive/d/cpm/cpmn.dsk 0:editansi.com images/nevadit-n/editansi.com


# CP/M 2.2 for Agon Light (more disk images)

A fork of Nihirash's Agon-CPM2.2 project, with more CP/M disk images.<p>

Disk images in the playpen branch should be considered under evaluation, for potential inclusion in the main branch.<br>
You can get all the images as a whole, or you can take each disk image individually.<p>

Games:<br>
<ul>
  <li>adventure-J:   Scott Adam's adventure games, and a version of startrek.bas edited for the Exidy 
                     Sorcerer.</li>
  <li>creative-computing-games-g:  Dave Ahl's 101 BASIC games published by Creative Computing back in
                     the late 70s</li>
</ul>

Editors:
<ul>
  <li>bdsc-red-e:    RED editor bundled with BDS C 1.6, configured for Agon.<br>
                     Interesting because, suitably configured and linked, this may interwork with BDSC compilation error reporting.</li>
  <li>emacs-e:       Micro Emacs 3.6 for CP/M-80. Patched for Agon.</li>
  <li>nevadit-e:     Nevada Edit 3.0, a programmer's editor. Patched for Agon.</li>
  <li>vedit-e:       V-Edit Plus 2.33, a programmer's editor with a simple visual editing mode, and a 
                     rich command mode. Patched for Agon.</li>
  <li>vi-j:          VE (Vi-like Editor) version 14; a minimal version of VI, patched for Agon. 
                     Written using the T3X progamming language. See the readme for more.</li>
  <li>wordmaster-e:  MicroPro Word Master text editor with video, command and insert modes. Patched 
                     for Agon. Interesting as it predates Wordstar.</li>
  <li>ZDE-e:         ZDE programmers editor for CP/M-80; a simple programmers editor. 
                     Uses a small number of wordstary / emacsy commands. 
</ul>

Programming:
<ul>
  <li>bbc-basic3-i:  R. T. Russell's original BBC BASIC 3.0 (you will need to run CONVERT.COM on .bas 
                     files to create .bbc files)</li>
  <li>bdsc-cdb-d:    CDB, an interactive symbolic breakpointing debugger for BDS C 1.6, configured for
                     Agon. See also bdsc-work-h du2c.c as a demo with CDB.</li>
  <li>bdsc-work-h:   The minimal set of files (to do hello world) with Leor Zolman's BDS C compiler 
                     (K&R)</br>
                     Includes du2c.c, a utility to add a CP/M EOF character to the end of files, 
                     enabling them to be properly read by various editors and tools. 
                     Interesting in its own right to see the port from cygwin (gnu) to CP/M (bdsc). 
                     Edited to work with CDB.</li>
</ul>

Utilities:
<ul>
  <li>pmarc-n:       PMARC archiver for uncompressing PMA archive files. Could be added to startkit, 
                     but from a different source.</li>
  <li>startkit-n:    Utilities to uncompress archive files; CP/M software was often compressed for 
                     exchange on bullet-in boards</li>
  <li>supersoft-util-n: File (and System) Utilities from Supersoft. Read the help files (esp. before using
                     disk.com). (I removed non-functional programs, like cpu.com and versions of diff.com, 
                     that didn't run on Agon).</li>
</ul>

Updates to Nihirash's disk images:<br>
<ul>
  <li>cpm+util-a:    Added ZSID, the later Digital Research Symbolic Debugger for Z80.</li>
</ul>

On the SD-card each disk image is given a unique drive letter, in the range cpma..cpmp<br>
To accomodate an increasing number of disk images, I have renamed Nihirash's images and given each ./disks/images/ a meaningful name. For example, cpma.dsk has become cpm+util-a.dsk<p>

To build all the disk images, cd into ./disks and run ./build.sh; I do this using cygwin (with cpmtools installed).<br>
(If you get an error running build.sh, you likely need to do a dos2unix conversion on ./diskdefs.)<br>
Note the build.sh script takes only the last letter of the disk image name (as in j from ./images/adventure-j) to generate an image (as in ./cpmj.dsk).<br>
There are several disk images ending with the same letter, in which case later built images will overwrite like-named ones.<p>

If you want to build an individual image, rather than run ./build.sh, (assuming cpmg.dsk is to be built) do:
<ol>
  <li>rm cpmg.dsk</li>
  <li>cp template.dsk cpmg.dsk</li>
  <li>cpmcp -fnihirash cpmg.dsk images/creative-computing-games-g/*.* 0:</li>
  <li>cpmls -fnihirash cmpg.dsk</li>
</ol>
The second step creates a blank formatted 8MB disk image (using nihirash formatting - see diskdefs). The third step copies the content of ./images/creative-computing-games-g/ to the blank disk. The last step lists the content of your built disk image.<p>

You can read a bit more in ./disks/readme.jhr.txt on creating disk images.<p>

You can copy your choice of disk images to a sub-directory, e.g. ./cpm, on your SD-card; along with the cpm.bin image in the same sub-directory.<br>
(You can download Nihirash's cpm.bin image from the zip file in https://github.com/nihirash/Agon-CPM2.2/releases/tag/1.1.)<br>
At runtime, from the MOS prompt: cd cpm; load cpm.bin; run.<br>
And from within cpm: cd G: for drive g, and dir to see the content.<p>

Everything below is as-is from Nihirash's project. Including the request for coffee.<p>

## Nihirash Readme

[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/D1D6JVS74)

This is a port of CP/M 2.2 for Agon Light(and Agon Light 2) that doesn't require reflashing VDP.

It starts directly from MOS as application(exit only via RESET) and uses MOS's filesystem drivers for working with drive images.

![GIF screen capture](doc/cpm.gif)

## Usage

Create some directory on Agon's SD card, put `cpm.bin` and disk images(**TO THE SAME DIRECTORY**) that will be named `cpma.dsk`, `cpmb.dsk` etc, drives from `A:` to `P:` are supported.

Load and run binary and you'll be in CP/M's wonderland.

## Working with images

I'm using [cpmtools](https://github.com/lipro-cpm4l/cpmtools) for working with CP/M drive images.

Disk definition nihirash can be copied from `diskdefs` file (in `disks` directory) for use with cpmtools.

For new drives better use `template.dsk`(make copy with proper naming and copy your own files to it).

## Development

Make sure that you **really want it**.

Build process have 2 stages. Building CP/M itself(`sources` directory) - you'll need [sjasmplus](https://github.com/z00m128/sjasmplus). As result - you'll get `cpm.sys` file that should be placed to directory with bootstrap source.

Bootstrap can be built with [ez80asm](https://github.com/envenomator/agon-ez80asm). Better use latest version.

As result - you'll got `cpm.bin` executable that will include bootstrap and system both.

## Important remarks

BIOS and CP/M core should be aligned by 256 bytes.

If you'll make some changes in code - make sure that bios starts from 0xXX00 address. In other case - you'll die until you find issue why software got broken.

All terminal issues caused by current VDP implementation. They can be fixed soon.

## Licensing

CP/M itself [opensourced](https://www.theregister.com/2001/11/26/cp_m_collection_is_back/) and free for redistribution.

My parts of code licensed as usual under [Coffeeware License](LICENSE).

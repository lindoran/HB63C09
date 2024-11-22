# TEST PROGRAMS

These Files are test programs for loading with srec (s19) or iHex (hex) into a serial monitor for testing

<pre>
hello.asm   -   simple hello world program, you can compile it with:  
                lwasm hello.asm -lhello.lst -fsrec -ohello.s19
                paste into a monitor after using the Load function Both mon09 and assist9 
                this is 'L'

                once loaded - type G 1000 to jump to the code in memory
                you can also dump the code from the monitor in mon09 with
                DM 1000,1050 

                It should output somthing like: 
                * DM 1000,1050
                1000  8E 10 26 A6  80 27 08 BD  10 1A B7 A0  01 20 F4 8E    ..&..'....... ..
                1010  A0 3E 86 FF  A7 84 86 01  A7 84 34 02  B6 A0 00 85    .>........4.....
                1020  02 27 F9 35  02 39 0A 0D  0A 0D 48 65  6C 6C 6F 2C    .'.5.9....Hello,
                1030  20 57 6F 72  6C 64 21 0A  0D 0A 0D 00  13 24 4A 9D     World!......$J.
                1040  9B BB 00 08  BF 8B 2C 82  84 6B 31 EC  86 DA AF 83    ......,..k1.....
                1050  FE F9 3E B3  99 98 AE 63  EA AC F3 E9  03 8B F4 36    ..>....c.......6

                keep in mind the program's last memory location is 0x103B so some of the 
                trailing data could be different.

                Note that if you load the .hex in mon09, after it finishes loading
                there is a bug that requires you to press enter, as the end of file
                detection requires a CLRF (which is not always there in the file.) 
                this behavor does not happen with the srec (s19) file.


</pre>

These files are for the Assist09 Monitor - not MON09, to use please select combo in vBIOS as your start up 
enviornment.

The source code is provided in C to remove complexity --- ultimately its most likely better to write these routines 
in assembly but I've used pointers here so that its obvious how to write to a LBA volume.

to build these you need cmoc at: http://perso.b2b2c.ca/~sarrazip/dev/cmoc.html

which in turn requires: LWTOOLS at http://www.lwtools.ca/

<pre>
filetest.c  -  loads the data into memory from the first sector of Disk DS0N00.DSK
               then the data is modified by adding the string "This is a test!" preceeding
               the header data from the file (this may make the file unusable if it contains data, 
               you would like to use) 
               There is a blank DS0N00.DSK in this directory

blank.c      - This just writes 00's to all 8MB of the disk volume (this takes quite a long time!)
</pre>

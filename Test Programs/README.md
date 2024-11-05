# TEST PROGRAMS

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

These Files are to be copied to the SD Card ROOT.  Format the SD card as a FAT32 volume.
this uses the arduino SD.h so card limitations are as such.

see https://docs.arduino.cc/libraries/sd/


<pre>

A9FLEX.BIN  --  FLEX 9, will bootsrap from FLPY00.DSK.  these files are 
                simply 80 track, 20 sector volumes.   I have included 
                4 image disks FLPY00 - FLPY03.   For now there is no
                newdisk routines but they are in the works.

                assist09 is the monitor here, its small and fits over 
                flex in memory.  i've changed the workspace so when you
                drop to monitor it does not touch flex (including the 
                stack and system page)  I thought that might be useful 


ASSIST09    --  Original Assist09 bin, sources have been made to comple
                by digicoolthings using asm6809.  places the work area
                for assist09 at the top of the 40k continuous space.

BBCBASIC    --  dominic beasly's bbc basic.  it needs work but is working.

BIOS        --  this is a combination rom project that was started by johnny 
                quest.  it includes a floating point basic that was made to 
                compile using asm6809 by digicoolthings, and mon09 by dave
                dunfield.  it's useful but does not natively support disk
                commands

COMBO       --  jeff tranters combination rom, this has been set up for the 
                hb63c09 memory map.

</PRE>
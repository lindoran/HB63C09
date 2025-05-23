# HB63C09M

This is the repository of the HB63C09M Home brew computer.

This is a 10x10 fully functioning HD63C09 Single board computer in the form factor (mostly)
of the Z80-MBC2.

This project uses off the shelf IC's and parts with the only legacy part being the HD63C09.

Gerbers are now released! please see the Design files folder,  This is what is being called the "PRE-RELEASE" edition.  It is ready though really geared towards software folks who might be interested in porting different platforms to the PCB.

A few provisos: 

Currently the board can boot FLEX, with a small IO package and interact with filesystem images, as well as a well rounded MON09 monitor boot rom as well that has access to a floating point basic.  There is also an implementation of BBC Basic IV.  The board runs at 20 Mhz and shares that clock between the IO contoler which is an atmega32a and the 63C09 CPU which is running at an E strobe of 5 Mhz.   All of the chips in the board can be programed with a very low cost ICSP programer such as the USBAsp Or the USBTiny programmer and once programmed all updates can be done via a USB cable using USB to TTL.  The board uses a Simple SD module for online storage and the system roms are loaded into the 128K memory module at boot time, the aditional ram is availible to the CPU through 16K banks.


what is currently needed is help developing ports to Nitros09, Cubix and more. If you think that's you, but the prospect of soldering the board from scratch is a hold up, I have a few pre-production boards left I can assemble and you can keep if you'll help with porting a platform to the computer.

you can find a significantly updated Programmers reference in the documents folder, if interested.

4.2 Is the most recent version of the PCB and is verified to work.

You can find more details at: 
https://hackaday.io/project/193108-hb63c09m-mini-itx-63c09-form-factor-computer
-and- 
https://www.hackster.io/z80dad/hb63c09m-mini-itx-hd63c09-avr-based-homebrew-computer-34904b

Additionally this project is sponsored by PCBWay.com 

PCBWay handles all the prototyping costs for the HB63C09M Please check out their website for PCB Fabrication, CNC Mechineing and 3D Printing.

http://pcbway.com

Thank you PCBWay!

PCB and hardware under the CERN-OHL-P V2 Licence

Software is under the GPL 3.0 Licence, Libraries and additional attribution can be found in the "attribution" folder under a specific folder, where they can not also be called out inline.

This Repository uses the floppy emulation code from the Z80-MBC2 Project by Fabio Defabis
this code is slightly modified to fit the differing architecutre, however the original code
is (C) Fabio Defabis under the terms of the GPL.

You can find details about the Z80-MBC2 here:
https://github.com/SuperFabius/Z80-MBC2 -or-
https://hackaday.io/project/159973-z80-mbc2-a-4-ics-homebrew-z80-computer


(C) 2024 David Collins
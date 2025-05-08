# The Flex ROM

The flex system rom contains the flex console and disk io package, as well as assist09 
as a system monitor.  The system will boot to flex but can drop back to the monitor if 
the user chooses to do so with the MON command.  It will always jump to the cold start 
location for assist09 and reformat the system area documented below under RAM usage.

The two systems are set up to be independant so that debugging of flex can be done without
touching the stack within flex.

The fre area between the IO range and the upper bank B000 - BFFF is the scratch area for
assist09, though it only uses a small poriton of the memory at boot time, this space can 
be used for other things if needed durring troubleshooting.

Flex loads from the `FLEX.SYS` file off of the disk using QLOAD which loads from the boot
sector on the first drive FLPY00.DSK.  `FLEX.SYS` is basically just `FLEX.COR` as the
system loader in the controller builds out the top 16K from the SD card.

This allows the flex package to be customized without rebuilding the disk images, which 
requires rebuilding them from scratch.


## Memory map (16K ROM):

ASSIST09      `$F800 - $FFFF`
DISK IO.	  `$DE00 - $DEB8`
CONSOLE 	  `$D3E5 - $D39A`

The remaining space is staged with `0x00` and will be filled in by QLOAD

The work area for assist09 was modified to eliminate touching flex when dropping to flex.
to return to flex from the monitor you can jump to 0xCD00.

There was not enough space to include the debugger and the dissasembler, though similar tools 
could be loaded from FLEX as well.
  
RAM usage:

FLEX		  $C000 - $E000 -- See the flex memory map
				(found in the flex adaptation guide)
ASSIST09      $BEAB - $BF00 -- (below the 255 byte system page)


<PRE>
------------------------------------------------------------------------

ASSIST09 Command List:

	B (reak)	 <cr>	- list break points
			 NNNN	- insert break point
			-NNNN	- delete break point
			-	- delete all break points

	C (all)		 <cr>	- call routine at PC as subroutine
			 NNNN	- call routine at NNNN as subroutine

	D (isplay)  NNNN NNNN	- display range mod16
			 NNNN	- display 16 bytes [NNNN]mod16
			 M	- display 16 bytes [M]mod16
			 P	- display 16 bytes [PC]mod16
			 W	- display 16 bytes [W]mod16
			 @	- following NNNN, M, P, or W displays
				  16 bytes indirectly

	E (ncode)	Encodes a 6809 postbyte based on the
			addressing mode syntax as follows:

				R = X, Y, U, or S

				Direct Addressing Modes
				------ ---------- -----

				     ,R		A,R
				    H,R		B,R
				   HH,R		D,R
				 HHHH,R		,-R
				   HH,PCR	,--R
				 HHHH,PCR	,R+
						,R++

				Indirect Addressing Modes
				-------- ---------- -----

				    [,R]	[A,R]
						[B,R]
				  [HH,R]	[D,R]
				[HHHH,R]
				  [HH,PCR]	[,--R]
				[HHHH,PCR]
				[HHHH]		[,R++]

	G (o)		<cr>	- Go to PC
			NNNN	- Go to NNNN

	L (oad)		Load a S1-S9 format data

	M (emory)	NNNN	- Display memory data at address NNNN
			<cr>	- Terminate memory function
			/	- Display current byte with address
			<lf>	- Display next byte with address
			^	- Display previous byte with address
			,	- Skip byte
			SPACE	- Display next byte

			The / may be used as an immediate command.

			After any of the display commands the memory contents
			may be altered by inputting a valid hex number or
			ascii 'string' enclosed by single quotes (').

	O (ffset)  NNNN NNNN	- Calculate the two and/or three byte offset

	P (unch)   NNNN NNNN	- Punch a S1-S9 format data

	R (egister)	Display 6809 registers and allow changes

			<cr>	- terminate command
			SPACE	- terminate value input or
				  skip to next register
			,	- terminate value input or
				  skip to next register

	V (erify)	Verify memory against S1-S9 format data

	W (indow)	NNNN	- Define a display window for the
				  D and M commands.

	Ctrl-X will abort any command.

    </PRE>
<H2> Updated Combo Rom </H2>

<p>
This is Jeff Tranters Combination rom with a updated program area to allow for more contiguous RAM Space between the top of basic and the 
bottom of the machine language monitor. 
</p>
<p>
All of this (except were its not / IE Microsoft / Motorola -- see sources ) is copywrite Jeff Tranter and released under the apachie licence (see licence in this folder.) 
</p>

<PRE>
This is a variant of firmware that combines both Microsoft BASIC and
the ASSIST09 monitor into one ROM. It also includes my disassembler
which adds a new monitor U command and trace function which adds a new
monitor T command.

It comes up in ASSIST09. You can start BASIC by running "G C000".

Memory map (16K EPROM):

BASIC         $C000 - $E3FF (9K)
DISASSEMBLER  $E400 - $EFFF (3K)
TRACE         $F000 - $F8FF (2K)
ASSIST09      $F800 - $FFFF (2K)

*** Updated ***
  The Ram usage was changed to allow more free RAM as a single block between BASIC and ASSIST09
  
RAM usage:

BASIC         $0000 - $0178
DISASSEMBLER  $9E2A - $9E36
TRACE         $9E3A - $9E56
ASSIST09      $9EAB - $9F00 -- (below the 255 byte system page)
        
ASSIST09 requires an offset -5955 below its ROM location which is 
coded to line 8485 of the monitor source under variable RAMOFS.

The other two added functions are ORG statements at the beginning
segments.

Note: there is approximately a 4 byte pad between sections, this 
seemed smart and as Jeff had already done this it was easier to 
do the math for the calculated area moves.

LICENCE: https://www.apache.org/licenses/LICENSE-2.0 (were applicable / DISSASEMBLER & TRACE) 
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

	T (race)	NNNN	- Trace execution from address NNNN.

	U (nassemble)	NNNN	- Disassemble memory from address NNNN.

	V (erify)	Verify memory against S1-S9 format data

	W (indow)	NNNN	- Define a display window for the
				  D and M commands.

	Ctrl-X will abort any command.
</PRE>

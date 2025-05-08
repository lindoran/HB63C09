;This is the flex boostrap LOADER program for the HB63C09M.
;this simply switches to the first sector of the volume 00 and loads 256 bytes
;at track 0, sector 1. Since this is HB63C09 "aware" its refrenceing track 1 as 
;0 as the controller uses 0 and not 1 to refrence the first sector on the track.

	INCLUDE "./console.sym"

;equates

FLPSEL 		EQU	$A006			;Select a floppy
FLPTRK		EQU	FLPSEL+1		;Select a track
FLPSEC		EQU	FLPSEL+2		;select a sector
FLPREA		EQU	FLPSEL+3		;read a sector
FLPSTA		EQU	FLPSEL+4		;read floppy status register


LDAREA		EQU	$C100			; LOAD THE BOOTSTRAP TO HERE.

; rom code starts here: 

		ORG	$F700			; JUST ABOVE THE FLEX OS IN RAM AREA



STRAP		CLRA				; load zero to zero out the drive location
		STA	FLPSEL			; select drive 0
		STA	FLPSEC			; select sector 1, loads 0 which the controller expects for 1
		STA	FLPTRK			; select track 0
		LDB	FLPSTA			; CHECK FOR ERRORS
		BNE	STRAP2			; IF THERE WAS AN ERROR JUMP.

;; DRIVE AND LOACITON CORRECTLY SEEKED TO 

		CLRB				; START COUNT FOR SECTOR READ
		LDX	#LDAREA			; LOCATION TO STORE BOOT PROGRAM TO.
STRAP1		LDA	FLPREA			; READ A BYTE
		STA	,X+			; STORE IT AND INC INDEX
		DECB				; INCREMENT THE COUNTER
		BNE	STRAP1			; LOOP BACK IF NOT ZERO
		LDA	FLPSTA			; CHECK FOR ERRORS
		BEQ	STRAP3			; IF NO ERRORS FINISH
STRAP2		LDX 	#ESTRING		; PRINT THE ERROR STRING
		BSR	ERPR			; PRINT THE ERROR
		JMP	[MONITR]		; jump to assist09 

STRAP3		
		LDX	#SSTRING
		BSR	ERPR			; print the succsess string
		
		JMP	LDAREA			; BOOT FLEX FROM DISK.


ERPR		LDA	,X+			; get character from string
		BEQ	DONE
		JSR	[OUTCH]			; Send the character
		BRA	ERPR			; keep printing
DONE		RTS				

ESTRING 	FCC 	"FLEX: Error, media not loading"
		FCB	$0D,$0A,$00

SSTRING		FCC	"FLEX: Boot Sector Loaded"
		FCB	$0D,$0A,$00	
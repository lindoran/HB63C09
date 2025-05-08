;; drive emulation drivers
;; This interfaces fabio difabis's IOS based drive emulation 
;; as modified for the HB63C09M SBC

;; macros

;; Set interupts
SEI     MACRO           
        ANDCC   #$EF
	ENDM

;; clear interupts 
CLI     MACRO
        ORCC    #$10
	ENDM

;; set carry flag
SEC	MACRO
	ORCC	#$01
	ENDM
CLC	MACRO 
	ANDCC	#$FE
	ENDM


;; Equates System Roms/Flex/console.asm

ACIA	EQU	$A000		;; Wrapper address
NULL	EQU	$A011		;; NULL ADDRESS (CURRENTLY EMPTY)
MON09	EQU	$F837		;; monitor
FLPSEL	EQU	ACIA+6		;; Select floppy
FLPTRK	EQU	ACIA+7		;; Select floppy track
FLPSEC	EQU	ACIA+8		;; Select Floppy Sector
FLPWRI	EQU	ACIA+9		;; WRITE A SECTOR
DRVREG	EQU	FLPSEL		;; Return the drive number from controler
TRKREG	EQU	FLPTRK		;; return the track number from controler
SECREG	EQU	FLPSEC		;; return the sector selected (-1, as the routine decrements this)
FLPREA	EQU	FLPWRI		;; READ A SECTOR
FLPSTA	EQU	ACIA+$0A	;; Read the controller for an error code.

;; vector table disk IO
	ORG	$DE00		;Expects the table to start here	
DREAD	JMP	READ		;Read A sector
DWRITE	JMP	WRITE		;write a sector
DVERIFY	JMP	VERIFY		;verify a sector that's been written
RESTOR	JMP	RST	    	;Track 0
DRIVE	JMP	DRV	        ;Select Disk
DCHECK	JMP	CHKRDY		;Ready Checks
DQUICK	JMP	CHKRDY		; ''
DINIT	JMP	INIT		;Drive Init 
DWARM	JMP	WARM		;Drive Warm reset
DSEEK	JMP	SEEK		;Seek to a track.

;; Global Storage

CURDRV	FCB	0		;Current Drive
DRVTRK	FDB	0,0		;Current track per drive

;; INIT AND WARM
;;
;; DRIVER INITIALIZATION
INIT	
	LDX	#CURDRV		;Point to Vars 
	LDB	#5		;no. of bytes to clean
INIT3	CLR	,x+		;clear the storage
	DECB	
	BNE	INIT3		;Loop till done
WARM	RTS

;; READ
;;
;; READ ONE SECTOR
READ	BSR	SEEK	 	; seek to track and sector
	TSTB			; if B<>0 then error
	BNE	READ3		; error detected
	CLRB			; Get sector length = 256
	CLI			; disable interupts for read command (multi step controller command)
READ2	LDA	FLPREA		; read a sector byte
	STA	,X+		; put in memory inc 'x'
	DECB			; dec the counter
	BNE	READ2		; loop til done with sector
	SEI			; inable interupts, were done.
	LDB	FLPSTA		; Check for errors 
READ3
	RTS			; return

;; SEEK 
;;
;; SEEK THE SPECIFIED TRACK
SEEK
	DECB			; sectors are numberd 1 to 20 on flex disk 0 to 19 on controller
	STB	FLPSEC		; Set the sector
	LDB	FLPSTA		; Check the controler Error register.
	BNE	SEEK4		; if B <> 0 there is an error
	CMPA	TRKREG		; Compare to current track
	BEQ	SEEK4		; Exit if same track
	STA	FLPTRK		; Store the track value
	LDB	FLPSTA		; check the controler error register.
SEEK4
	RTS			; we are done, error is returned in 'b'


;; WRITE
;;
;; WRITE ONE SECTOR
WRITE
	BSR	SEEK	 	; seek to track and sector
	TSTB			; if B<>0 then error
	BNE	WRITE3		; error detected		
	CLRB			; Get sector length = 256
	CLI			; disable interupts for write command (multi step controller command)
WRITE2	
	LDA	,X+		; get a data byte from memory
	STA	FLPWRI		; write the data byte to the sector
	DECB			; decriment the counter
	BNE	WRITE2		; wait till done.
	SEI			; Enable interupts we're done (doesn't modify 'z')
	LDB	FLPSTA		; Check for errors 
WRITE3
	RTS			; we are done error is in 'b'


;; VERIFY
;;
;; VERIFY LAST SECTOR WRITTEN
;; This is redundant, the write command is going to return an error 
;; if there was an error it will be passed through we still check
;; for a drive error if there is one we return a CRC.
VERIFY
	LDB 	FLPSTA		; Check for errors
	RTS

;; RESTORE
;;
;; RESET TRACK 0
RST
	PSHS	X		; Save 'x' register
	BSR	DRV		; do select
	LDB	#1		; SET TO SECTOR 1
	CLRA			; SET TO TRACK 0
	BSR	SEEK		; do track 0 seek		
	PULS	X		; restore 'x'
	BITB	#$D8		; just check for 'other' type WD Drive Controller errors
	RTS			; return

;; DRV
;; 
;; SELECT THE SPECIFIED DRIVE
DRV
	LDA	3,X		; GET THE DRIVE NUMBER
	CMPA	#3		; is it less than 4?
	BLS	DRV2		; Branch if ok
	LDB	#$0F		; Set Error value if not
	SEC			; set carry flag.
	RTS			; return prematurely
DRV2	BSR	FINDTRK		; Find the track
	LDB	TRKREG		; Check the current track
	STB	,X		; Save it
	STA	FLPSEL		; Store the new Floppy number on controller
	STA	CURDRV		; Store the new floppy drive in memory		
	BSR	FINDTRK		; Find the track
	LDA	,X		
	STA	FLPTRK		; Set the track in the controller
	BRA	OK		; branch to exit of CHKRDY

	
;; CHKRDY
;;
;; CHECK DRIVE READY ROUTINE 
CHKRDY
	LDA	3,X		; Get drive number
	CMPA	#1		; Be sure its a 1 or 0
	BLS	OK		; branch if ok
	LDB	#$80		; else show not ready
	SEC			; set carry
	RTS			; Return early
OK	CLRB			; Show no error
	CLC			; Clear Carry
	RTS


;;
FINDTRK
	LDX	#DRVTRK		;Point to track store
	LDB	CURDRV		;Get the current drive
	ABX			;point to drive's track
	RTS			

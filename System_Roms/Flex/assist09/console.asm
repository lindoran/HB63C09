;; these are the console drivers for the HB63C09M for flex.
;; (C) D. Collins 2025

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

;; These are the IO Routines for the 6850 ACIA wrapper built into the HB6309

;; Equates

ACIA	EQU	$A000		;; Wrapper address
NULL	EQU	$A011		;; NULL ADDRESS (CURRENTLY EMPTY)
MON09	EQU	$F837		;; monitor


;; IO table for FLEX09

	ORG	$D3E5		;Expects Table to start here	Vector offset

INCHNE	FDB	INNECH		; Input ch no echo  		0
IHNDLR	FDB	IHND		; irq interuput handler		2	
SWIVEC	FDB	NULL		; SWI3 Vector			4
IRQVEC	FDB	NULL		; IRQ VECTOR LOCATOIN 		6
TMOFF	FDB	TOFF		; Timer off			8
TMON	FDB	TON		; Timer on			10
TMINT	FDB	TINT 		; Timer init			12
MONITR	FDB	MON09		; monitor reset vector		14
TINIT	FDB	TRINIT		; Terminal init			16
STAT	FDB	STATUS		; check terminal status		18	
OUTCH	FDB	OTCHR		; Terminal char output		20
INCH	FDB	INCHR		; Terminal char input		22


;; routines start here:
	ORG	$D370

;Terminl is auto set to 115200 8n1 no flow control
TRINIT	
	RTS

; input no echo 
INNECH
	LDA	ACIA		; Get terminal status
	ANDA	#$01		; A character presnent?
	BEQ	INNECH		; Loop if not
	LDA	ACIA+1		; Get the character
	ANDA	#$7F		; remove parity 
	RTS			; All Done

; input with echo
INCHR	
	BSR	INNECH		; we get the character...

; fall through to echo the character

; echo character in 'a'
OTCHR	
	PSHS	A		; save 'a'
OTCHR2	LDA	ACIA		; transmit empty?
	ANDA	#$02		
	BEQ	OTCHR2		; wait if not
	PULS	A		; restore character to 'a'
	STA	ACIA+1		; output character
	RTS

;check for a character, preserve 'a'
STATUS
	PSHS	A		; save 'a'
	LDA	ACIA		; get terminial status
	ANDA	#$01		; check for a character
	PULS	A		; restore 'a'
	RTS	

;timer handling (disabled) 
TINT	
TON
TOFF
	RTS

; interrupt handling (disabled)
IHND
	RTI


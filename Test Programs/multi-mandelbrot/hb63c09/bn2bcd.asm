;
; Title:		Binary to BCD Conversion 
;
; Name:			BN2BCD
;
; Purpose:		Converts 16 bit Binary Number's 
;			To 24 bit Packed BCD
; Entry:		Register D = Binary data
; Exit:			Register Q = BCD data
;			F = tens and ones 
;			E = thousands and one-hundreds
;			B = ten-thousands

;
; Registers Used:	Q,CC
; 

BN2BCD:
; Allocate space on stack for 1 byte to store 100's digit
	LEAS	-3,S		; Allocate 3 bytes on stack
	CLR	,S		; Clear space for 10000's digit
	CLR	1,S		; Clear space for 1000's digit
	CLR 	2,S		; Clear Space for 100's digit

; Calculate the 10000's digit
; Devide data by 10000
	LDF	#$FF		; Start quotient at -1
D10000LP:
	INCF			; add 1 to quotient
	SUBD	#10000		; subtract 10000 from the dividend (D)
	BCC	D10000LP	; jump if difference is still positive
	ADDD	#10000		; if not add the last 10000 back.
	STF	,S		; save the 10000's digit in stack space S
; Calculate the 1000's digit
; Divide the data by 1000
	LDF	#$FF		; Start quotient at -1
D1000LP:
	INCF			; add 1 to quotient
	SUBD	#1000		; subtract 1000 from the dividend (D)
	BCC	D1000LP		; jump if difference is still postive
	ADDD	#1000		; if not add the last 1000 back.
	STF	1,S		; save the 1000's digit in stack space 1,S

; Calculate 100's digit (for numbers greater than or equal to 100)
; Divide the data by 100	
	LDF	#$FF		; Start quotient at -1
D100LP:
	INCF			; Add 1 to quotient
	SUBD	#100		; Subtract 100 from the dividend (D)
	BCC	D100LP		; Jump if difference is still positive
	ADDD	#100		; If not, add the last 100 back
	STF	2,S		; Save 100's digit in stack space 2,S

; Calculate 10's digit from the remainder of 100's division
; Divide the remainder by 10
	LDF	#$FF		; Start quotient at -1
D10LP:  INCF			; Add 1 to quotient
	SUBD	#10		; Subtract 10 from the dividend (D)
	BCC	D10LP		; Jump if difference is still positive
	ADDD	#10		; If not, add the last 10 back

; Combine the 1's and 10's digits
; Shift the 10's digit to the high nibble (left shift 4 times)	
	TFR	F,A		; Load 10's digit in A
	LSLA		 	; Shift 1 time (equivalent to multiplying by 2)
	LSLA			; Shift 2 times (equivalent to multiplying by 4)
	LSLA			; Shift 3 times (equivalent to multiplying by 8)
	LSLA			; Shift 4 times (equivalent to multiplying by 16)
	ADDR	A,B		; add 1's digit to 10's digit, result in B (6309)
	TFR	D,W		; move lower digits to bottom of Q
	LDB	2,S		; load 100's digit into B
	LDA	1,S		; load 1000's digit into A
	LSLA		 	; Shift 1 time (equivalent to multiplying by 2)
	LSLA			; Shift 2 times (equivalent to multiplying by 4)
	LSLA			; Shift 3 times (equivalent to multiplying by 8)
	LSLA			; Shift 4 times (equivalent to multiplying by 16)
	ADDR	A,B		; add 100's and 1000's digit, result in B (6309)
	TFR	B,E		; move B into E (100's and 1000's)

; Return the final BCD value (A = BCD data)
	SEXW			; CLEAR W
	LDB	,S		; Load 10000's into B
	LEAS	3,S		; Deallocate 3 bytes from stack
	RTS			; Return with BCD data in register A


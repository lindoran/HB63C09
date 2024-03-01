;; hello world!
ACIA:   equ   $A000           ; ACIA Address



        org   $FFC4           ; load program at FFC4

        ldy   #hello          ; address of string into y

PrintString:
        lda   ,Y+             ; read a character into a
        cmpa  #255            ; are we at the end of the string?
        beq   retAST9         ; if we are, return to Assist09
        bra   PrintChar       ; we aren't done so we print the character
        bra   PrintString     ; keep going....

PrintChar:
wait:  ldb   ACIA             ; load the status register into b
       bitb  #2               ; is 'transmit data ready' set?
       beq   wait             ; wait if it is not.
       sta   ACIA+1           ; send the char to the ACIA
       bra   PrintString      ; get another character

retAST9:                      ; if for some reason we fall through...
       sync

hello:
        dc.b "Hello, World!",10,13,255,225

; RESET VECTORS
        DC.W    $0             ; RESERVED SLOT
        DC.W    $0             ; SOFTWARE INTERRUPT 3
        DC.W    $0             ; SOFTWARE INTERRUPT 2
        DC.W    $0             ; FAST INTERRUPT REQUEST
        DC.W    $0             ; INTERRUPT REQUEST
        DC.W    $0             ; SOFTWARE INTERRUPT
        DC.W    $0             ; NON-MASKABLE INTERRUPT
        DC.W    $FFC4          ; RESTART

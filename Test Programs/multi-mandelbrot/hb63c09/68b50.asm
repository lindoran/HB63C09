; UART and control the routines are set up for the 6850
; or 'like' such as the 6850 Wrapper in the HB63C09M.

USTAT   EQU $A000           ; UART Status Register
UDATA   EQU $A001           ; UART Data Register

; Send CRLF to terminal
CRLF:   LDA #$0A               ; Line feed
        BSR CHOUT
        LDA #$0D               ; Carriage return
        BSR CHOUT
        RTS

; Map iterations to printable ASCII characters
PLOT:   LDA 6,S                ; Load iteration count
        INCA                   ; Offset for gradient lookup
        LDY #PSUSHD            ; Address of gradient table
        LDA A,Y                ; Load corresponding ASCII shade
; Fall through to `CHOUT`

CHOUT:  PSHS A                 ; Save character in A
WRWAIT: LDA USTAT              ; Check UART status
        BITA #2                ; Ready to send?
        BEQ WRWAIT             ; Wait until ready
        PULS A                 ; Restore character
        STA UDATA              ; Send character
        RTS

;; STRING START STORED IN X
PRINT:  LDA ,X+                 ; START STRING IN X
        BEQ PRDONE              ; IF NULL TERM
        BSR CHOUT               ; PRINT THAT CHARACTER
        BRA PRINT               ; LETS GET THE NEXT ONE
PRDONE: RTS                     ; RETURN FROM PRINT

PRINT_BCD:   
        LDA ,X+                 ;Start of BCD in X
        BEQ PRINT_BCD           ;Skip MSB's Zeros
        TFR  A,B                ;Save A
        CMPA #$FF               ;it's the end? (value of number is 0)
        BEQ BCDONE              ;we are done.
        LSRA                    ;Top Nibble
        LSRA
        LSRA
        LSRA
        BEQ SKIP_HIGH           ;Skip the High bit (we know the low bits are > 1)                    
        ORA #$30                ;convert to ASCII
        BSR CHOUT               ;send character
KEEPZERO:
        LDA ,X+                 ;Get next byte
        TFR A,B                 ;Save A
        CMPA #$FF               ;is it the end?
        BEQ BCDONE              ;We're Done
        LSRA                    ;Top Nibble
        LSRA
        LSRA
        LSRA
        ORA #$30                ;convert to ASCII
        BSR CHOUT               ;send character

SKIP_HIGH:
        TFR  B,A                ;restore character
        ANDA #$0F               ;MASK HIGH
        ORA #$30
        BSR CHOUT
        BSR KEEPZERO            ;NEXT
BCDONE:
        RTS                     ; Return


; 16 levels of pseudo-shades in 7-Bit ASCII (darkest to lightest)
PSUSHD: FCB $23,$40,$25,$26,$58,$2A,$2B,$3D ; Darker characters
        FCB $2D,$7E,$3A,$2E,$2C,$60,$20,$20 ; Lighter characters
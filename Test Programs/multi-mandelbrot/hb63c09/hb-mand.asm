;; asembly battle royal for the HB63C09M
;; This simply resets the comptuer at the end of exicution, and should 
;; technically work on any homebrew computer with a serial MLM as long as 
;; there is a supported UART -- You should check the addresses in your
;; source file for the UART to make shure they align with your architecture 

;; Load this into MON09 or ASSIST09 with 'L' command by pasteing the .S19 
;; into the terminal.

;; lets enable 6309 since the CPU is required for the architecture of the computer
h6309   EQU     1
TCTRL   EQU     $A03B
TBYTH   EQU     $A03C
TBYTL   EQU     $A03D 



; interupt service routine



        ORG     $1000           ; Jump to this location with G 1000 in MON09 or ASSIST9
CONFIG:
        ifdef h6309
	LDMD #1 		; h6309 native mode
        endif
       
;; main loop
        LEAS    -5,S            ; Allocate 5 bytes on the stack
        CLR     ,S              ; Clear X (temp low byte)
        CLR     1,S             ; Clear X (temp high byte)
        CLR     2,S             ; Clear Y (temp low byte)
        CLR     3,S             ; Clear Y (temp high byte)
                                ; Dispite what mand_get says in mandelbrot.asm itterations is in 6,S

        CLR     TIMER           ; Clear the timer space
        CLR     TIMER+1
        CLR     TIMER+2
        CLR     TIMER+3
        CLR     BCDT            ;Clear the BCD SPACE
        CLR     BCDT+1          
        CLR     BCDT+2
        CLR     BCDT+3         


        LDD     #1              ; overflow value in ms                 
        STD     TBYTH           ; Set the Timer  
        ANDCC   #$EF            ; enable interupts from IRQ line
        LDA     #$81            ; bit 1 and 8 set in A (set interupts start timer) 
        STA     TCTRL           ; start timer

loop:
        LBSR    mand_get        ; Compute Mandelbrot for current position
        LBSR    PLOT            ; Map result to a gradient character and send it to UART

        LDD     ,S              ; Load X register
        ADDD    #1              ; Increment X
        STD     ,S              ; Save X back

        CMPD    #MAND_WIDTH     ; Check if X reached the width
        BNE     loop            ; If not, continue

        LBSR    CRLF            ; Send CRLF to start a new line
        CLR     ,S              ; Reset X to 0
        CLR     1,S

        LDD     2,S             ; Load Y register
        ADDD    #1              ; Increment Y
        STD     2,S             ; Save Y back

        CMPD    #MAND_HEIGHT    ; Check if Y reached the height
        BNE     loop            ; If not, continue
        LEAS    5,S             ; Deallocate stack
        
DONE:       
        CLRA
        STA     TCTRL           ; Stop Timer
        ORCC    #$10            ; disable interups IRQ line
        LDD     TIMER+2         ; pull just the 16 bits off the bottom of the timer.
        BSR     BN2BCD          ; break out BCD to Q
        STQ     BCDT            ; save temp for testing
        LDX     #BCDT           ; index for BCD Printer
        LBSR    PRINT_BCD       ; Print BCD ms
        LDX     #TSTR           ; get ready to print the string
        LBSR    PRINT           ; Print the String
        LBSR    CRLF            ; CR
        

        ifdef h6309
	
        LDMD    #0 		; h6809 emulation mode
        
        endif

        JMP     [$FFFE]         ; Jump to reset vector

;; timer interupt service routine 
ISR:
        LDA     TCTRL           ; clear the interupt 
        LDQ     TIMER           ; Load the timer
        INCW                    ; Begin 32bit inc.
        BNE     L2              
        INCD                    
L2      EQU     *

        STQ     TIMER           ; place the timer back in memory
        RTI                     ; Return from interupt

TIMER   FCB 0,0,0,0             ; 32bit Timer
BCDT:   FCB 0,0,0,0,255         ; Binary coded decimal output
TSTR:     
        FCC " msec."  ; The message string
        FCB 0         ; Null Terminator    


;; includes uart and multi-mandelbrot for 24 bit fp math for optimal results.

        INCLUDE "bn2bcd.asm"    ;bin 2 bcd converter

        INCLUDE "68b50.asm"     ;68b50 routines
        
        INCLUDE "../6x09/mandelbrot24.asm" ; Include Mandelbrot and fixed-point routines

        ORG $FFF8     ; set interupt vector for IRQ to ISR
        FDB ISR
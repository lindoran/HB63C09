; timer testing
TCTRL   EQU     $A03B




	ORG 	$1000
 
        ANDCC   #$EF            ; enable interupts from IRQ line (6309)
        LDA     #1              ; set bit 1
        STA     TCTRL           ; start timer interupts
	JMP     [$FFFE]         ; Jump to reset vector

ISR:
        ;; end early if we need to to save so many cycles!!
        ;; we are going to do very small incriments
        LDA     TCTRL           ; clear the interupt
      	RTI

; set interupt vector

 	ORG $FFF8     ; set interupt vector for IRQ to ISR
        FDB ISR
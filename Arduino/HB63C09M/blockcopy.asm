; short block copy
loader: equ    $4000                      ; copy loader to this location
sycall: equ    $3f08                      ; this is machine code to call monitor
loadsm: equ    $84                        ; this is the specific byte we write to loader+1
                                          ; this modifys the code to be lda x

loadfm: equ    $A03E                      ; address of the byte loader



        ;org    $FFC0                      ; this is where the loader is placed by the MCU
        org    $300
InitValues:
        ldx    #CopyBlock                 ; copy code starts here
        ldu    #loader                    ; destination of loader code
CpByts: ldw    #((LstByt+2)-CopyBlock)    ; number of bytes into W

;;  we need to relocate the loader to the static block so we can load the rom space.

CopyBlock:
        lda    x+                         ; transfer a byte
        sta    u+                         ; store a byte
        decb
LstByt: bne CopyBlock

;;  This is a bit tricky, the byte loader sends a new byte at each read attempt
;;  so when we are reading from the address stored in x multiple times each time
;;  this is a new value.  See the Arduinio sketch for the specifc structure
;;  of the stored data (TLDR - byte 1 and 2 = destination address,  byte 3 and 4 contain
;;  the number of bytes - and the following data is the actual ROM code.)
;;  The read attempt must be continuious and consecutive or the loader will reset.
;;  that is, you can not do other IO routines with the AVR while reading the data (like
;;  use the disk routines or the UART.)


        ldx   #loadfm                     ; set the start address to the loader
        lda   #255                        ; unlock code magic nubmer to the loader
        sta   x                           ; unlock the loader
        lda   x                           ; load destiation address into D
        ldb   x                           ; ...
        tfr   d,u                         ; we will need A
        lde   x                           ; byte count into W
        ldf   x                           ; ...
        lda   #loadsm                     ; load self modifying code to a
        sta   loader+1                    ; modify the code lda x+ -> lda x
        jmp   loader                      ; jump to self copied loader



USTAT     EQU $A000            ; UART status register address
UDATA     EQU $A001            ; UART data register address
LOADER    EQU $A03E            ; Loader register
ULOCK     EQU 255              ; UNLOCK code for the loader register
RESET     EQU 1                ; reset code for loader, causes a cpu reset (*not an avr reset*)


          ORG $1000
START     LDX #MESSAGE         ; Point X to the start of the message
LOOP      LDA ,X+              ; Load the next character
          BEQ DONE             ; If null terminator, exit loop
          JSR WAITACIA         ; Wait for UART to be ready
          STA UDATA            ; Send the character to the UART
          BRA LOOP             ; Repeat for the next character
DONE      
          LDX #LOADER          ; we will reset the monitor with the loader
          LDA #ULOCK           ; unlock the loader (magic number)
          STA ,X               ; send the unlock command
          LDA #RESET           ; reset code (see arduino sketch LOADERR)
          STA ,X               ; reset the CPU, this should take us back to the monitor.


WAITACIA  PSHS A               ; Push accumulator A onto the stack to save its value
WRWAIT    LDA  USTAT           ; Load the UART status register into accumulator A
          BITA #2              ; Test bit 1 (mask 0x02) of the UART status register
          BEQ  WRWAIT          ; If bit 1 is clear, loop back to WRWAIT (UART not ready)
          PULS A               ; Pop the original value of accumulator A from the stack
          RTS                  ; Return from subroutine


MESSAGE   FCB 10
          FCB 13
          FCB 10
          FCB 13
          FCC "Hello, World!"  ; The message string
          FCB 10
          FCB 13
          FCB 10
          FCB 13
          FCB $00              ; Null terminator
          END START


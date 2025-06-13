#include "tinytypes.h"
#include "fdos.h"
#include "ftypes.h"

// Defines for internal routines, not meant to be used by user. These are here for 
// testing, but will most likely cause bugs if used in a user program and so are not 
// wrapped
#define DINCH   0xCD09   // FLEX: Input a Character from the OS via the console
#define DINCH2  0xCD0C   // FLEX: Input a character from the redirected source 
#define DOUTCH  0xCD0F   // FLEX: Output a character from the OS via the console
#define DOUTCH2 0xCD12   // FLEX: Output a character to the redirectable source 

#define GETCHR  0xCD15   // FLEX: Get a character from the terminal or a file honors TTYSET
#define PUTCHR  0xCD18   // FLEX: Output a character to terminal or file honors TTYSET
#define INBUF   0xCD1B   // FLEX: Input a line from keyboard into line buffer
#define CLASS   0xCD21   // FLEX: Classify character as alphanumeric or not
#define PCRLF   0xCD24   // FLEX: Output carriage return and line feed
#define NXTCH   0xCD27   // FLEX: Get next character from line buffer (skips spaces, handles EOL) update psudoregister (carry)
#define RSTRIO  0xCD2A   // FLEX: Restore IO vectors for OUTCH, INCH, and File IO addresses to defalut
#define OUTDEC  0xCD39   // FLEX: Output an 16-bit value as decimal, manages leading zeros
#define OUTHEX  0xCD3C   // FLEX: Output an 8-bit value as hexadecimal
#define GETHEX  0xCD42   // FLEX: Get hexadecimal number from line buffer, update psudoregister (b,x and carry)
#define OUTADR  0xCD45   // FLEX: Output 16 bit Hexidecimal number
#define INDEC   0xCD48   // FLEX: Input a decimal number.

#define NL      0x0A     // new line
#define CR      0x0D     // carrage return
#define CCHPTR  0xCC18   // curent character pointer
#define TTYEOL  0xCC02   // end of line separator from ttyset 

flex_lastop_t flex_lastop;  // Pseudoregister structure for capturing FLEX operation results.
                            // Used to store secondary return conditions from FLEX routines,
                            // such as carry flag, register values, or other status info.
                            // Typically updated by wrapper functions after system calls.


// char fgetChar(void) -- return a character from source (terminal or file)
//
// This routine retrieves a single character, honoring all TTYSET parameters.
// - The Current Line Number location is cleared by this call.
// - If the "Input Switch" is non-zero, input is taken from INCH2 (redirected source).
// - If "Input Switch" is zero, but "File Input Address" is non-zero, input is taken from the specified file.
// - If both are zero, input is taken from the console via INCH.
// - X and B registers are preserved.
// - Preferred over INCH for general input, as it respects FLEX terminal and file redirection settings.

asm char fgetChar(void) {
    asm {
            JSR GETCHR      // get a character place in a
            TFR A,B         // return character in b (cmoc expects this)
    }
}

// fputChar (character to output) - Outputs a character to the terminal or file.
//
// Outputs a character using the FLEX PUTCHR routine ($CD18).
// - Honors all TTYSET parameters (e.g., line length, escape key).
// - If the "Special I/O Flag" is zero, manages column count and newlines.
// - If using ACIA, checks for TTYSET Escape Character and pauses output if typed.
// - If "Output Switch" is non-zero, sends the character via OUTCH2.
// - If "File Output Address" is non-zero, writes the character to the specified file (via FCB).
// - Otherwise, sends the character to the terminal using OUTCH (which can be redirected).

// note that we are pulling from 3,S this is because cmoc converts char into uint16_t 
// when it pushes to the stack, so we need to pull from the lsb of the 16 bit value 
// stored at 2,S.  OR we could have pulled 2,s into D and then Transfered B to A, but this is simpler.

asm void fputChar(char c) {
    asm {
        LDA 3,S         // Load the character to output into register A
        JSR PUTCHR      // Call the putchr routine
    }            
}

// Calls the FLEX INBUFF routine ($CD1B) to input a line from the keyboard into the Line Buffer.
// - Handles TTYSET Backspace and Delete characters.
// - Ignores all control characters except RETURN and LINE FEED.
// - RETURN is stored at the end of the buffer; LINE FEED is stored as a space but echoed as CR/LF.
// - Maximum 128 characters (including RETURN) are accepted; excess input is discarded.
// - On exit, the Line Buffer Pointer points to the start of the buffer.
// Caution: Using INBUFF will overwrite the command line buffer, which may disrupt DOS command processing.

asm void finBuffer(void) {
    asm {
        JSR INBUF       ; // Call the inbuf routine
        LDA #NL         ; // we need to do a new line as we just echoed return
        JSR PUTCHR      ; // lets put the new line in.
    }
}

// FLEX strings are very antiquated; what's worse, they don't behave like C strings.
// They use a 0x04 (EOT) character to terminate the string, and they do not use a null terminator.
// In order to use C strings with FLEX, we need to use a setup that allows us to make use of string
// literals, but also allows us to honor TTYSET parameters. -- This is the most basic way to do this.

// Building a framework to support FLEX strings as-is is difficult and requires a lot of space.
// Using C strings is the best solution as it allows the compiler to optimize the code better.
// It allows for more natural use of strings; the downside here is that we have to use a separate
// set of functions to handle binary to integer and hex conversion. As CMOC does not support
// type detection within parameters like C11, this is what makes printf so BIG in CMOC.

// print (string)

// Outputs a string literal to FLEX honoring TTYSET parameters.
// Does not support printf-style formatting, but does support
// C control characters such as \n. Just outputs the string as is with
// no trailing line feed or carriage return unless you pass \n; it mimics
// printf and passes CR as well.

// Absolutely not thread safe, don't call from an interrupt while running.
// This just runs until it's hit the null so it's not memory safe either.
// If you pass any old character buffer it's just going to run until it hits a null.

asm void print (const char *s) {
    asm {
        LDX  2,s      ; load the address of the string to output
    p1: 
        LDA  ,x+      ; load the first character of the string
        CMPA #NL      ; if its a new line
        BNE  p2       ; we have to print carrage return if not we skip and go on
        LDA  #CR      ; some retro terminals really hate it the other way round
        JSR  PUTCHR   ; print the carrage return
        LDA  #NL      ; stage the new line
    p2: JSR  PUTCHR   ; call the putchr routine to output the character
        BNE  p1       ; the character is not null, we keep going

    }
}


// classify if character is a number or a letter and update system Last
// terminator character. (this specifically changes the dos enviornment)
// if a file is open or being read this could interfear with that.

asm bool fchrClass(char ch) {
    asm {
        CLRB        ; start with b as false ie not a letter / number
        LDA 3,s     ; get the lsb from 16 bit value from the stack
        JSR CLASS   ; determine if a letter or a number,if so carry is cleared 
        BCS cl      ; if c flag is set (this also updates the last terminator in the dos enviorntment)
        INCB        ; return true if its a character 
    cl:
    }
}

// pcrlf - Outputs a carriage return and line feed to the terminal.

asm void pcrlf(void) {
    asm {
        JSR PCRLF       // Call the pcrlf routine to output a CR/LF
    }
}

// fgetNext - Fetches the next character from the FLEX line buffer using the NXTCH routine.
//
// The NXTCH routine works as follows:
// - The character in the "Current Character" location is copied to "Previous Character".
// - The character pointed to by the Line Buffer Pointer is fetched from the Line Buffer and saved in "Current Character".
// - Multiple spaces are skipped, so a sequence of spaces is treated as a single space.
// - The Line Buffer Pointer is advanced to the next character, unless the fetched character is a RETURN (0x0D) or the TTYSET End-of-Line character.
//   - If RETURN or EOL is encountered, repeated calls to NXTCH will continue to return the same character.
// - NXTCH cannot cross into the next command in the buffer.
// - On exit, NXTCH calls the CLASS routine to classify the character (alphanumeric or not).
// - The fetched character is returned in register A (and B for CMOC).
// - The carry flag is clear if the character is alphanumeric, set otherwise.
// - Registers B and X are preserved.
//
// This function is typically used to sequentially read and parse command-line or file input from the FLEX system buffer.
// updates the psudo register value - flex_lastop.carry 

asm char fgetNext (void) {
    asm {
        CLR  :flex_lastop.carry     ; no error (a character)
        JSR  NXTCH                  ; get next character
        BCC  fgn1                   ; check for error
        INC  :flex_lastop.carry     ; theres an error (no character)
    fgn1:    
        TFR  A,B                    ; place in B where cmoc expects return
    }
}

// Restore IO vectors (for completeness).
asm void frestoreIO (void) {
    asm {
        JSR RSTRIO      ; restore io vectors
    }
}

// outputs a 16 bit unsigned number as an intiger, if spaces is zero (FALSE), printing
// will start with first non zero number.  if spaces is non zero (TRUE) zeros will be
// substituted for leading zeros.

asm void foutDecimal(const void* var, bool spaces) { 
    asm {
        LDX 2,s     ; get the pointer address
        LDB 5,s     ; get space operation
        JSR OUTDEC  ; print the number

    }
}

// foutHex (var) - Outputs the contents of an 8 bit variable as hexadecimal digits.
// void type is used to minimize casting in this case, generally this expects a 
// pointer to char or uint8_t, but can be used with any 8 bit variable. if used 
// with a larger variable, only the first byte will be output as hex. 

asm void foutHex(const void* var) {
    asm {
        LDX 2,s        // Load the address of the variable to output
        JSR OUTHEX     // Call the outhex routine
    }


}

// fgetHex reads a hexadecimal number from the FLEX line buffer using the GETHEX routine ($CD42).
// - On entry, the Line Buffer Pointer should point to the first character of the number.
// - On exit:
//   - Carry is clear if a valid number was found.
//   - Register B is nonzero if a number was found, zero if only a separator was found.
//   - Register X contains the parsed value (0 if no number).
//   - The Line Buffer Pointer is advanced past the separator (unless it's CR or EOL).
//   - If a non-hex character is found, skips to the next separator and sets carry.
// - The value is truncated to 16 bits (0..0xFFFF).
// - Updates the flex_lastop pseudoregister. (carry, x and b)

asm uint16_t fgetHex(void) {
    asm {
            CLR  :flex_lastop.carry     ; no error (a character)
            JSR  GETHEX                 ; Get a hex number
            BCC  fgh1
            INC  :flex_lastop.carry     ; error (no character)
    fgh1:   
            STB  :flex_lastop.reg_b     ; save b for later
            STX  :flex_lastop.reg_x     ; save x for later
            TFR  X,D                    ; Transfer X (16-bit result) to D for CMOC return
    }
}


// foutHex (var) - Outputs the contents of an 16 bit variable as hexadecimal digits.
// void type is used to minimize casting in this case, generally this expects a 
// pointer to uint16_t, but can be used with any 16 bit variable. if used 
// with a smaller variable, it will output two bytes in sequence and the first
// two hex digits will contain the value of the 8 bit type with the remaining 
// two digits based on whatever physical data comes next.

asm void foutLongHex(const void* var) {
    asm {
        LDX 2,S     ; load the address of variable to output
        JSR OUTADR  ; output the 16 bit number at pointer
    }
}

// fgetDec (void)
// - On entry, the Line Buffer Pointer should point to the first character of the number.
// - On exit:
//   - Carry is clear if a valid number was found.
//   - Register B is nonzero if a number was found, zero if only a separator was found.
//   - Register X contains the parsed value (0 if no number).
//   - The Line Buffer Pointer is advanced past the separator (unless it's CR or EOL).
//   - If a non-hex character is found, skips to the next separator and sets carry.
// - The value is truncated to 16 bits of presision (0..65535)
// - Updates the flex_lastop pseudoregister. (carry, x and b)

asm uint16_t fgetDec(void) {
        asm {   
            CLR  :flex_lastop.carry     ; no error (a character)
            JSR  INDEC                  ; Get a decimal numberFLEX_DOS_MEMMAP->line_buffer_ptr
            STB  :flex_lastop.reg_b     ; save b for later
            STX  :flex_lastop.reg_x     ; save x for later
            TFR  X,D                    ; Transfer X (16-bit result) to D for CMOC return
        }
}

//void setLineBuffer(const char s*)
//Loads the string into the start of the system buffer. resets the relivant pointers.
// - destroys the system buffer
// - sets the pointer at FLEX_DOSMEMMAP->line_buffer_ptr to 0xC080 in (ftypes.h as LBUFF)
// - terminates line with CR (0x0D) for compatiblilty with DOCMND etc.. 
// - this is as if the user entered a line with INBUFF but imports a c string literal.
// - this replaces the final character in the string with a CR (which should stop most flex tolkinizers)

asm void setLineBuffer(const char* s) {
    asm {
            LDY #LBUFF                              ; load x with the start of the line buffer 
            STY CCHPTR                              ; store the buffer starting address into the pointer   
            LDX 2,S                                 ; Get the string addres from the entry point
    slb1:
            LDA ,x+                                 ; get the character at x 
            STA ,y+
            BNE slb1                                ; if not zero keep going 
            LDA #CR                                 ; terminator
            STA -1,y                                ; terminate the string in the buffer with the TTY set separator
    }   

}

// Copies the next token from the FLEX line buffer into a C string using NXTCH.
// Stops at a separator or EOL as defined by FLEX.
// dst must be large enough to hold the token plus a null terminator.

asm void getLineToken(char* dst) {
    asm {
        LDX 2,S            ; X = destination buffer pointer (dst)
    glt1:
        JSR NXTCH          ; Get next character from FLEX buffer (A)
        BCS glt2           ; If separator or EOL, stop
        STA ,x+            ; Store character to destination
        BRA glt1
    glt2:
        CLR ,x             ; Null-terminate the C string
    }
}



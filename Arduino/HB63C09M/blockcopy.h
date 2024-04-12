#ifndef _blockcopy_H
#define _blockcopy_H
/*
  ;; short block copy

;; the hb63C09M boot up process 
;; The HB63C09M uses a Atmega32A Micro controller as it's io controller.  It handles many of
;; the IO features of the system including stageing (herafter refered to as the AVR). 
;;
;;   1) system enters reset by virtue of the control line attached to the AVR
;;   2) system enters halt   "" 
;;   3) once halted, the buses (control, addres and data) are held high by the pull ups, 
;;      reset and halt are asserted low by the AVR.
;;   4) AVR takes control of the bus transceiver, pulling xsin_ low (it does this by setting 
;;      the pin to output). It then sets _bioreq low to enable the transceiver by doing the 
;;      same.
;;   5) AVR takes control of the bus direction by setting the r/w_ pin for the next few steps 
;;      this is bit 2 on PORTB (note the data direction from the AVR is opisite the CPU - 
;;      so high = write and low = read.) this is PUSH - PULL. 
;;   6) AVR uses the "address nibble" bits 2-7 on PORTC to pull the the address lines A0 - A5  
;;      low to set the write address for the next few steps as needed. - rememember the  
;;      addresses are a logical 'AND' with 0xFFFF.(ADDRESS NIBBLE) by virtue of the pull up 
;;      resistors on the bus so setting the address nibble to 0x00 will efectively set the 
;;      address bus to 0xFFC0 in terms of the ram chip.  Specifically the pins on PORTC are 
;;      set to outputs to pull the lines low and input to set the lines high (essentially an 
;;      open collector configuration.)
;;   7) AVR sets the frist byte of the reset vector to the address bus, and stores 0x4000 to 
;;      the location in memory at 0xFFFE using _bwr and _brd (bits 2 and 3 on PORTD) using a 
;;      open collector configuration as described above with the address nibble.  If it can 
;;      not read back this byte it asks if it should asume the option rom is installed.
;;              - if the option rom is installed, the computer can boot from rom 
;;                and flow would skip the following, set the system to a run
;;                state, which would set all of the control lines back to their 
;;                "run" position as described below in step 9.
;;   8) AVR sets to it's lowest meomry loaction addressable (0xFFC0) and loads the following
;;      6309 asembly code to the memory.
;;   9) The AVR enters a run state, which sets all of the control lines to read, and removes 
;;      any internal pull up value after wich it proceeds to release reset and halt to let 
;;      the CPU run freely -- the CPU then jumps to the location stored at 0xFFFE (the loader
;;      code or the option rom) If its the option rom, computer loads as normal to whatever is 
;;      loaded in this space, and flow does not continue to 10
;;  10) the byte loader code, being loaded to memory copyies itself to 0x4000, the top of the 
;;      fixed block.
;;
;;      for the next few steps keep in mind AVR can only move 8 bits at a time on to the bus, 
;;      and the byte loader on the AVR resides at only one memory location.  Therefore 16 bit 
;;      values must be loaded into a split acumulator pair, and transfered to the apropriate  
;;      index register afterwards. -- so if it looks like data is being moved around inefficently
;;      this is due to that limitation. 
         
;;      The code from this point on is inline commented.

loader: equ    $4000                    ; copy loader to this location

loadsm: equ    $84                      ; this is the specific byte we write to loader+1
                                        ; this modifys the code at that location to be 'lda x'
loadfm: equ    $A03E                    ; address of the byte loader

        org    $FFC0                    ; this is where the loader is placed by the MCU
        ldx    #CopyBlock               ; copy code starts here
        ldy    #loader                  ; destination of loader code - 
        ldw    #((LstByt+2)-CopyBlock)  ; number of bytes into W 

;;  we need to relocate the loader to the static block so we can load the rom space.

CopyBlock:
        lda    ,x+                      ; transfer a byte
        sta    ,y+                      ; store a byte
        decw                            ; this location is also tracked by the AVR to detrmine 
                                        ; when to reset after the code is loaded
LstByt: bne CopyBlock

;;  This is a bit tricky, the byte loader sends a new byte at each read attempt
;;  so when we are reading from the address stored in x multiple times each time
;;  this is a new value.  See the Arduinio sketch for the specifc structure
;;  of the stored data (TLDR - byte 1 and 2 = destination address,  byte 3 and 4 contain
;;  the number of bytes - and the following data is the actual ROM code.)
;;  The read attempt must be continuious and consecutive or the loader will reset.
;;  that is, you can not do other IO routines with the AVR while reading the data (like
;;  use the disk routines or the UART.)
;;
;;  keep in mind -  the destionation address is in y, the byte loader address (the AVR) 
;;  is in x, and the byte count is stored in w.  When the code jumps to the loader this is 
;;  a modified version of the code above this comment block from CopyBlock to LstByt almost
;;  exactly -- with the acception that lda ,x+ has now become lda ,x by virtue of the last 
;;  two lines before the jump.


        ldx   #loadfm                   ; set the start address to the loader
        lda   #255                      ; unlock code magic nubmer to the loader
        sta   ,x                        ; unlock the loader
        lda   ,x                        ; load destiation address into D
        ldb   ,x                        ; ''
        tfr   d,y                       ; we will need A
        lde   ,x                        ; byte count into W
        ldf   ,x                        ; ''
        lda   #loadsm                   ; load self modifying code to a
        sta   loader+1                  ; modify the code lda ,x+ -> lda ,x
        jmp   loader                    ; jump to self copied loader

;;  from this point CPU is in free run, but after 1/4 E the AVR should reset the system
;;  as it has counted along with the w register. From this point the 63C09 should bootstrap
;;  the system by loading 1 byte at a time to the location set by the y register typically
;;  to the top of memory.


*/

// reset vector is set by the AVR

//This is the 63C09 loader Machine code.  see blockcopy.asm above for source

#define blockcopy_start 0x0000FFC0ul
#define blockcopy_len 0x0000002Eu
#define blockcopy_end 0x0000FFEDul
static const unsigned char blockcopy_data[] =
{
  0x8e,0xff,0xcb,0x10,0x8e,0x40,0x00,0x10,0x86,0x00,0x08,0xa6,0x80,0xa7,0xa0,0x10,
  0x5a,0x26,0xf8,0x8e,0xa0,0x3e,0x86,0xff,0xa7,0x84,0xa6,0x84,0xe6,0x84,0x1f,0x02,
  0x11,0xa6,0x84,0x11,0xe6,0x84,0x86,0x84,0xb7,0x40,0x01,0x7e,0x40,0x00
};

typedef struct
{
  const char *data;
  unsigned long start;
  unsigned long end;
  unsigned len;
} blockcopy_blk;
static const blockcopy_blk blockcopy_blks[] =
{
  { blockcopy_data, blockcopy_start, blockcopy_end, blockcopy_len },
  { 0, 0, 0, 0 }
};

#endif /* _blockcopy_H */

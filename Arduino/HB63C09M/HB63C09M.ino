// This is very rough - be advised 

/*
 * MIT License

Copyright (c) 2024 Dave Collins

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */

// This is a test

#include <stdint.h>



// PORT A IS THE AVR DATABUS
#define  D0           0 // PA0 pin 40   
#define  D1           1 // PA1 pin 39
#define  D2           2 // PA2 pin 38
#define  D3           3 // PA3 pin 37
#define  D4           4 // PA4 pin 36
#define  D5           5 // PA5 pin 35
#define  D6           6 // PA6 pin 34
#define  D7           7 // PA7 pin 33

#define  res_         0  // PB0 pin 1    6309 ~reset line
#define  halt_        1  // PB1 pin 2    6309 ~Halt line
#define  r_w          2  // PB2 pin 3    6309 read/~write line
#define  irq_         3  // PB3 pin 4    6309 ~irq
#define  ss_          4  // PB4 pin 5    sd SPI
#define  mosi         5  // PB5 pin 6    sd SPI
#define  miso         6  // PB6 pin 7    sd SPI
#define  sck          7  // PB7 pin 8    sd SPI

#define  scl_pc0      0 // PC0 pin 22   i2c signals
#define  sda_pc1      1 // PC1 pin 23   i2c signals
#define  a0           2 // PC2 pin 24   6309 A0}     These are lines used to sniff the address bus when the clock is stretching
#define  a1           3 // PC3 pin 25   6309 A1}     this controls the function of the databus, just like any other paripheral 
#define  a2           4 // PC4 pin 26   6309 A2} --  addtionally, we can control the top 16 bytes of the adress space in order    
#define  a3           5 // PC5 pin 27   6309 A3}     to set the vector table in ram only mode. (at least that is the hope)     
#define  res1         6 // PC6 pin 28   future use
#define  res2         7 // PC7 pin 29   future use

#define a_nibble ((PINC & 60) >> 2)     //this pulls the adress lines off as a 4 bit nibble

#define  rx           0 // PD0 pin 14   This is the RX PIN
#define  tx           1 // PD1 pin 15   This is the TX PIN
#define  wr_          2 // PD2 pin 16   RAM wr_ strobe
#define  rd_          3 // PD3 pin 17   RAM rd_ strobe
#define  bclk         4 // PD4 pin 18   bank address clock pin
#define  xsin         5 // PD5 pin 19   bus tranciever inhibit line
#define  ioreq_       6 // PD6 pin 20   io request bar line
#define  iognt_       7 // PD7 pin 21   io grant bar line

// Global System variables

uint8_t busData = 0;    // data for current step through the loop
uint8_t bankReg = 0;    // last value set in bank register

void setup() {
Serial.begin(115200);


busTstate();               // This tri-states the MCU bus -- it is pulled low by external pulldowns 

//outputs
bitSet(DDRD, iognt_); // these two lines set the grant signal 
bitSet(PORTD, iognt_);
bitSet(DDRB, res_); // seting these to outputs simply sets them low
bitSet(DDRB, halt_);
bitSet(DDRD, bclk); // set up the bank clock pin

// 63C09 System is in tri-state

/*  clear bank register: 
 *   we turn some inputs around to take controll of the busses
 *   since the mcbus is tied low, it passes 0x00 to the bus
 *   through the tranciver.  bclk is pulsed which sets the bank register
 *   to 0x00.  We then set the bankReg variable to 0 which stores
 *   the value that is passed when reading 0xA00F (the bank register)
 */ 
 
bitSet(DDRD,  xsin);
bitSet(PORTD, xsin);      // inhibit the bus tranciever at its enable pin
bitSet(DDRB, r_w);
bitSet(PORTB, r_w);       // set tranciever to put data on the bus
bitSet(DDRD, ioreq_);     // set io request as out / it's already clear so ioreq_ is low which enables transciever

bitSet(PORTD, bclk);
bitClear(PORTD, bclk);    //register is now Zero
bankReg = 0;

_delay_ms(1600);          // Delay is needed for some USB dongles to properly initilize after being pluged in.

//inputs        
bitClear(DDRB, r_w);
bitClear(DDRD, ioreq_);
bitClear(DDRD, xsin);

Serial.println();
Serial.println("HB6809 - HB63C09M Test Build");


// Lets burn this candle! -- System coming out of reset state
// these two have external pull ups- setting the lines on the arduino to inputs effectively tri-states the link between the two chips

bitClear(DDRB, res_);
bitClear(DDRB, halt_);

}


// Main loop

void loop(){
  // checking for ioreq_
  if (!(bitRead(PIND, ioreq_))) {
      if (bitRead(PIND, xsin)) {
        busIO();  // end early for expansion bus IO  
      }
      else {
       // bus write
       if (!(bitRead(PINB, r_w))) {
           
           busData = busRead();    // read the waiting data from the bus.
           
           switch (a_nibble) {
             case 0:
               //This is normally the UART control register for the 6850, since it is internally
               //configured in the AVR we don't need to store this information. but we need to 
               //leave this for legacy support of older code. 
            
              break;
             
             case 1:
              // send a byte to the terminal.
             
              Serial.write(busData);
             
              break;

             case 15: 
              // write value on the bus to the bank adress latch
              bankReg = busData;
              bitSet(PORTD, bclk);
              bitClear(PORTD, bclk);
             
             default:
              break;
              // should never jump to here - this can never be true.
          }
       }  
       // bus read
       else {
            busData = 0;
            switch (a_nibble) {
              case 0:
               //This is the UART status register it is simalar to the 6850 (however has a built in buffer TX & RX) 
               // bit 0 = Receive Data register full (this is set if there is data waiting)
               // bit 1 = transmit data empty (this is set if tx buffer has space)
               // bit 2 = depriciated
               // bit 3 = depriciated
               // bit 4 = depricaited
               // bit 5 = buffer over/underrun - not yet implimented 
               // bit 6 = depricated 
               // bit 7 = irq  - not yet implimented
               
               // buffers are manually set to 128bytes, testing has shown that there is little chance of an underun / overflow 
               // set at this level. since hardware flow control is not used, the other status bits are not set.  
               // /DCD, /CTS and FE are depreciated.

               // TODO - write code for bits 5 and 7 which will interupt the CPU if the buffer has under/over run
               
               if (Serial.available() > 0) bitSet(busData, 0);
               if (Serial.availableForWrite() > 0) bitSet(busData, 1);
              
               break; // end of read uart control register
              
              case 1:
               //this is reading a byte from the terminal. so we need to write the next byte in
               //the uart to the bus.

               busData = Serial.read();
              
               break; // end of read uart 

              case 15:
              //this reads out last value stored in bank register.
              busData = bankReg;
             
              default:
               break;
               // should never jump here, this can never be true.
            }
            busWrite(busData);  // data is on the bus
       } // inner else
       busIO();  // end the current Io Request as data is ready to be read or written;
      } // outer else
      
  
     } // io request
     

} // end - on to next loop

// tri-state the bus
void busTstate(void) {
    DDRA = 0x00;            // input
    PORTA = 0x00;           // atmega bus is in tri-state
}

//read a bite, asumes bus is configured to read.
uint8_t busRead(void) {
  return(PINA); 
}

//write a byte, leaves the bus in write, must be tri-stated with busTstate()

void busWrite(uint8_t data) {
  DDRA = 0xFF;     // bus is output
  PORTA = data; // data is on the b
}

//end the current io request
void busIO(void) {
      // send bgnt_
      bitClear(PORTD, iognt_);  
      bitSet(PORTD, iognt_);
      busTstate(); // this just makes sure we are ready to read on the next go-through.
}

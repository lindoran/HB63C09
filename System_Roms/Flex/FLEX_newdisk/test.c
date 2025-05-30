#include "fdos.h"
#include "fbios.h"


int main() {

      
       char test[] = "hello world \r\n"; // Define a string to be printed
       print("FLEX New Disk Test Program ");
       println("Version 1.0");
       print(test);
       fputChar('A'); // Output a character to the console
       pcrlf(); // Output a carriage return and line feed

       while (getTerminalState()); // Wait for any key to be pressed 
       char var = getCharNoEcho(); // Wait for a character without echo
       foutHex(&var); // Output the variable as hex
       pcrlf();      
       outChar('A'); // Output a character to the console
       pcrlf(); // Output a carriage return and line feed



    // exit to flex
    exitFLEX();


    return 0;
}
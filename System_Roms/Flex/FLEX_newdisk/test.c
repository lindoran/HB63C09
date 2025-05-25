#include <cmoc.h>
#include "tinytypes.h"
#include "fbios.h"

// placeholder for the main function




int main() {

    asm {
        LDA #'A'
        JSR [$D3EF]
        
    }
    putChar('A'); // Example usage of outChar
    exitFLEX(); // Example usage of exitFLEX




    return 0;
}
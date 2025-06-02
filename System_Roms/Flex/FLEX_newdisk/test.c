#include "fdos.h"
#include "fbios.h"
#include "ftypes.h"
#include "tinytypes.h"

int main() {

    char test[] = "hello world \r\n";
    print("FLEX New Disk Test Program ");
    println("Version 1.0");
    print(test);
    fputChar('A');
    pcrlf();

    // Wait for any key to be pressed 
    while (getTerminalState());
    char var = getCharNoEcho();
    foutHex(&var);
    pcrlf();
    outChar('A');
    pcrlf();

    // Read from struct
    uint16_t memend_struct = FLEX_DOS_MEMMAP->memory_end;
    print("memend (struct): ");
    foutHex((uint8_t*)&memend_struct);     // High byte
    foutHex((uint8_t*)&memend_struct + 1); // Low byte
    pcrlf();

    char input[129]; // 128 chars + null terminator
    int i = 0;
    char ch;

    print("Type a line and press RETURN:\r\n");
    finBuf(); // Read a line into the FLEX system buffer

    // Fetch characters from the buffer and build a C string
    while (i < 128) {
        ch = fgetNext();
        if (ch == 0x0D || ch == FLEX_DOS_MEMMAP->ttyset_eol) // RETURN or EOL
            break;
        input[i++] = ch;
    }
    input[i] = '\0'; // Null-terminate the string

    print("You typed: ");
    println(input);
    
    exitFLEX();
    return 0;
}
#include <cmoc.h>

#define IO_BASE_ADDRESS 0xA000

// I/O addresses for disk operations
unsigned char* SELDISK   = (unsigned char*)(IO_BASE_ADDRESS + 0x02);
unsigned char* SELTRACK  = (unsigned char*)(IO_BASE_ADDRESS + 0x03);
unsigned char* SELSECT   = (unsigned char*)(IO_BASE_ADDRESS + 0x04);
unsigned char* WRITESECT = (unsigned char*)(IO_BASE_ADDRESS + 0x05);

// Disk parameters
#define NUM_TRACKS 512
#define NUM_SECTORS_PER_TRACK 32
#define SECTOR_SIZE 512

void softStart(void) {
    asm {
        swi
        fcb     8   // MONITR - Soft start ASSIST09
    }
}

void fillDiskWithZeros(void) {
    // Select disk (assuming disk 0, adjust as needed)
    *SELDISK = 0;

    // Loop through all tracks and sectors
    for (short track = 0; track < NUM_TRACKS; ++track) {
        for (unsigned char sector = 0; sector < NUM_SECTORS_PER_TRACK; ++sector) {
            // Select track and sector
            *SELTRACK = (unsigned char)track & 0xFF;         // LSB
            *SELTRACK = (unsigned char)((track >> 8) & 0xFF);  // MSB
            *SELSECT = sector;

            // Fill the sector with '00'
            for (short byteCount = 0; byteCount < SECTOR_SIZE; ++byteCount) {
                *WRITESECT = 0x00;
            }
        }
    }
}

int main(void) {
    fillDiskWithZeros();

    // Reset the monitor after the operation is complete
    softStart();

    return 0;
}

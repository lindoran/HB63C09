#ifndef FTYPES_H
#define FTYPES_H

/* This is memory region defines for the Flex 9 OS by Technical Systems Consultants
 * a very simple way of grabbing pointers to specific memory regions in a human readable
 * way.
 * 
 * this is public domain software and is released for any use, 
 * including commercial use. no warranty is given or implied.
 * 
 * Copyright (C) 2025 David Collins
 * 
 */
 
#include "tinytypes.h"

#define LBUFF       0xC080  // start of line buffer
#define SVARS       0xCC00  // start of system variables

// Line buffer: $C080-$C0FF (128 bytes)
typedef struct {
    uint8_t buffer[0x80];
} flex_line_buffer_t;

#define FLEX_LINE_BUFFER ((flex_line_buffer_t*)LBUFF)

// DOS memory map: $CC00-$CCFF
typedef struct {
    uint8_t  ttyset_backspace;        // $CC00
    uint8_t  ttyset_delete;           // $CC01
    uint8_t  ttyset_eol;              // $CC02
    uint8_t  ttyset_depth;            // $CC03
    uint8_t  ttyset_width;            // $CC04
    uint8_t  ttyset_null;             // $CC05
    uint8_t  ttyset_tab;              // $CC06
    uint8_t  ttyset_bs_echo;          // $CC07
    uint8_t  ttyset_eject;            // $CC08
    uint8_t  ttyset_pause;            // $CC09
    uint8_t  ttyset_escape;           // $CC0A
    uint8_t  system_drive;            // $CC0B
    uint8_t  working_drive;           // $CC0C
    uint8_t  system_scratch;          // $CC0D
    uint8_t  system_date[3];          // $CC0E-$CC10
    uint8_t  last_terminator;         // $CC11
    uint16_t user_cmd_table_addr;     // $CC12-$CC13
    uint16_t line_buffer_ptr;         // $CC14-$CC15
    uint16_t escape_return_reg;       // $CC16-$CC17
    uint8_t  current_char;            // $CC18
    uint8_t  prev_char;               // $CC19
    uint8_t  current_line_num;        // $CC1A
    uint16_t loader_addr_offset;      // $CC1B-$CC1C
    uint8_t  transfer_flag;           // $CC1D
    uint16_t transfer_addr;           // $CC1E-$CC1F
    uint8_t  error_type;              // $CC20
    uint8_t  special_io_flag;         // $CC21
    uint8_t  output_switch;           // $CC22
    uint8_t  input_switch;            // $CC23
    uint16_t file_output_addr;        // $CC24-$CC25
    uint16_t file_input_addr;         // $CC26-$CC27
    uint8_t  command_flag;            // $CC28
    uint8_t  current_output_col;      // $CC29
    uint8_t  system_scratch2;         // $CC2A
    uint16_t memory_end;              // $CC2B-$CC2C
    uint16_t error_name_vector;       // $CC2D-$CC2E
    uint8_t  file_input_echo_flag;    // $CC2F
    uint8_t  system_scratch3[0x1E];  // $CC30-$CC4D
    uint8_t  system_constants[0x72]; // $CC4E-$CCBF
    uint8_t  printer_init[0x18];     // $CCC0-$CCD7
    uint8_t  printer_ready[0x0C];    // $CCD8-$CCE3
    uint8_t  printer_output[0x14];   // $CCE4-$CCF7
    uint8_t  system_scratch4[0x08];  // $CCF8-$CCFF
} flex_dos_memmap_t;

#define FLEX_DOS_MEMMAP ((flex_dos_memmap_t*)SVARS)

#endif /* FTYPES_H */
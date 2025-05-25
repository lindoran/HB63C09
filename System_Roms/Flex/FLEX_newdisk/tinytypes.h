#ifndef tinytypes_h
#define tinytypes_h

/* quick and dirty intiger types for CMOC very limited 
 * 
 * this is public domain software and is released for any use, 
 * including commercial use. no warranty is given or implied.
 * 
 * Copyright (C) 2025 David Collins
 * 
 */

// Define the types for 8, 16, and 32 bit unisgned integers
typedef unsigned char uint8_t;      // Define uint8_t as char
typedef unsigned int uint16_t;      // Define uint16_t as int
typedef unsigned long uint32_t;     // Define uint32_t as long

// define the types for 8, 16, and 32 bit signed integers
typedef char int8_t;                // Define int8_t as char
typedef int int16_t;                // Define int16_t as int
typedef long int32_t;               // Define int32_t as long


// Define other common types
typedef unsigned char byte;         // Define byte as char
typedef unsigned int word;         // Define word as int
typedef unsigned long dword;        // Define dword as long

// Define boolean types
typedef char bool;                 // Define bool as char
#define true 1                    // Define true as 1
#define false 0                   // Define false as 0  




#endif /* tinytypes_h */



// Virtual CMOS Settings Management for the HB63C09M

// see LICENCE for details

// this is a minimal staging envorment for the HB63C09M. This uses sd direct access vs iOS floppy emulation
// its a WYSIWYG command interpereter, pressing ? <enter> shows avalible commands.

/* 
Attribution: 

IOS/Z80-MBC Code for Floppy emulation is (C) Fabio Defabis under the GPL 3.0
In line call outs within the code specify where this is the case. (in HB63C09M main sketch)
Details for the Z80-MBC2 project are: 
https://github.com/SuperFabius/Z80-MBC2

SD library from: https://github.com/greiman/PetitFS (based on 
PetitFS: http://elm-chan.org/fsw/ff/00index_p.html)

PetitFS licence:
/-----------------------------------------------------------------------------/
/  Petit FatFs - FAT file system module  R0.03                  (C)ChaN, 2014
/-----------------------------------------------------------------------------/
/ Petit FatFs module is a generic FAT file system module for small embedded
/ systems. This is a free software that opened for education, research and
/ commercial developments under license policy of following trems.
/
/  Copyright (C) 2014, ChaN, all right reserved.
/
/ * The Petit FatFs module is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/-----------------------------------------------------------------------------/
*/

// vcmos.cpp

#include <Arduino.h>
#include <stdint.h>
#include <string.h>
#include <SPI.h>
#include <SD.h>
#include "vbios.h"
#include "const.h" 

//command processor commands
static void cmd_help(const char*, const char*);
static void cmd_commit(const char*, const char*);
static void cmd_show(const char*, const char*);
static void cmd_dir(const char*, const char*);
static void cmd_cd(const char*, const char*);
static void cmd_set(const char*, const char*);



typedef void (*CommandFunc)(const char*, const char*);

// Structure to represent a variable
typedef struct {
    char name[VAR_SYMBOL_LENGTH];
    void *ptr;
    char formatSpecifier[VAR_FORMAT_LENGTH];
} Variable;

// command table structure 
typedef struct {
  const char* name; 
  CommandFunc function;
} Command;

// command table
Command commands[] = {
    {"HELP", cmd_help},
    {"?", cmd_help},
    {"COMMIT", cmd_commit},
    {"SHOW", cmd_show},
    {"DIR", cmd_dir},
    {"CD", cmd_cd},
    {"SET", cmd_set},
    
};


// Variable table
Variable variables[NUM_VARIABLES] = {
    {"START", &biosStart, "%X"},
    {"SIZE", &biosSize, "%X"},
    {"FILE", &biosName, "%s"},
    {"PATH", &curPath, "%s"}
};

char shellPath[MAX_FN_LENGTH]  = "/";  // path the user has set with CD

void buildFilePath(const char* directory, const char* filename) {
    switch(strlen(directory)) {

      // root directory 
      case 0: // fall through to 1 ( this is 0 or 1)
      case 1:
        sprintf(filePath, "/%s",filename);
        break;
      
      // sub directory 
      default :
        sprintf(filePath, "/%s/%s", directory, filename);
    }
    
}


// change the directory (sub directory not suported)
FRESULT change_dir(const char *path) {
    lastDir = dir; // save incase of errors.
    
    // to go back twards root by 1
    if (strcmp(path, "..") == 0) path = "/";
    
    // set the new system directory
    errCodeSD = pf_opendir(&dir, path);
    if (errCodeSD) {
        dir = lastDir; // change it back if its an error
    }
    strcpy(shellPath, path); // save the path name for use with set file.
    // strcpy(curPath, path);  // copy the new path name for later
    return errCodeSD;
}

// displays a directory entry. TODO: filtering for things like *.BAS etc.
FRESULT scan_files(void) {  
    errCodeSD = pf_readdir(&dir, &fno);
    if (errCodeSD || (fno.fname[0] == 0)) { 
      return errCodeSD; // we are done for 1 reson or another.
    }
    // Print file name, printf formating is broken maybe? this works
    Serial.print(fno.fname);
    for (int a = 0; a < MAX_FN_LENGTH - strlen(fno.fname) - 1 ; a++) {
      Serial.write(' ');
    }

    // Print file type          
    Serial.printf("  %s  ", (fno.fattrib & AM_DIR) ? "<DIR>" : "     ");

    // Print size and label if not directory TODO: commas, every thousands
    if (!(fno.fattrib & AM_DIR)) {
      Serial.printf("%10lu", fno.fsize); 
    } else {  // blank out column in case of directory. 
      for (int a = 0; a < 10 ; a++) {
        Serial.write(' ');
      }
    }

    // Prinnt date and time (NA format date)
    Serial.printf("  %02u/%02u/%04u  %02u:%02u:%02u",
      ((fno.fdate >> 5) & 0xF), (fno.fdate & 0x1F),
      ((fno.fdate >> 9) + 1980),
      (fno.ftime >> 11), ((fno.ftime >> 5) & 0x3F),
      ((fno.ftime & 0x1F) * 2));
                
    // Print a newline after each file
    Serial.println();
        
    return errCodeSD;
}


// test for a valid hex digit otherwise return false
static bool isHex(const String& str) {
    for (size_t i = 0; i < str.length(); i++) {
        char c = str.charAt(i);
        if (!isxdigit(c)) {
            return false;
        }
    }
    return true;
}

static void setVariable(int index, const char* value) {
    if (value == nullptr || value[0] == '\0') {
        // Empty or null input value, do nothing
        return;
    }
    
    if (strcmp(variables[index].formatSpecifier, "%s") == 0) {
        // If the format specifier is "%s", copy the string directly
        strncpy(reinterpret_cast<char*>(variables[index].ptr), value, MAX_FN_LENGTH - 1);
        reinterpret_cast<char*>(variables[index].ptr)[MAX_FN_LENGTH - 1] = '\0'; // Ensure null-termination
        if (index == 2) {
          strcpy(curPath, shellPath);  // copy the new path name for later
        }
    } else if (strcmp(variables[index].formatSpecifier, "%X") == 0) {
        // If the format specifier is "%X", parse as hexadecimal
        uint16_t hexValue = 0;
        
        // Check if the input starts with "0x" prefix
        if (strncmp(value, "0X", 2) == 0) {
            // Skip the "0x" prefix and parse the rest as hexadecimal
            value += 2;
        }
        
        if (!isHex(value)) {  // not a number?
            Serial.println(F("Value is not numeric!"));
            return;
        }


        // command processor corrects for capitals, convert to uint16_t
        for (int i = 0; value[i] != '\0'; ++i) {   
            if (value[i] >= '0' && value[i] <= '9') {
                hexValue = hexValue * 16 + (value[i] - '0');
            } else if (value[i] >= 'A' && value[i] <= 'F') {
                hexValue = hexValue * 16 + (value[i] - 'A' + 10);
            }
        }
        
        // Store the parsed hexadecimal value in the variable
        *reinterpret_cast<uint16_t*>(variables[index].ptr) = hexValue;
    }
}


// Function to show the variables
void showVariables() {
    for (int i = 0; i < NUM_VARIABLES; i++) {
        Serial.printf(" %-7s: ", variables[i].name);
        
        // Cast the pointer to the appropriate type before dereferencing
        if (strcmp(variables[i].formatSpecifier, "%s") == 0) {
            // If the format specifier is "%s", cast to char* and print as a string
            Serial.printf(variables[i].formatSpecifier, reinterpret_cast<char*>(variables[i].ptr));
        } else {
            // Otherwise, assume it's an integer and cast to uint16_t*
            Serial.printf(variables[i].formatSpecifier, *reinterpret_cast<uint16_t*>(variables[i].ptr));
        }
        
        Serial.println();
    }
}


String readSerialLine() {
  String input = ""; // Initialize an empty string to store the input

  while (true) { // Continue reading until a newline character is encountered
    if (Serial.available()) { // Check if data is available to read
      char c = Serial.read(); // Read a character from the serial buffer
      if (c == '\n' || c == '\r') { // If newline character is encountered, exit the loop
        Serial.println(); // Print newline character to move cursor to the next line
        break;
      } else if (c == '\b') { // Handle backspace
        if (input.length() > 0) { // Check if input string is not empty
          input.remove(input.length() - 1); // Remove the last character from input string
          Serial.write(0x08); // Move cursor back
          Serial.write(' '); // Clear the character
          Serial.write(0x08); // Move cursor back again
        }
      } else if (c == ESC_KEY) { // handle escape sequence, combination keys etc...
        // do nothing, ignore the escape character.
      } else { // Otherwise, append the character to the input string and echo it back
        if (input.length() < MAX_INPUT_LENGTH - 1) { // Check if input length is within limit
          c = toupper(c); // make sure its capitalized
          input += c;
          Serial.write(c); // Echo the character back to the serial interface
        }
      }
    }
    delay(10); // Add a small delay to avoid busy-waiting
  }

  return input; // Return the input string
}

static void cmd_help (const char* variable, const char* value) { 
    Serial.println(F("vCMOS Commands"));
    Serial.println(F("--------------"));
    Serial.println(F("You can do:"));
    Serial.println();
    Serial.println(F("CD       - Change directory off of root. Example commands:"));
    Serial.println(F("            - CD ..     | go to root directory"));
    Serial.println(F("            - CD <NAME> | go to directory"));
    Serial.println(F("              (subdirectorys not supported)"));
    Serial.println(F("COMMIT   - Commit any pending changes to EEPROM in AVR"));
    Serial.println(F("DIR      - List all files in current directory"));
    Serial.println(F("HELP     - Display help information"));
    Serial.println(F("QUIT     - Exit the program"));
    Serial.println(F("SET      - Define a system variable with a hexadecimal address value or string"));
    Serial.println(F("            Usage:   SET <VARIABLE> {ADDRESS} | {STRING}"));
    Serial.println(F("            Example: SET START C000"));
    Serial.println(F("                     SET FILE BIOS.ROM"));
    Serial.println(F("SHOW     - Display all variables"));
    Serial.println();
    Serial.println(F("To display this dialog, type '?' and press <ENTER>"));
}

static void cmd_commit (const char* variable, const char* value) { 
    updateEEPROM();
    Serial.println(F("Committing changes..."));
}

static void cmd_show (const char* variable, const char* value) {
    showVariables();
}

static void cmd_dir (const char* variable, const char* value) { 
    lastDir = dir;  // save for later
    while(true) {
        errCodeSD = scan_files();
        if (errCodeSD) { 
            printErrSD(2,errCodeSD,NULL);
            break; // fail out error!
        } else if (fno.fname[0] == 0) {
            break; // were done.
        }
        // were not done yet..
        } 
        dir = lastDir; // restore directory
}

static void cmd_cd  (const char* variable, const char* value) {  
    errCodeSD = change_dir(variable);
        if (errCodeSD) {
            printErrSD(2,errCodeSD,variable);
        
        }
}

static void cmd_set(const char* variable, const char* value) {
    int found = 0;
    for (int i = 0; i < NUM_VARIABLES; i++) {
        if (strcmp(variable, variables[i].name) == 0) {
            setVariable(i, value);
            found = 1;
            break;
        }
    }

    if (!found) {
        Serial.println(F("Unknown variable"));
    }
}


// this is the command processor
static uint8_t commandProcessor(const char* cmd, const char* variable, const char* value) {
    if (strcmp(cmd, "QUIT") == 0) {
       Serial.println(F("Quitting..."));
       return(1); // we're done.
    }
    for (int i = 0; i < sizeof(commands)/sizeof(Command); i++) {
        if (strcmp(cmd, commands[i].name) == 0) {
            commands[i].function(variable, value);
            return(0); // we're not done 
        }
        
    }
    
    Serial.println(F("Unknown command"));
    return (0); // we're not done
}

// Function to parce the command line
uint8_t processCommand(const String& command) {
    char cmd[VAR_SYMBOL_LENGTH];
    char variable[VAR_SYMBOL_LENGTH];
    char value[MAX_FN_LENGTH];
/*
    if (command.length() == 0) {
      // empty command do nothing 
      return 0;
    }
*/
    // edge case fixes
    switch (command.length()) {
      
      // empty command
      case 0:
        // nothing to do return to command processor prompt
        return 0;
        
      // "CD" or "CD " with no additional information
      case 2:  // fall through to 3 if needed
      case 3:
        if ((command[0] == 'C') && (command[1] == 'D')) {
          printErrSD(2,3,NULL);  // no or missing file (directory) info from command processor
          return 0;
        }
        break;
    }
    
    sscanf(command.c_str(), "%s %s %s", cmd, variable, value);
   
    return (commandProcessor(cmd,variable,value)); // process the command through a jump table.
}    

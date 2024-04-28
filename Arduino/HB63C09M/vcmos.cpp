// Virtual CMOS Settings Management for the HB63C09M

// see LICENCE for details
// vcmos.cpp

#include <Arduino.h>
#include <stdint.h>
#include <string.h>
#include "vcmos.h"
#include "PetitFS.h"
#include "const.h" 

// Structure to represent a variable
typedef struct {
    char name[20];
    void *ptr;
    char formatSpecifier[5];
} Variable;


// Variable table
Variable variables[NUM_VARIABLES] = {
    {"START", &biosStart, "%X"},
    {"SIZE", &biosSize, "%X"},
    {"FILE", &biosName, "%s"}
};

FRESULT scan_files(char *path)
{
    FRESULT res;
    FILINFO fno;
    DIR dir;
    int filesPrintedOnLine = 0;

    res = pf_opendir(&dir, path);
    if (res == FR_OK)
    {
        while (true)
        {
            res = pf_readdir(&dir, &fno);
            if (res != FR_OK || fno.fname[0] == 0) {
                Serial.println();
                break;
            }
            if (fno.fattrib & AM_HID)
                continue;

            // Print directory name with []
            if (fno.fattrib & AM_DIR)
            {
                Serial.print("[");
                Serial.printf("%s]",fno.fname);
                // Pad with spaces if necessary
                for (int i = strlen(fno.fname); i < MAX_FN_LENGTH-1; i++)
                {
                    Serial.print(" ");
                }
            }
            else // Print file name
            {
                Serial.print(fno.fname);
                // Pad with spaces if necessary
                for (int i = strlen(fno.fname); i < MAX_FN_LENGTH; i++)
                {
                    Serial.print(" ");
                }
            }

            // Move to the next line if required
            filesPrintedOnLine++;
            if (filesPrintedOnLine >= MAX_FILES_PER_LINE)
            {
                Serial.println();
                filesPrintedOnLine = 0;
            }
            else // Print space between file names
            {
                Serial.print(" ");
            }
        }
    }

    return res;
}

// test for a valid hex digit otherwise return false
bool isHex(const String& str) {
    for (size_t i = 0; i < str.length(); i++) {
        char c = str.charAt(i);
        if (!isxdigit(c)) {
            return false;
        }
    }
    return true;
}

void setVariable(int index, const char* value) {
    if (value == nullptr || value[0] == '\0') {
        // Empty or null input value, do nothing
        return;
    }
    
    if (strcmp(variables[index].formatSpecifier, "%s") == 0) {
        // If the format specifier is "%s", copy the string directly
        strncpy(reinterpret_cast<char*>(variables[index].ptr), value, MAX_FN_LENGTH - 1);
        reinterpret_cast<char*>(variables[index].ptr)[MAX_FN_LENGTH - 1] = '\0'; // Ensure null-termination
    } else if (strcmp(variables[index].formatSpecifier, "%X") == 0) {
        // If the format specifier is "%X", parse as hexadecimal
        uint16_t hexValue = 0;
        
        // Check if the input starts with "0x" prefix
        if (strncmp(value, "0X", 2) == 0) {
            // Skip the "0x" prefix and parse the rest as hexadecimal
            value += 2;
        }
        
        if (!isHex(value)) {
            Serial.println(F("Value is not numeric!"));
            return;
        }
        
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


// Function to process the commands
uint8_t processCommand(const String& command) {
    char cmd[10];
    char variable[20];
    char value[MAX_FN_LENGTH];
    uint8_t err; 
    
    if (command.length() == 0) {
      // empty command do nothing 
      return 0;
    }
    
    sscanf(command.c_str(), "%s %s %s", cmd, variable, value);

    // SET
    if (strcmp(cmd, "SET") == 0) {
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

    // DIR
    }  else if (strcmp(cmd, "DIR") == 0) {
        err = scan_files("");
        if (err) { 
          printErrSD(2,err,NULL);
          
        }
        
    // SHOW
    }  else if (strcmp(cmd, "SHOW") == 0) {
        showVariables();
    
    // COMMIT    
    } else if (strcmp(cmd, "COMMIT") == 0) {
        updateEEPROM(biosStart,biosSize,biosName);
        Serial.println(F("Committing changes..."));
    
    // QUIT
    } else if (strcmp(cmd, "QUIT") == 0) {
        Serial.println(F("Quitting..."));
        return(1); // were done
    
    } else if (strcmp(cmd, "?") == 0) {
        Serial.println(F("vCMOS Commands"));
        Serial.println(F("--------------"));
        Serial.println(F("You can do:"));
        Serial.println();
        Serial.println(F("COMMIT   - Commit any pending changes to memory"));
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
    } else {
        Serial.println(F("Unknown command"));
        
    }
    return(0); // were not done yet
}

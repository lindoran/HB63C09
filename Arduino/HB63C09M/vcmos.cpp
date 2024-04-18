// Virtual CMOS Settings Management for the HB63C09M

// see LICENCE for details
// vcmos.cpp

#include <Arduino.h>
#include <stdint.h>
#include <string.h>
#include "vcmos.h"

// Structure to represent a variable
typedef struct {
    char name[20];
    void *ptr;
    char formatSpecifier[5];
} Variable;


// Variable table
Variable variables[NUM_VARIABLES] = {
    {"START", &biosStart, "0x%X"},
    {"SIZE", &biosSize, "0x%X"},
    {"FILE", &biosName, "%s"}
};


void setVariable(int index, const char* value) {
    if (value == nullptr || value[0] == '\0') {
        // Empty or null input value, do nothing
        return;
    }
    
    if (strcmp(variables[index].formatSpecifier, "%s") == 0) {
        // If the format specifier is "%s", copy the string directly
        strncpy(reinterpret_cast<char*>(variables[index].ptr), value, MAX_FN_LENGTH - 1);
        reinterpret_cast<char*>(variables[index].ptr)[MAX_FN_LENGTH - 1] = '\0'; // Ensure null-termination
    } else if (strcmp(variables[index].formatSpecifier, "0x%X") == 0) {
        // If the format specifier is "0x%X", parse as hexadecimal
        uint16_t hexValue = 0;

        // Check if the input starts with "0x" prefix
        if (strncmp(value, "0x", 2) == 0) {
            // Skip the "0x" prefix and parse the rest as hexadecimal
            value += 2;
        }
        Serial.printf("%s \n\r",value);
        for (int i = 0; value[i] != '\0'; ++i) {
            Serial.printf("%X \n\r",hexValue);
            if (value[i] >= '0' && value[i] <= '9') {
                hexValue = hexValue * 16 + (value[i] - '0');
            } else if (value[i] >= 'A' && value[i] <= 'F') {
                hexValue = hexValue * 16 + (value[i] - 'A' + 10);
            } else if (value[i] >= 'a' && value[i] <= 'f') {
                hexValue = hexValue * 16 + (value[i] - 'a' + 10);
            }
        }
        // Store the parsed hexadecimal value in the variable
        *reinterpret_cast<uint16_t*>(variables[index].ptr) = hexValue;
    }
}

// Function to show the variables
void showVariables() {
    for (int i = 0; i < NUM_VARIABLES; i++) {
        Serial.printf("%s: ", variables[i].name);
        
        // Cast the pointer to the appropriate type before dereferencing
        if (strcmp(variables[i].formatSpecifier, "%s") == 0) {
            // If the format specifier is "%s", cast to char* and print as a string
            Serial.printf(variables[i].formatSpecifier, reinterpret_cast<char*>(variables[i].ptr));
        } else {
            // Otherwise, assume it's an integer and cast to uint16_t*
            Serial.printf(variables[i].formatSpecifier, *reinterpret_cast<uint16_t*>(variables[i].ptr));
        }
        
        Serial.printf("\n\r");
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
      } else { // Otherwise, append the character to the input string and echo it back
        if (input.length() < MAX_INPUT_LENGTH - 1) { // Check if input length is within limit
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

    if (command.length() == 0) {
      // empty command do nothing 
      return 0;
    }
    
    sscanf(command.c_str(), "%s %s %s", cmd, variable, value);

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
            Serial.printf("Unknown variable\n\r");
        }
    }  else if (strcmp(cmd, "SHOW") == 0) {
        showVariables();
        
    } else if (strcmp(cmd, "COMMIT") == 0) {
        updateEEPROM(biosStart,biosSize,biosName);
        Serial.printf("Committing changes...\n\r");
        return(1); // were done
    } else if (strcmp(cmd, "QUIT") == 0) {
        Serial.printf("Quitting without changes...\n\r");
        return(1); // were done
    } else {
        Serial.printf("Unknown command\n\r");
        
    }
    return(0); // were not done yet
}

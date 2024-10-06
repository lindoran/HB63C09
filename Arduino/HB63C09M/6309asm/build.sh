#!/bin/bash

# Function to clean build files
clean() {
  rm -f *.listing *.bld *.h
  echo "Cleaned up build files."
}

# Function to install the .h file
install() {
  cp blockcopy.h ../
  echo "Installed blockcopy.h to the parent directory."
}

# Check if the first argument is "clean" or "install"
if [ "$1" == "clean" ]; then
  clean
  exit 0
elif [ "$1" == "install" ]; then
  install
  exit 0
fi

# Check if asl is installed
if ! command -v asl &> /dev/null; then
  echo "asl not found. Please download it from: http://john.ccac.rwth-aachen.de:8000/as/"
  exit 1
fi

# Check if p2hex is installed
if ! command -v p2hex &> /dev/null; then
  echo "p2hex (a part of asl) not found. Please download and install from http://john.ccac.rwth-aachen.de:8000/as/"
  exit 1
fi

# Assemble the blockcopy.asm file
asl_output=$(asl blockcopy.asm -CPU 6309 -L -olist blockcopy.listing -o blockcopy.bld 2>&1)
echo "$asl_output"

# Check for errors in the asl output
if echo "$asl_output" | grep -q "errors" && ! echo "$asl_output" | grep -q "0 errors"; then
  echo "Assembly failed with errors."
  exit 1
fi

# Convert the output to hex format
p2hex_output=$(p2hex blockcopy.bld blockcopy.h -F c 2>&1)
echo "$p2hex_output"

# Check for specific output in the p2hex output
if ! echo "$p2hex_output" | grep -q "Deduced address range: 0x0000FFC0-0x0000FFED" || ! echo "$p2hex_output" | grep -q "blockcopy.bld==>>blockcopy.h (46 Bytes)"; then
  echo "Hex conversion failed with errors."
  exit 1
fi

# Create a temporary file to store the modified content
temp_file=$(mktemp)

# Read the blockcopy.h file and insert the specified line and the contents of blockcopy.listing after the first instance of #define _blockcopy_H
awk '
  BEGIN { found = 0 }
  {
    print $0
    if (!found && $0 ~ /#define _blockcopy_H/) {
      found = 1
      print ""
      print "/*"
      print "blockcopy.asm listing below for reference, please see the c code below the comments"
      while ((getline line < "blockcopy.listing") > 0) {
        print line
      }
      print "*/"
    }
  }
' blockcopy.h > "$temp_file"

# Replace the original blockcopy.h with the modified content
mv "$temp_file" blockcopy.h

echo "Build process completed successfully."

#!/bin/bash

# Build a cropped ROM file

# Default values (for modified Jeff Tranter combo ROM)
# Default crop range start address
default_crop_start="0xC000"
# Default crop range end address, this should be one address value greater than
# the end of the cropped range, because srec_cat needs it that way.
default_crop_end="0x10000"
# Default offset value. This moves the code to the bottom of the ROM. Value is
# negative.
default_offset="-0xC000"

# Initialize variables with default values
crop_start="$default_crop_start"
crop_end="$default_crop_end"
offset="$default_offset"
input_file=""

# Function to display usage information
usage() {
    echo "Usage: $0 [-s <crop_start_address>] [-e <crop_end_address>] [-o <offset_value>] <input_file>"
    echo
    echo "Options:"
    echo "  -s, <crop_start_address>  Specify the starting address for cropping the binary file."
    echo "  -e, <crop_end_address>    Specify the ending address for cropping the binary file."
    echo "  -o, <offset_value>        Specify the offset value to apply during the conversion."
    echo "  -h, --help                Show this help message and exit."
    echo "  clean                     Clean up intermediate files (bin, s19, crf, lst, sym)."
    exit 1
}

# Clean up intermediate files
clean_files() {
    echo "Cleaning up intermediate files..."
    rm -f *.s19 *.bin *.crf *.lst *.sym
    echo "Done."
    exit 0
}

# Parse command line options
if [ "$#" -eq 1 ]; then
    if [ "$1" == "clean" ]; then
        clean_files
    else
        input_file="$1"
    fi
elif [ "$#" -eq 7 ]; then
    while getopts ":s:e:o:" opt; do
        case ${opt} in
            s )
                crop_start="$OPTARG"
                ;;
            e )
                crop_end="$OPTARG"
                ;;
            o )
                offset="$OPTARG"
                ;;
            \? )
                echo "Invalid option: $OPTARG" 1>&2
                usage
                ;;
            : )
                echo "Invalid option: $OPTARG requires an argument" 1>&2
                usage
                ;;
        esac
    done
    shift $((OPTIND -1))
    input_file="$1"
elif [ "$#" -eq 0 ]; then
    usage
else
    usage
fi


# Check if all required arguments are provided
if [ -z "$input_file" ]; then
    echo "Input file argument is required."
    usage
fi

# Extract filename without extension
filename=$(basename -- "$input_file")
filename_no_ext="${filename%.*}"

# Assemble the ASM file if it exists
asm_file="${filename_no_ext}.asm"
if [ -f "$asm_file" ]; then
    echo "Assembling ASM file: $asm_file"
    as9 "$asm_file" -l c s s19 cre now
fi

# Build binary file using srec_cat command
srec_cat "${filename_no_ext}.s19" -crop "$crop_start" "$crop_end" -offset "$offset" -o "${filename_no_ext}.bin" -binary

echo "Binary file created: ${filename_no_ext}.bin"
echo "Crop range: $crop_start-$crop_end"
echo "Offset: $offset"
echo "File size: $(stat -c %s "${filename_no_ext}.bin") bytes"

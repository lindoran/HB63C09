# Makefile to build and install cropped ROM binary

# Default crop and offset values
CROP_START ?= 0xC000
CROP_END   ?= 0x10000
OFFSET     ?= -0xC000

# Default input file
DEFAULT_INPUT := ASSIST09_Original_ASM6809
INPUT         := $(if $(filter-out clean install,$(firstword $(MAKECMDGOALS))),$(basename $(firstword $(MAKECMDGOALS))),$(DEFAULT_INPUT))

# File names
ASM_FILE := $(INPUT).asm
S19_FILE := $(INPUT).s19
BIN_FILE := $(INPUT).bin
LST_FILE := $(INPUT).txt
SYM_FILE := $(INPUT).sym

# Install target destination
INSTALL_DIR := ../ROMS
INSTALL_FILE := $(INSTALL_DIR)/ASSIST09.BIN

# Default target
all: $(BIN_FILE)

# Rule to generate S19 from ASM if ASM file exists
$(S19_FILE): $(ASM_FILE)
	@echo "Assembling $(ASM_FILE)..."
	asm6809 -o $(S19_FILE) -l $(LST_FILE) -s $(SYM_FILE) --srec $(ASM_FILE)

# Rule to generate BIN from S19
$(BIN_FILE): $(S19_FILE)
	@echo "Creating binary from $(S19_FILE)..."
	srec_cat $(S19_FILE) -crop $(CROP_START) $(CROP_END) -offset $(OFFSET) -o $(BIN_FILE) -binary
	@echo "Binary file created: $(BIN_FILE)"
	@echo "Crop range: $(CROP_START)-$(CROP_END)"
	@echo "Offset: $(OFFSET)"
	@echo -n "File size: "; stat -c %s $(BIN_FILE); echo " bytes"

# Install target
.PHONY: install
install: $(BIN_FILE)
	@echo "Installing $(BIN_FILE) to $(INSTALL_FILE)..."
	@mkdir -p $(INSTALL_DIR)
	@cp $(BIN_FILE) $(INSTALL_FILE)
	@echo "Installation complete."

# Clean target
.PHONY: clean
clean:
	@echo "Cleaning up..."
	@rm -f *.s19 *.bin *.crf *.lst *.sym *.txt
	@echo "Done."

# Prevent make from interpreting file names as targets
%:
	@:

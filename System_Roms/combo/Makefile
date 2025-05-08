# Makefile for assembling combo-hb6309m.asm and creating 16K ROM at $C000

AS = as9
SRC = combo-hb6309m.asm
BASE = $(basename $(SRC))

# Output files
CRF = $(BASE).crf
LST = $(BASE).lst
S19 = $(BASE).s19
SYM = $(BASE).sym
BIN = $(BASE).bin

# ROM install path
INSTALL_DIR = ../ROMS
INSTALL_FILE = $(INSTALL_DIR)/COMBO.BIN

# Address range for cropping
CROP_START = 0xC000
CROP_END   = 0x10000
OFFSET     = -0xC000

all: $(BIN)

# Assemble with as9
$(S19) $(LST) $(SYM) $(CRF): $(SRC)
	$(AS) $(SRC) -l c s s19 cre

# Build 16K ROM image by cropping and offsetting the top 16K
$(BIN): $(S19)
	srec_cat $(S19) \
		-crop $(CROP_START) $(CROP_END) \
		-offset $(OFFSET) \
		-fill 0xFF 0x0000 0x3FFF \
		-o $(BIN) -binary

# Install the binary to ../ROMS/COMBO.BIN
.PHONY: install
install: $(BIN)
	@echo "Installing $(BIN) to $(INSTALL_FILE)..."
	@mkdir -p $(INSTALL_DIR)
	@cp $(BIN) $(INSTALL_FILE)
	@echo "Installation complete."

# Clean build artifacts
.PHONY: clean
clean:
	@echo "Cleaning up..."
	@rm -f $(CRF) $(LST) $(S19) $(SYM) $(BIN)
	@echo "Done."


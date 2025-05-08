# Assembler flags
LWASM_FLAGS=--6309 --tabs=8 -f srec
LWASM_BIN_FLAGS=--6309 --tabs=8 -f decb

# Source files (without extension)
SRC_FILES = test
BIN_FILE = qload

# Default target
all:  assist09-build $(SRC_FILES:=.srec) $(BIN_FILE).srec $(BIN_FILE).bin


.PHONY: assist09-build install

assist09-build:
	@echo "Building assist09 with its own toolchain..."
	@$(MAKE) -C assist09



# SREC build rules
%.srec: %.asm
	@echo "Assembling $< to SREC..."
	@lwasm $(LWASM_FLAGS) \
		--list=$*.lst \
    	--symbol-dump=$*.sym \
    	--output=$@ \
    	$< || (echo "Error assembling $<" && exit 1)

# BIN build rule for Flex_loader only
$(BIN_FILE).bin: $(BIN_FILE).asm
	@echo "Assembling $< to BIN..."
	@lwasm $(LWASM_BIN_FLAGS) \
    	--list=$(BIN_FILE).lst \
    	--symbol-dump=$(BIN_FILE).sym \
		--format=abs \
    	--output=$@ \
    	$< || (echo "Error assembling $<" && exit 1)
	@echo "Binary output: $@"
	@truncate -s 256 $(BIN_FILE).bin

    # Extract load address from .sym file
	@LOAD_ADDR=`awk '/LOAD EQU/ { print $$NF }' $(BIN_FILE).sym` && \
	SIZE=`wc -c < $@` && \
	echo "  Load address: $$LOAD_ADDR" && \
	echo "  Size        : $$SIZE bytes"

    # Optional: dump first few lines of bin as hex
	@echo "  Hexdump     : $@"
	@xxd -g1 -c16 -u $@ | head -n 16


install:
	@echo "Copy qload.bin to bootsector.bin..."
	@cp qload.bin ./flextools/disks/target_lflex/bootsector.bin
	@echo "building disksets..."
	cd flextools/disks && ./build_disks.sh
	cp flextools/disks/lflex_disks/tscflex09.lflex.img ../ROMS/FLPY00.DSK
	cp flextools/disks/lflex_disks/basic.lflex.img ../ROMS/FLPY01.DSK
	cp flextools/disks/lflex_disks/eforth.lflex.img ../ROMS/FLPY02.DSK
	cp flextools/disks/lflex_disks/dynastar.lflex.img ../ROMS/FLPY03.DSK
	


# Clean target
clean:
	@echo "Cleaning up top-level build artifacts..."
	@rm -f *.lst *.sym *.srec *.bin
	@echo "Cleaning assist09 build artifacts..."
	@$(MAKE) -C assist09 clean
	@echo "Cleanup complete."
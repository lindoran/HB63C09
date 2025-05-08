# === Configuration ===
ROM_START  := 0xC000
ROM_END    := 0x10000       # Exclusive end for srec_cat
ROM_OFFSET := -0xC000       # To move $F700 down to address 0 in the binary
OUT_NAME   := A9FLEX

ASM_SRCS   := $(wildcard *.asm)
S19_FILES  := $(ASM_SRCS:.asm=.s19)
LST_FILES  := $(ASM_SRCS:.asm=.lst)
SYM_FILES  := $(ASM_SRCS:.asm=.sym)

MERGED_S19 := $(OUT_NAME).merged.s19
FINAL_BIN  := $(OUT_NAME).BIN

# === Default target ===
all: $(FINAL_BIN)

# === Assemble .asm files to .s19
%.s19: %.asm
	@echo "Assembling $<"
	asm6809 -o $@ -l $(@:.s19=.lst) -s $(@:.s19=.sym) --srec $<

# === Merge all S19 files
$(MERGED_S19): $(S19_FILES)
	@echo "Merging S19 files..."
	srec_cat $(S19_FILES) -o $@ 

# === Generate binary from merged S19, cropping ROM area only
$(FINAL_BIN): $(MERGED_S19)
	@echo "Creating binary ROM image from $<..."
	srec_cat $< -crop $(ROM_START) $(ROM_END) -offset $(ROM_OFFSET) -o $@ -binary
	@if [ -f $@ ]; then \
		echo "Binary created: $@ 0x$$(printf '%x' $$(stat -c %s $@)) bytes at $(ROM_START)"; \
	else \
		echo "Error: binary file not created."; \
	fi

install:
	@echo "Copy bin file to ROMS folder for dist."
	@cp *.BIN ../../ROMS
	@echo "Complete!"



# === Clean up
clean:
	@echo "Cleaning intermediate files..."
	@rm -f $(S19_FILES) $(LST_FILES) $(SYM_FILES) $(MERGED_S19) $(FINAL_BIN)
	@echo "Clean complete."

# === Help
help:
	@echo "Usage:"
	@echo "  make           - Build ROM image from all .asm files"
	@echo "  make clean     - Remove intermediate files"
	@echo
	@echo "Settings:"
	@echo "  ROM_START=$(ROM_START)"
	@echo "  ROM_END=$(ROM_END)"
	@echo "  ROM_OFFSET=$(ROM_OFFSET)"
	@echo "  OUT_NAME=$(OUT_NAME)"

.PHONY: all clean help

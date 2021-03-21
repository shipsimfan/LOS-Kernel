# DIRECTORIES
SRC_DIR := ./src
TARGET_DIR := ./target
OBJ_DIR := $(TARGET_DIR)/obj
BIN_DIR := ./bin
SYSROOT_DIR := ../sysroot

# TARGET
KERNEL := $(BIN_DIR)/kernel.elf

# SOURCE FILES
ASM_SRC_FILES := $(shell find $(SRC_DIR) -name '*.asm')
LINK_FILE := ./link.ld

# OBJECT FILES
RUST_TARGET := ./target/x86_64-los/debug/libkernel.rlib
ASM_OBJ_FILES := $(ASM_SRC_FILES:$(SRC_DIR)/%.asm=$(OBJ_DIR)/%.o)

# PROGRAMS
ASM := nasm
ASM_FLAGS := -f elf64 -g -F dwarf

LD := x86_64-los-ld
LD_FLAGS := -z max-page-size=0x1000 -T $(LINK_FILE) -g
LD_POST_FLAGS := -nostdlib

# BASE RULES
all: install
	@echo "[ KERNEL ] Build complete!"

install: dirs $(KERNEL)
	@cp $(KERNEL) $(SYSROOT_DIR)/
	@echo "[ KERNEL ] Installed!"

# TARGET RULE
$(KERNEL): $(RUST_TARGET) $(ASM_OBJ_FILES)
	@echo "[ KERNEL ] (LD) $@ . . ."
	@$(LD) $(LD_FLAGS) -o $@ $^ $(LD_POST_FLAGS)

# INTERMEDIATE RULES
.SECONDEXPANSION:

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.asm | $$(@D)/.
	@echo "[ KERNEL ] (ASM) $^ . . ."
	@$(ASM) $(ASM_FLAGS) -o $@ $^

$(RUST_TARGET):
	@cargo build

# DIRECTORY RULES
$(OBJ_DIR)/.:
	@mkdir -p $@

$(OBJ_DIR)%/.:
	@mkdir -p $@

dirs:
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(OBJ_DIR)

# . RULES
.PRECIOUS: $(OBJ_DIR)/. $(OBJ_DIR)%/.
.PHONY: dirs $(RUST_TARGET)
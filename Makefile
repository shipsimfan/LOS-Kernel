# DIRECTORIES
BIN_DIR := ./bin
OBJ_DIR := ./obj
SRC_DIR := ./src

# TARGET
KERNEL := $(BIN_DIR)/kernel.elf

# SOURCE FILES
CPP_SRC_FILES := $(shell find $(SRC_DIR) -name '*.cpp')
ASM_SRC_FILES := $(shell find $(SRC_DIR) -name '*.asm')
C_SRC_FILES := $(shell find $(SRC_DIR) -name '*.c')

LINK_FILE := ./link.ld

# OBJECT FILES
CPP_OBJ_FILES := $(CPP_SRC_FILES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
ASM_OBJ_FILES := $(ASM_SRC_FILES:$(SRC_DIR)/%.asm=$(OBJ_DIR)/%.o)
C_OBJ_FILES := $(C_SRC_FILES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# PROGRAMS
CPP := x86_64-elf-g++
CPP_FLAGS := -ffreestanding -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -c -Wall -I./include -I./stdinc -D DEBUG -g

ASM := nasm
ASM_FLAGS := -f elf64 -g -F dwarf

CC := x86_64-elf-gcc
CC_FLAGS := $(CPP_FLAGS)

LD := x86_64-elf-g++
LD_FLAGS := -ffreestanding -z max-page-size=0x1000 -T $(LINK_FILE) -g
LD_POST_FLAGS := -nostdlib -lgcc

# BASE RULES
all: dirs $(KERNEL)

clean:
	rm -rf $(OBJ_DIR)/*
	rm -rf $(BIN_DIR)/*

# COMPILATION RULES
.SECONDEXPANSION:

$(KERNEL): $(CPP_OBJ_FILES) $(ASM_OBJ_FILES) $(C_OBJ_FILES)
	$(LD) $(LD_FLAGS) -o $@ $^ $(LD_POST_FLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $$(@D)/.
	$(CPP) $(CPP_FLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.asm | $$(@D)/.
	$(ASM) $(ASM_FLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $$(@D)/.
	$(CC) $(CC_FLAGS) -o $@ $^

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
.PHONY: dirs
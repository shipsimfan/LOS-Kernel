# DIRECTORIES
BIN_DIR := ./bin
OBJ_DIR := ./obj
SRC_DIR := ./src
ACPICA_DIR := ./acpica
ACPICA_OBJ_DIR := $(ACPICA_DIR)/obj

# TARGET
KERNEL := $(BIN_DIR)/kernel.elf

# SOURCE FILES
CPP_SRC_FILES := $(shell find $(SRC_DIR) -name '*.cpp')
ASM_SRC_FILES := $(shell find $(SRC_DIR) -name '*.asm')
ACPICA_SRC_FILES := $(shell find $(ACPICA_DIR) -name '*.cpp')

LINK_FILE := ./link.ld

# OBJECT FILES
CPP_OBJ_FILES := $(CPP_SRC_FILES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
ASM_OBJ_FILES := $(ASM_SRC_FILES:$(SRC_DIR)/%.asm=$(OBJ_DIR)/%.o)
ACPICA_OBJ_FILES := $(ACPICA_SRC_FILES:$(ACPICA_DIR)/%.cpp=$(ACPICA_OBJ_DIR)/%.o)

# PROGRAMS
CPP := x86_64-los-g++
CPP_FLAGS := -ffreestanding -fomit-frame-pointer -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -fno-exceptions -fno-rtti -c -Wall -I./include  -D DEBUG -g

ASM := nasm
ASM_FLAGS := -f elf64 -g -F dwarf

LD := x86_64-los-g++
LD_FLAGS := -ffreestanding -z max-page-size=0x1000 -T $(LINK_FILE) -g
LD_POST_FLAGS := -nostdlib -lgcc

# BASE RULES
all: dirs $(KERNEL)
	@echo "[ KERNEL ] Build Complete!"

clean:
	@rm -rf $(OBJ_DIR)/*
	@rm -rf $(BIN_DIR)/*
	@echo "[ KERNEL ] Cleaned!"

clean-acpica:
	@rm -rf $(ACPICA_OBJ_DIR)/*
	@echo "[ ACPICA ] Cleaned!"

# COMPILATION RULES
.SECONDEXPANSION:

$(KERNEL): $(C_OBJ_FILES) $(CPP_OBJ_FILES) $(ASM_OBJ_FILES) $(ACPICA_OBJ_FILES)
	@echo "[ KERNEL ] (LD) $@ . . ."
	@$(LD) $(LD_FLAGS) -o $@ $^ $(LD_POST_FLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $$(@D)/.
	@echo "[ KERNEL ] (CPP) $^ . . ."
	@$(CPP) $(CPP_FLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.asm | $$(@D)/.
	@echo "[ KERNEL ] (ASM) $^ . . ."
	@$(ASM) $(ASM_FLAGS) -o $@ $^

$(ACPICA_OBJ_DIR)/%.o: $(ACPICA_DIR)/%.cpp
	@echo "[ ACPICA ] (CPP) $^ . . ."
	@$(CPP) $(CPP_FLAGS) -o $@ $^


# DIRECTORY RULES
$(OBJ_DIR)/.:
	@mkdir -p $@

$(OBJ_DIR)%/.:
	@mkdir -p $@

dirs:
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(ACPICA_OBJ_DIR)

# . RULES
.PRECIOUS: $(OBJ_DIR)/. $(OBJ_DIR)%/.
.PHONY: dirs
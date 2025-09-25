# Compiler settings
CC = gcc
CFLAGS = -I$(INCLUDE_DIR)
LDFLAGS =
AS = nasm
ASFLAGS = -f elf64  # Use -f win64 for Windows
LD = ld

# Directories
SRC_DIR = src
INCLUDE_DIR = include
OBJ_DIR = obj
COMPILER_DIR = compiler
ASM_LIB_DIR = libraries/asm_user_libs
OBJ_LIB_DIR = libraries/obj_user_libs
OUTPUT_DIR = output

# Files
TARGET = bjornc
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))
ASM_LIB_FILES = $(wildcard $(ASM_LIB_DIR)/*.asm)
OBJ_LIB_FILES = $(patsubst $(ASM_LIB_DIR)/%.asm,$(OBJ_LIB_DIR)/%.o,$(ASM_LIB_FILES))

# Default rule
.PHONY: all
all: $(COMPILER_DIR)/$(TARGET) compile_libs compile build_exe

# Rule 1: Build the bjornc compiler
$(COMPILER_DIR)/$(TARGET): $(OBJ_FILES) | $(COMPILER_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Compile .c files to .o files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Rule 2: Compile .asm files to .o files in libraries/obj_user_libs/
compile_libs: $(OBJ_LIB_FILES)

$(OBJ_LIB_DIR)/%.o: $(ASM_LIB_DIR)/%.asm | $(OBJ_LIB_DIR)
	$(AS) $(ASFLAGS) $< -o $@

# Rule 3: Compile .bjo script to .asm
compile: $(COMPILER_DIR)/$(TARGET) | $(OUTPUT_DIR)
	./$(COMPILER_DIR)/$(TARGET) $(SCRIPT) $(OUTPUT_DIR)/$(OUTPUT_SCRIPT)

# Rule 4: Build the final executable
build_exe: $(OUTPUT_DIR)/$(OUTPUT_SCRIPT).asm | $(OUTPUT_DIR)
	$(AS) $(ASFLAGS) $< -o $(OUTPUT_DIR)/$(OUTPUT_SCRIPT).o
	$(LD) $(LDFLAGS) $(OUTPUT_DIR)/$(OUTPUT_SCRIPT).o $(OBJ_LIB_FILES) -o $(OUTPUT_DIR)/$(OUTPUT_SCRIPT)

# Rule 5: Run the final executable 
run_exe: $(OUTPUT_DIR)/$(OUTPUT_SCRIPT).asm | $(OUTPUT_DIR)
	./$(OUTPUT_DIR)/$(OUTPUT_SCRIPT)


# Directory creation
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(COMPILER_DIR):
	mkdir -p $(COMPILER_DIR)

$(OBJ_LIB_DIR):
	mkdir -p $(OBJ_LIB_DIR)

$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

# Clean rules
.PHONY: clean clean_bjorn clean_libs clean_output
clean: clean_bjorn clean_libs clean_output

clean_bjorn:
	rm -rf $(OBJ_DIR) $(COMPILER_DIR)

clean_libs:
	rm -rf $(OBJ_LIB_DIR)

clean_output:
	rm -rf $(OUTPUT_DIR)









#compiling  make SCRIPT=test.bjo OUTPUT_SCRIPT=test
#running ./output/test
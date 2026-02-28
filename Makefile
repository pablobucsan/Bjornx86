# Compiler settings
CC = gcc
CFLAGS = -I$(INCLUDE_DIR)  
# CFLAGS = -I$(INCLUDE_DIR)
LDFLAGS =
AS = nasm
ASFLAGS = -f elf64
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

# Build compiler
$(COMPILER_DIR)/$(TARGET): $(OBJ_FILES) | $(COMPILER_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile asm libs
compile_libs: $(OBJ_LIB_FILES)

$(OBJ_LIB_DIR)/%.o: $(ASM_LIB_DIR)/%.asm | $(OBJ_LIB_DIR)
	$(AS) $(ASFLAGS) $< -o $@

# === Generate .asm ===
# We tell bjornc to write:   $(OUTPUT_DIR)/$(OUTPUT_SCRIPT)
compile: $(COMPILER_DIR)/$(TARGET) | $(OUTPUT_DIR)
	./$(COMPILER_DIR)/$(TARGET) -nl $(SCRIPT) $(OUTPUT_DIR)/$(OUTPUT_SCRIPT)

# === Assemble ===
# Input:  $(OUTPUT_DIR)/$(OUTPUT_SCRIPT).asm
# Output: $(OUTPUT_DIR)/$(OUTPUT_SCRIPT).o
$(OUTPUT_DIR)/$(OUTPUT_SCRIPT).o: $(OUTPUT_DIR)/$(OUTPUT_SCRIPT).asm | $(OUTPUT_DIR)
	$(AS) $(ASFLAGS) $< -o $@

# === Link ===
# Input:  $(OUTPUT_DIR)/$(OUTPUT_SCRIPT).o + library objects
# Output: $(OUTPUT_DIR)/$(OUTPUT_SCRIPT)   ← final exe, no extension
build_exe: $(OUTPUT_DIR)/$(OUTPUT_SCRIPT).o $(OBJ_LIB_FILES) | $(OUTPUT_DIR)
	$(LD) $(LDFLAGS) $< $(OBJ_LIB_FILES) -o $(OUTPUT_DIR)/$(OUTPUT_SCRIPT)

# Convenience target: full pipeline
.PHONY: exe
exe: build_exe

# Run the result
run_exe: build_exe
	./$(OUTPUT_DIR)/$(OUTPUT_SCRIPT)

# Directory creation
$(OBJ_DIR) $(COMPILER_DIR) $(OBJ_LIB_DIR) $(OUTPUT_DIR):
	mkdir -p $@

# Clean rules
.PHONY: clean clean_bjorn clean_libs clean_output
clean: clean_bjorn clean_libs clean_output

clean_bjorn: ; rm -rf $(OBJ_DIR) $(COMPILER_DIR)
clean_libs:  ; rm -rf $(OBJ_LIB_DIR)
clean_output:; rm -rf $(OUTPUT_DIR)

# Deployment (auto-link version)
.PHONY: deploy
deploy: all compile_libs
	@echo "=== Deploying Bjorn compiler and standard libraries ==="
	@mkdir -p $(INSTALL_BIN_DIR) $(INSTALL_STD_DIR) $(INSTALL_OBJ_DIR)
	@cp $(COMPILER_DIR)/$(TARGET) $(INSTALL_BIN_DIR)/bjornc
	@chmod +x $(INSTALL_BIN_DIR)/bjornc
	@cp -r $(wildcard libraries/user_libs/*.bjo) $(INSTALL_STD_DIR)/
	@cp -r $(wildcard $(OBJ_LIB_DIR)/*.o) $(INSTALL_OBJ_DIR)/ 2>/dev/null || true
	@echo "=== Deployment complete! ==="
	@echo "Compiler installed to $(INSTALL_BIN_DIR)/bjornc"
	@echo "Standard library sources copied to $(INSTALL_STD_DIR)/"
	@echo "Compiled extern objects copied to $(INSTALL_OBJ_DIR)/"

	
INSTALL_BIN_DIR  := $(HOME)/bin
INSTALL_STD_DIR  := $(HOME)/bjorn-std
INSTALL_OBJ_DIR  := $(INSTALL_STD_DIR)/obj


#make SCRIPT=test.bjo OUTPUT_SCRIPT=test
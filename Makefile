# === Compiler Settings ===
CC      = gcc -O3
CFLAGS  = -I$(INCLUDE_DIR)

# === Paths ===
COMPILER_DIR = compiler
INCLUDE_DIR  = $(COMPILER_DIR)/include
SRC_DIR      = $(COMPILER_DIR)/src
OBJ_DIR      = $(COMPILER_DIR)/obj

INSTALL_BIN_DIR = /usr/local/bin
INSTALL_LIB_DIR = /usr/local/lib/bjorn

# === Sources & Objects ===
SRC_FILES = $(COMPILER_DIR)/main.c \
            $(wildcard $(SRC_DIR)/backend/*.c) \
            $(wildcard $(SRC_DIR)/frontend/*.c) \
            $(wildcard $(SRC_DIR)/misc/*.c)

OBJ_FILES = $(patsubst $(COMPILER_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_FILES))

TARGET = bjornc2



# === Bjorn Standard Library ===
BJORN_LIB_DIR = bjorn-lib
BJORN_SRC_DIR = $(BJORN_LIB_DIR)/src
BJORN_ASM_DIR = $(BJORN_LIB_DIR)/asm
BJORN_ASM_EXTERN_DIR = $(BJORN_LIB_DIR)/asm_extern
BJORN_CUB_DIR = $(BJORN_LIB_DIR)/cubs


# === Default ===
.PHONY: all
all: build

# === Build Compiler ===
.PHONY: build
build: $(TARGET)

$(TARGET): $(OBJ_FILES)
	$(CC) $(CFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: $(COMPILER_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@


.PHONY: stdlib
stdlib: stdlib-asm stdlib-cub

.PHONY: stdlib-asm
stdlib-asm:
	@echo "=== Compiling .bjo -> .asm ==="
	@mkdir -p $(BJORN_ASM_DIR)
	@rm -rf saved-temps/bjornc
	bjornc2 -s -save-temps $(BJORN_SRC_DIR)/*.bjo
	@cp saved-temps/bjornc/*.asm $(BJORN_ASM_DIR)/
	@echo "=== Done: .asm files in $(BJORN_ASM_DIR) ==="

.PHONY: stdlib-cub
stdlib-cub:
	@echo "=== Assembling .asm -> .cub ==="
	@mkdir -p $(BJORN_CUB_DIR)
	bjornas $(BJORN_ASM_DIR)/*.asm $(BJORN_ASM_EXTERN_DIR)/*.asm -cub $(BJORN_CUB_DIR)
	@echo "=== Done: .cub files in $(BJORN_CUB_DIR) ==="


# === Deploy ===
.PHONY: deploy
deploy: build stdlib
	@echo "=== Deploying bjornc2 compiler ==="
	@mkdir -p $(INSTALL_BIN_DIR)
	@cp $(TARGET) $(INSTALL_BIN_DIR)/bjornc2
	@chmod +x $(INSTALL_BIN_DIR)/bjornc2
	@echo "=== Deploying bjorn-lib ==="
	@rm -rf $(INSTALL_LIB_DIR)
	@mkdir -p $(INSTALL_LIB_DIR)
	@cp -r bjorn-lib/* $(INSTALL_LIB_DIR)/
	@echo "=== Setting BJORN_LIB_PATH ==="
	@if ! grep -q "BJORN_LIB_PATH" $(HOME)/.bashrc; then \
		echo 'export BJORN_LIB_PATH=/usr/local/lib/bjorn' >> $(HOME)/.bashrc; \
		echo "Added BJORN_LIB_PATH to ~/.bashrc"; \
	else \
		echo "BJORN_LIB_PATH already set in ~/.bashrc, skipping"; \
	fi
	@echo "=============================="
	@echo "Done!"
	@echo "Compiler  → $(INSTALL_BIN_DIR)/bjornc2"
	@echo "Libraries → $(INSTALL_LIB_DIR)"
	@echo "Run 'source ~/.bashrc' or open a new terminal to apply BJORN_LIB_PATH"

# === Clean ===
.PHONY: clean
clean:
	rm -rf $(OBJ_DIR) $(TARGET)
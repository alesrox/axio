CC          := gcc
CFLAGS      := -Wall -Wextra -std=c11 -g -I../compiler/include -I../vm/include -I../common/include
BIN_DIR     := bin

# Fuentes excluyendo los main.c de producción
COMPILER_SRCS := $(filter-out ../compiler/src/main.c, $(wildcard ../compiler/src/*.c))
VM_SRCS       := $(filter-out ../vm/src/main.c, $(wildcard ../vm/src/*.c))

# Detección automática de fuentes de test
COMPILER_TEST_SRCS := $(wildcard compiler/test_*.c)
VM_TEST_SRCS       := $(wildcard vm/test_*.c)

# Mapeo a nombres de binarios/targets (ej. compiler/test_lexer.c -> test_lexer)
COMPILER_TEST_NAMES := $(patsubst compiler/%.c, %, $(COMPILER_TEST_SRCS))
VM_TEST_NAMES       := $(patsubst vm/%.c, %, $(VM_TEST_SRCS))

ALL_TEST_NAMES      := $(COMPILER_TEST_NAMES) $(VM_TEST_NAMES)

.PHONY: all compiler vm clean $(ALL_TEST_NAMES)

# Executa todos los tests por defecto
all: compiler vm

# Grupos de tests por módulo
compiler: $(COMPILER_TEST_NAMES)
vm: $(VM_TEST_NAMES)

# --- Reglas para tests del Compilador ---
$(COMPILER_TEST_NAMES): %: compiler/%.c $(COMPILER_SRCS) | $(BIN_DIR)
	@$(CC) $(CFLAGS) $^ -o $(BIN_DIR)/$@
	@echo "==> Running Compiler Test: $@..."
	@./$(BIN_DIR)/$@

# --- Reglas para tests de la VM ---
$(VM_TEST_NAMES): %: vm/%.c $(VM_SRCS) | $(BIN_DIR)
	@$(CC) $(CFLAGS) $^ -o $(BIN_DIR)/$@
	@echo "==> Running VM Test: $@..."
	@./$(BIN_DIR)/$@

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

clean:
	@rm -rf $(BIN_DIR)
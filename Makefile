.PHONY: all compiler vm tests test_compiler_tests test_vm_tests test_lexer test_vm clean help

all: compiler vm

compiler:
	@echo "==> Building Compiler..."
	$(MAKE) -C compiler

vm:
	@echo "==> Building VM..."
	$(MAKE) -C vm

tests:
	@echo "==> Running full test suite..."
	$(MAKE) -C tests

tests_compiler:
	@echo "==> Running all Compiler tests..."
	$(MAKE) -C tests compiler

test_lexer:
	$(MAKE) -C tests test_lexer

clean:
	@echo "==> Cleaning project..."
	$(MAKE) -C compiler clean
	$(MAKE) -C vm clean
	$(MAKE) -C tests clean

help:
	@echo "Available commands from root:"
	@echo "  make                     - Build compiler and VM"
	@echo "  make compiler            - Build compiler module only"
	@echo "  make vm                  - Build VM module only"
	@echo "  make tests               - Run all tests"
	@echo "  make test_compiler_tests - Run all compiler-related tests"
	@echo "  make test_vm_tests       - Run all VM-related tests"
	@echo "  make test_lexer          - Run Lexer test only"
	@echo "  make test_vm             - Run VM test only"
	@echo "  make clean               - Remove object files and binaries"
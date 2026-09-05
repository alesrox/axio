.PHONY: all compiler vm tests test_lexer test_vm clean help

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

test_lexer:
	$(MAKE) -C tests test_lexer

test_vm:
	$(MAKE) -C tests test_vm

clean:
	@echo "==> Cleaning project..."
	$(MAKE) -C compiler clean
	$(MAKE) -C vm clean
	$(MAKE) -C tests clean

help:
	@echo "Available commands from root:"
	@echo "  make            - Build compiler and VM"
	@echo "  make compiler   - Build compiler module only"
	@echo "  make vm         - Build VM module only"
	@echo "  make tests      - Run all tests"
	@echo "  make test_lexer - Run Lexer tests only"
	@echo "  make clean      - Remove object files and binaries"
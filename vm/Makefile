CC       := gcc
CFLAGS   := -Wall -Wextra -Werror -std=c11 -g -Iinclude -I../common/include
SRC_DIR  := src
OBJ_DIR  := obj
BIN_DIR  := ../bin

SRCS     := $(wildcard $(SRC_DIR)/*.c)
OBJS     := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.p1.o, $(SRCS))
TARGET   := $(BIN_DIR)/vm

.PHONY: all clean

all:
ifneq ($(strip $(SRCS)),)
	@$(MAKE) $(TARGET)
else
	@echo "==> Skipping VM build (no source files in $(SRC_DIR)/)"
endif

$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(OBJ_DIR)/%.p1.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(OBJ_DIR) $(TARGET)
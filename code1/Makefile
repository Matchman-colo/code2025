TARGET   = hello

CC       = gcc
CFLAGS   = -std=c99 -Wall -I.

LINKER   = gcc
LFLAGS   = -Wall -I. -lm

SRC_DIR   = src
OBJ_DIR   = bin/obj
BIN_DIR   = bin

SOURCES  := $(wildcard $(SRC_DIR)/*.c)
OBJECTS  := $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
RM_R      = rm -r
MKDIR_P   = mkdir -p

$(BIN_DIR)/$(TARGET): $(OBJECTS)
	@$(LINKER) $(OBJECTS) $(LFLAGS) -o $@

$(OBJECTS): $(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	@$(MKDIR_P) ${BIN_DIR} $(OBJ_DIR)
	@$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	@$(RM_R) $(OBJ_DIR) $(BIN_DIR)



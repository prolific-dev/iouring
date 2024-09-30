# Makefile for building static library from util directory

# Directories
SRC_DIR := .
INC_DIR := ./include
LIB_DIR := ./lib

# Source files
SRCS := $(SRC_DIR)/cfg.c \
		$(SRC_DIR)/log.c \
		$(SRC_DIR)/sock.c \
		$(SRC_DIR)/list.c \
		$(SRC_DIR)/io_time.c

OBJS := $(SRCS:$(SRC_DIR)/%.c=$(SRC_DIR)/%.o)

# Compiler
CC := gcc

# Compiler flags
CFLAGS  := -Wall -Werror -Wno-unused-function -I$(INC_DIR)

# Archive command for creating static library
AR      := ar
ARFLAGS := rcs

# Target
TARGET := $(LIB_DIR)/libutil.a

# Build rule
$(TARGET): $(OBJS)
	@mkdir -p $(LIB_DIR)
	$(AR) $(ARFLAGS) $@ $^
	rm -rf $(OBJS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -rf $(TARGET)

.PHONY: clean

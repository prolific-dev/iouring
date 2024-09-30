# Makefile

# Directories
SRC_DIR := .
OBJ_DIR := .
INC_DIR := ./include
TAR_DIR := ../..

UTIL_INC_DIR     := ../util/include
LIBURING_INC_DIR := /usr/include/liburing

UTIL_LIB_DIR     := ../util/lib

# Source files
SRCS := $(SRC_DIR)/nat.c \
		$(SRC_DIR)/if_net.c \
		$(SRC_DIR)/if_ip.c \
		$(SRC_DIR)/if_io.c \
		$(SRC_DIR)/if_poll.c \
		$(SRC_DIR)/if_iouring.c \
		$(SRC_DIR)/if_liburing.c \
		$(SRC_DIR)/if_stats.c

OBJS := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Compiler
CC := gcc

# Compiler flags
IFLAGS        := -I$(UTIL_INC_DIR) -I$(INC_DIR) -I/usr/include/liburing
LDFLAGS       := -luring -lpthread -L$(UTIL_LIB_DIR) -lutil
CFLAGS        := -Wall -Werror -Wno-unused-variable -Wno-unused-but-set-variable -Wno-uninitialized $(IFLAGS)

# Target
TARGET := $(TAR_DIR)/nat

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(URING_LDFLAGS)
	rm -rf $(OBJS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@ $(IFLAGS)

clean:
	rm -rf $(TARGET)

.PHONY: all clean

CC=gcc

CFLAGS=-Wall -Werror -Wno-unused-variable -Wno-unused-but-set-variable -Wno-uninitialized
IFLAGS=-I/usr/include/liburing
LDFLAGS=-luring -lpthread

TARGET=bufcopy

all: $(TARGET)

$(TARGET): bufcopy.c
	$(CC) $(CFLAGS) $^ -o $@ $(IFLAGS) $(LDFLAGS)

clean:
	rm -rf $(TARGET)
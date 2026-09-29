CC = clang
TARGET = btrstream
CFLAGS = -Llib -std=c99 -Werror -Iinclude -lshout -Wall -Wextra -O3
SRC = *.c

.PHONY: all

all:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

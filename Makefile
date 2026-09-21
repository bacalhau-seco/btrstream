CC = clang
TARGET = build/niggastream

CFLAGS = -Llib -std=c99 -Werror -Iinclude -lshout -Wall -Wextra -O3

SRC = src/*.c

.PHONY: all run clean force raylib

all:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

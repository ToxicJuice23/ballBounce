# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = $(shell sdl2-config --cflags --libs) -lm

# Source and target
SRC = main.c
HEADERS = main.h
TARGET = ball

# Default target
all: $(TARGET)

# Build target
$(TARGET): $(SRC) $(HEADERS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

# Clean target
clean:
	rm -f $(TARGET)
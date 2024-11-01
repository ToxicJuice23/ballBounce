# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O3
LDFLAGS = $(shell sdl2-config --cflags --libs) -lm

# Source and target
HEADERS = main.h
TARGET = ball_sdl
DEBUG_TARGET = ball_debug
SRC = ball_sdl.c string_functions.c main.c

# Default target
all: $(TARGET)
debug: $(DEBUG_TARGET)

# Build target
$(TARGET): $(SRC) $(HEADERS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

$(DEBUG_TARGET): $(SRC) $(HEADERS)
	$(CC) $(CFLAGS) -g -o $(DEBUG_TARGET) $(SRC) $(LDFLAGS)

# Clean target
clean:
	rm -f $(TARGET)
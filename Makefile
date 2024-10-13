# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O3
LDFLAGS = $(shell sdl2-config --cflags --libs) -lm

# Source and target
FAST_SRC = fast_ball.c
FAST_HEADERS = fast_ball.h
FAST_TARGET = fast_ball
HEADERS = ball.h
TARGET = ball
SRC = ball.c

# Default target
all: $(TARGET) $(FAST_TARGET)

fast: $(FAST_TARGET)

# Build target
$(TARGET): $(SRC) $(HEADERS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

$(FAST_TARGET): $(FAST_SRC) $(FAST_HEADERS)
	$(CC) $(CFLAGS) -o $(FAST_TARGET) $(FAST_SRC) $(LDFLAGS)

# Clean target
clean:
	rm -f $(TARGET) $(FAST_TARGET)
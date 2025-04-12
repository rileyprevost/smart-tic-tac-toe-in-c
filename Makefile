# Name of the executable
TARGET = tic_tac_toe

# Source files
SRCS = tic_tac_toe.c set.c

# Object files
OBJS = $(SRCS:.c=.o)

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -g -O0 -Wall -fsanitize=address,undefined

# Default rule
all: $(TARGET)

# Build the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)
	dsymutil $(TARGET) || true  # only runs on macOS

# Compile .c to .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(TARGET) $(OBJS)
	rm -rf $(TARGET).dSYM

.PHONY: all clean
# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -std=c99 -O2

# Source files
SRCS = main.c file_system.c menu.c

# Object files (placed in build directory)
OBJS = $(SRCS:%.c=build/%.o)

# Executable name (placed in build directory)
TARGET = build/file_system

# Default target
all: $(TARGET)

# Link object files to create the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

# Compile source files into object files (placed in build directory)
build/%.o: %.c | build
	$(CC) $(CFLAGS) -c $< -o $@

# Create the build directory if it doesn't exist
build:
	mkdir -p build

# Run the executable
run: $(TARGET)
	./$(TARGET)

# Clean up build files
clean:
	rm -rf build

# Phony targets (not actual files)
.PHONY: all clean run

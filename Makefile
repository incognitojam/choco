CC = gcc

CFLAGS = -Wall -g

BUILD_DIR = build

TARGET = $(BUILD_DIR)/main

SRCS = main.c

HEADERS = tokenizer.h

OBJS = $(SRCS:%.c=$(BUILD_DIR)/%.o)

.PHONY: all clean run snapshot test test-all test-ci test-verbose regen

all: $(TARGET)

$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: %.c $(HEADERS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

run: $(TARGET)
	./$(TARGET)

# Test all fixtures using the Python test script
test: $(TARGET)
	./test.py test

# Test all fixtures (alias for test)
test-all: $(TARGET)
	./test.py test

# Test in CI mode (only fail on new failures)
test-ci: $(TARGET)
	./test.py test --ci

# Test with verbose output
test-verbose: $(TARGET)
	./test.py test --verbose

# Test in CI mode with verbose output
test-ci-verbose: $(TARGET)
	./test.py test --ci --verbose

# Regenerate all snapshots
regen: $(TARGET)
	./test.py regenerate

# Test specific fixture (e.g., make test-vars)
test-%: $(TARGET)
	./test.py test-$*

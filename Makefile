CC = gcc

CFLAGS = -Wall -g

BUILD_DIR = build

TARGET = $(BUILD_DIR)/main

SRCS = main.c

HEADERS = tokenizer.h

OBJS = $(SRCS:%.c=$(BUILD_DIR)/%.o)

.PHONY: all clean run snapshot test

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

snapshot: $(TARGET)
	./$(TARGET) > snapshot.txt

test: $(TARGET)
	./$(TARGET) > output.txt
	@if diff -q snapshot.txt output.txt > /dev/null; then \
		echo "Test passed: output matches expected"; \
		rm output.txt; \
	else \
		echo "Test failed: output differs from expected"; \
		echo "Expected:"; \
		cat snapshot.txt; \
		echo "Actual:"; \
		cat output.txt; \
		rm output.txt; \
		exit 1; \
	fi

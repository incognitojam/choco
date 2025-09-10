C = gcc

CFLAGS = -Wall -g

BUILD_DIR = build

TARGET = $(BUILD_DIR)/main

SRCS = main.c

OBJS = $(SRCS:%.cpp=$(BUILD_DIR)/%.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(C) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(C) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

run: $(TARGET)
	$(TARGET)

clean:
	rm -rf $(BUILD_DIR)

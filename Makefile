CXX = g++

CXXFLAGS = -Wall -g

BUILD_DIR = build

TARGET = $(BUILD_DIR)/main

SRCS = main.cpp

OBJS = $(SRCS:%.cpp=$(BUILD_DIR)/%.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

run: $(TARGET)
	$(TARGET)

clean:
	rm -rf $(BUILD_DIR)

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude

# Project name
TARGET = device

# Source files (using wildcard to gather all .cpp files in src/)
SRCS = main.cpp $(wildcard src/*.cpp)

# Output directory for object files
OUT_DIR = out
OBJS = $(SRCS:src/%.cpp=$(OUT_DIR)/%.o)

# Default rule
all: $(OUT_DIR) $(TARGET)

# Create output directory if it doesn't exist
$(OUT_DIR):
	@mkdir -p $(OUT_DIR)

# Linking
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compilation
$(OUT_DIR)/%.o: src/%.cpp | $(OUT_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up
clean:
	rm -rf $(OUT_DIR) $(TARGET)

# Rebuild everything
rebuild: clean all

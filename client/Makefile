# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Iinclude -Ilib/imgui -Ilib/imgui/backends -MMD `pkg-config --cflags glfw3`
LDFLAGS = `pkg-config --static --libs glfw3` -lGL -ldl -lpthread

# Source and output directories
SRC_DIR = src core lib/imgui lib/imgui/backends
BUILD_DIR = out
SOURCES = $(wildcard $(addsuffix /*.cpp, $(SRC_DIR)))

# Generate object file paths in the out/ directory
OBJECTS = $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SOURCES))
DEPS = $(OBJECTS:.o=.d) # File phụ thuộc (.d)

# Executable output
EXECUTABLE = $(BUILD_DIR)/main

# Include dependency files
-include $(DEPS)

# Create build directory if it doesn't exist
$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link all objects
$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

# Default target
all: $(EXECUTABLE)

# Clean build files
clean:
	rm -rf $(BUILD_DIR)

rebuild: clean all
# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall

# Project name
TARGET = device

# Source files
SRCS = main.cpp Light.cpp AC.cpp Logger.cpp Fan.cpp NetworkHandler.cpp TimerManager.cpp CommandHandler.cpp Device.cpp RuntimeTracker.cpp AuthenticationManager.cpp Utility.cpp

# Output directory for object files
OUT_DIR = out
OBJS = $(addprefix $(OUT_DIR)/, $(SRCS:.cpp=.o))

# Default rule
all: $(OUT_DIR) $(TARGET)

# Create output directory if it doesn't exist
$(OUT_DIR):
	mkdir -p $(OUT_DIR)

# Linking
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compilation
$(OUT_DIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up
clean:
	rm -rf $(OUT_DIR) $(TARGET)

# Rebuild everything
rebuild: clean all

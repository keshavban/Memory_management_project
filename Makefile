# Compiler settings
CXX = g++
CXXFLAGS = -std=c++11 -Wall

# Directories
SRC_DIR = src
INC_DIR = include

# Target executable name
TARGET = memsim

# Source files
SOURCES = $(SRC_DIR)/main.cpp $(SRC_DIR)/MemoryManager.cpp $(SRC_DIR)/Cache.cpp

# Build rule
all:
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) $(SOURCES) -o $(TARGET)

# Clean rule
clean:
	rm -f $(TARGET)
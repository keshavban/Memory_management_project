CXX = g++
CXXFLAGS = -std=c++11 -Wall

SRC_DIR = src
INC_DIR = include
TARGET = memsim

# ADD BuddyAllocator.cpp here
SOURCES = $(SRC_DIR)/main.cpp \
          $(SRC_DIR)/MemoryManager.cpp \
          $(SRC_DIR)/BuddyAllocator.cpp \
          $(SRC_DIR)/Cache.cpp \
          $(SRC_DIR)/VirtualMemory.cpp

all: $(TARGET)
$(TARGET):
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) $(SOURCES) -o $(TARGET)
run: $(TARGET)
	./$(TARGET)
clean:
	rm -f $(TARGET)


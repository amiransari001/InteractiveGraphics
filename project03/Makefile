# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -Wextra -O2 -std=c++11

# Paths to GLEW and project headers (moved one level up)
GLEW_PATH = ../glew-2.2.0
INCLUDES = -I$(GLEW_PATH)/include -Iinclude

# Library paths and linking
LDFLAGS = -L$(GLEW_PATH)/lib -lGLEW -lglut -lGL -Wl,-rpath,$(GLEW_PATH)/lib

# Source files
SOURCES = $(wildcard src/*.cpp)

# Object files (replace .cpp with .o)
OBJECTS = $(SOURCES:.cpp=.o)

# Output executable name
TARGET = prog

# Default build target
all: $(TARGET)

# Linking step
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Compiling source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Run the program with correct GLEW library path
run: $(TARGET)
	LD_LIBRARY_PATH=$(GLEW_PATH)/lib:$$LD_LIBRARY_PATH ./$(TARGET)

# Clean generated files
clean:
	rm -f $(OBJECTS) $(TARGET)

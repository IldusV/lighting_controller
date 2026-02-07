# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -pthread -I/usr/local/include
LDFLAGS = -L/usr/local/lib -lpaho-mqttpp3 -lpaho-mqtt3as -lpigpio

# Source files
SRC = KeypadHandler.cpp main.cpp MqttMgr.cpp

# Header files
HEADERS = KeypadHandler.h MqttMgr.h

# Output binary
OUTPUT = app

# Object files
OBJ = $(SRC:.cpp=.o)

# Build the application
all: $(OUTPUT)

$(OUTPUT): $(OBJ)
	$(CXX) $(OBJ) -o $(OUTPUT) $(CXXFLAGS) $(LDFLAGS)


# Compile C++ source files
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean the build
clean:
	rm -f $(OBJ) $(OUTPUT)

.PHONY: all clean

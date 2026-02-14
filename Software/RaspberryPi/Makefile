# Compiler and flags
CXX = g++
CC = gcc
CXXFLAGS = -std=c++17 -Wall -pthread -I/usr/local/include -Ilib/u8g2/csrc/. -I.
CFLAGS = -Wall -Ilib/u8g2/csrc/. -I.
LDFLAGS = -L/usr/local/lib -lpaho-mqttpp3 -lpaho-mqtt3as -lpigpio

# Separate source files by type
CPP_SRC = src/KeypadHandler.cpp src/main.cpp src/MqttMgr.cpp src/linux-i2c.cpp
C_SRC = $(wildcard lib/u8g2/csrc/*.c)

# Generate object files for both types
OBJ = $(CPP_SRC:.cpp=.o) $(C_SRC:.c=.o)

# Header files
HEADERS = KeypadHandler.h MqttMgr.h linux-i2c.h

# Output binary
OUTPUT = app

all: $(OUTPUT)

$(OUTPUT): $(OBJ)
	$(CXX) $(OBJ) -o $(OUTPUT) $(LDFLAGS)

# Rule for C++ files
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule for C files (u8g2 library)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(OUTPUT)

.PHONY: all clean
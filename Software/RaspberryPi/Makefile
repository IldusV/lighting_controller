# Compiler and flags
CXX = g++
CC = gcc
CXXFLAGS = -std=c++17 -Wall -pthread -I/usr/local/include -Ilib/u8g2/csrc/. -I.
CFLAGS = -Wall -Ilib/u8g2/csrc/. -I.
LDFLAGS = -L/usr/local/lib -lpaho-mqttpp3 -lpaho-mqtt3as -lpigpio

# Separate source files by type
CPP_SRC =   src/main.cpp \
            src/MqttMgr.cpp \
            src/I2CManager.cpp \
            src/I2CSerialAdapter.cpp \
            src/KeypadDriver.cpp \
            src/CommandManager.cpp \
            src/TopicLoader.cpp \
            src/ActuatorState.cpp \
            src/LoggerHandler.cpp \
            src/U8G2Driver.cpp \
            src/DisplayRenderer.cpp \
            src/DisplayHandler.cpp \
            src/WiFiWatcher.cpp

C_SRC = $(wildcard lib/u8g2/csrc/*.c)

# Generate object files
OBJ = $(CPP_SRC:.cpp=.o) $(C_SRC:.c=.o)

# Header files
HEADERS =   src/MqttMgr.h \
            src/SerialInterface.h \
            src/I2CManager.h \
            src/I2CSerialAdapter.h \
            src/KeypadDriver.h \
            src/Events.h \
            src/IEventHandler.h \
            src/CommandManager.h \
            src/TopicLoader.h \
            src/ActuatorState.h \
            src/LoggerHandler.h \
            src/U8G2Driver.h \
            src/DisplayRenderer.h \
            src/DisplayHandler.h \
            src/SystemState.h \
            src/WiFiWatcher.h \
            src/SystemEventQueue.h

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
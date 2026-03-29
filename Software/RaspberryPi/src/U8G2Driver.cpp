#include "U8G2Driver.h"
#include <u8g2.h>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>

#define BUFSIZ_I2C 16384  // Increase this! ST75256 needs more space

// This callback is what u8g2 will use to talk to your I2CSerialAdapter
uint8_t u8x8_byte_cpp_adapter(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    // 1. Retrieve the adapter from the u8g2 user pointer
    auto* adapter = static_cast<I2CSerialAdapter*>(u8x8_GetUserPtr(u8x8));
    
    static uint8_t buffer[BUFSIZ_I2C]; // Buffer for I2C transactions
    static uint8_t buf_idx;

    switch (msg) {
        case U8X8_MSG_BYTE_INIT:
            return 1;
        case U8X8_MSG_BYTE_START_TRANSFER:
            buf_idx = 0;
            return 1;
        case U8X8_MSG_BYTE_SEND: {
            uint8_t *data = (uint8_t *)arg_ptr;
            while (arg_int > 0 && buf_idx < BUFSIZ_I2C) {
                buffer[buf_idx++] = *data++;
                arg_int--;
            }
            return 1;
        }
        case U8X8_MSG_BYTE_END_TRANSFER:
            // 2. Use your existing adapter's write method
            // Assuming your adapter has a write(data, len) or similar method
            try {
                adapter->send(buffer, buf_idx);
                return 1;
            } catch (...) {
                return 0; // I2C Failure
            }
        case U8X8_MSG_BYTE_SET_DC:
            return 1; // Not used for I2C usually
        default:
            fprintf(stderr, "Unknown message type: %d\n", msg);
            return 0; // Unknown message
    }
    return 0;
}


uint8_t u8x8_linux_i2c_delay(u8x8_t *u8x8,
		     uint8_t msg,
		     uint8_t arg_int,
		     void *arg_ptr)
{
	struct timespec req;
	struct timespec rem;
	int ret;

	req.tv_sec = 0;
	
	switch(msg) {
	case U8X8_MSG_DELAY_NANO:  // delay arg_int * 1 nano second
		req.tv_nsec = arg_int;
		break;
	case U8X8_MSG_DELAY_100NANO:       // delay arg_int * 100 nano seconds
		req.tv_nsec = arg_int * 100;
		break;
	case U8X8_MSG_DELAY_10MICRO: // delay arg_int * 10 micro seconds
		req.tv_nsec = arg_int * 10000;
		break;
	case U8X8_MSG_DELAY_MILLI:  // delay arg_int * 1 milli second
		req.tv_nsec = arg_int * 1000000;
		break;
	default:
		return 0;
	}

	while((ret = nanosleep(&req, &rem)) && errno == EINTR){
		struct timespec tmp = req;
		req = rem;
		rem = tmp;
	}
	if (ret) {
		perror("nanosleep");
		fprintf(stderr, "can't sleep\n");
		return(errno);
	}
	
	return 1;
}

U8G2Driver::U8G2Driver(std::shared_ptr<I2CSerialAdapter> adapter) : adapter_(adapter) {
    // Initialize the C struct
    // We use your custom callback 'u8x8_byte_cpp_adapter'
    u8g2_Setup_st75256_i2c_jlx25664_f(&u8g2_, U8G2_R0, u8x8_byte_cpp_adapter, u8x8_linux_i2c_delay);

    // This is the "Magic" step: Save the C++ object pointer inside the C struct
    u8g2_SetUserPtr(&u8g2_, adapter_.get());
}

U8G2Driver::~U8G2Driver() {}

void U8G2Driver::init() {
    // u8g2_Setup_st75256_i2c_jlx25664_f(&u8g2_, U8G2_R0, u8x8_byte_linux_i2c, u8x8_linux_i2c_delay);

    u8g2_SetI2CAddress(&u8g2_, 0x3c);
    u8g2_InitDisplay(&u8g2_);
    u8g2_SetPowerSave(&u8g2_, 0);
	u8g2_SetContrast(&u8g2_, 75);

	u8g2_ClearBuffer(&u8g2_);
}

void U8G2Driver::clear() {
    u8g2_ClearBuffer(&u8g2_);
    u8g2_ClearDisplay(&u8g2_);
}

void U8G2Driver::update() {
    u8g2_SendBuffer(&u8g2_);
}

void U8G2Driver::setTheme(const ThemeConfig& config) {
    currentTheme_ = config;
}

std::vector<unsigned char> loadXBM(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "[ERROR] Failed to open: " << filename << std::endl;
        throw std::runtime_error("Cannot open XBM file: " + filename);
    }

    std::vector<unsigned char> data;
    std::string line;
    bool dataStarted = false;

    while (std::getline(file, line)) {
        // Find the start of the actual hex data after the '{'
        if (!dataStarted) {
            if (line.find('{') != std::string::npos) {
                dataStarted = true;
                continue;
            }
            continue;
        }

        // Stop if we hit the end of the array '};'
        if (line.find('}') != std::string::npos) break;

        // Clean up formatting characters (commas, semicolons)
        std::replace(line.begin(), line.end(), ',', ' ');
        std::replace(line.begin(), line.end(), ';', ' ');

        std::stringstream ss(line);
        std::string hexStr;
        while (ss >> hexStr) {
            try {
                // Convert "0xFF" text to the actual byte 255
                unsigned char byte = static_cast<unsigned char>(std::stoul(hexStr, nullptr, 16));
                data.push_back(byte);
            } catch (...) {
                continue; // Skip things that aren't hex (like 'char' or 'static')
            }
        }
    }
    return data;
}

void U8G2Driver::drawActuatorIcon(int slot, const std::string& iconName, bool active) {
    int x, y;
    getCoordinatesForSlot(slot, x, y);

    std::string state = active ? "_on.xbm" : "_off.xbm";
    std::string fullPath = currentTheme_.baseDir + "/" + iconName + state;

    auto bitmap = loadXBM(fullPath);
    u8g2_DrawXBM(&u8g2_, x, y, 48, 43, bitmap.data());
}

void U8G2Driver::getCoordinatesForSlot(int slot, int& x, int& y) {
    // Layout logic: linear left to right with 18 pixel offset
    x = slot * 50+3;
    y = 9;
}

void U8G2Driver::drawWifiStatus(bool connected, int signalLevel) {
    if (connected) {
        u8g2_SetFont(&u8g2_, u8g2_font_5x7_tr);
        u8g2_DrawStr(&u8g2_, 0, 62, "WiFi: ON");
        u8g2_DrawStr(&u8g2_, 50, 62, ("Signal: " + std::to_string(signalLevel)).c_str());
    }
    else {
        u8g2_SetFont(&u8g2_, u8g2_font_5x7_tr);
        u8g2_DrawStr(&u8g2_, 0, 62, "WiFi: OFF");
    }
}

void U8G2Driver::drawMqttStatus(bool connected) {
    if (connected) u8g2_DrawDisc(&u8g2_, 100, 4, 2, U8G2_DRAW_ALL);
}

void U8G2Driver::drawFooter(const std::string& text) {
    u8g2_SetFont(&u8g2_, u8g2_font_5x7_tr);
    u8g2_DrawStr(&u8g2_, 0, 62, text.c_str());
}
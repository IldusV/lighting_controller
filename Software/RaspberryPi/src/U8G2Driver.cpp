#include "U8G2Driver.h"
#include "linux-i2c.h"
#include <u8g2.h>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <vector>
#include <string>

U8G2Driver::U8G2Driver() {
    // Initial setup of u8g2 structure based on your specific OLED (e.g., SSD1306)
    // u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2_, ...);
}

U8G2Driver::~U8G2Driver() {}

void U8G2Driver::init() {
    u8g2_Setup_st75256_i2c_jlx25664_f(&u8g2_, U8G2_R0, u8x8_byte_linux_i2c, u8x8_linux_i2c_delay);

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

#include <fstream>
#include <sstream>
#include <algorithm>

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
        // Draw bars icon based on signalLevel
        u8g2_DrawBox(&u8g2_, 110, 0, 4, 8); 
    } else {
        u8g2_DrawLine(&u8g2_, 110, 0, 114, 8); // 'X' for disconnected
    }
}

void U8G2Driver::drawMqttStatus(bool connected) {
    if (connected) u8g2_DrawDisc(&u8g2_, 100, 4, 2, U8G2_DRAW_ALL);
}

void U8G2Driver::drawFooter(const std::string& text) {
    u8g2_SetFont(&u8g2_, u8g2_font_5x7_tr);
    u8g2_DrawStr(&u8g2_, 0, 62, text.c_str());
}
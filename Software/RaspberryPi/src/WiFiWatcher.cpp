#include "WiFiWatcher.h"
#include <sys/resource.h>

#include <fstream>
#include <sstream>
#include <iostream>
#include <chrono>
#include <iostream>

WiFiWatcher::WiFiWatcher(WiFiStatusCallback callback) : callback_(callback) {}

WiFiWatcher::~WiFiWatcher() {
    stop();
}

void WiFiWatcher::start() {
    if (!running_) {
        running_ = true;
        workerThread_ = std::thread(&WiFiWatcher::watchLoop, this);
    }
}

void WiFiWatcher::stop() {
    running_ = false;
    if (workerThread_.joinable()) {
        workerThread_.join();
    }
}

void WiFiWatcher::watchLoop() {
    setpriority(PRIO_PROCESS, 0, 10); // Boost thread priority for responsiveness
    while (running_) {
        parseProcNet("wlan0");
        std::this_thread::sleep_for(std::chrono::seconds(pollIntervalS));
    }
}

void WiFiWatcher::parseProcNet(const std::string& targetIface) {
    std::ifstream file("/proc/net/wireless");
    if (!file.is_open()) {
        return; // Don't hang if the kernel node is busy
    }

    std::string line;
    bool found = false;

    // Fast-forward past headers
    for(int i = 0; i < 2; ++i) {
        if (!std::getline(file, line))
            return;
    }

    while (std::getline(file, line)) {
        if (line.find(targetIface) != std::string::npos) {
            try {
                // Split by spaces manually or use a lighter scan
                // The format is: iface status link level noise...
                std::vector<std::string> tokens;
                std::string token;
                std::istringstream iss(line);
                while (iss >> token) tokens.push_back(token);

                if (tokens.size() >= 4) {
                    // tokens[2] is link quality, tokens[3] is level (RSSI)
                    int8_t rssi = static_cast<int8_t>(std::stof(tokens[3]));
                    std::cout << "WiFiWatcher: Detected " << targetIface << " with RSSI " << static_cast<int>(rssi) << std::endl;
                    // SystemState::getInstance().setWiFiStatus(ConnectionStatus::CONNECTED, rssi);
                    callback_(true, rssi);
                    found = true;
                }
            } catch (...) {
                std::cerr << "WiFiWatcher: Error parsing line: " << line << std::endl;
                //Prevent crash on malformed lines
            }
            break;
        }
    }

    if (!found) {
        std::cout << "WiFiWatcher: " << targetIface << " not found, marking as DISCONNECTED" << std::endl;
        callback_(false, 0);
        // SystemState::getInstance().setWiFiStatus(ConnectionStatus::DISCONNECTED, 0);
    }
    
    file.close(); // Explicitly close to free the handle immediately
}
#include "TopicLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

// Helper to trim whitespace from strings
std::string trim(const std::string& s) {
    size_t first = s.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = s.find_last_not_of(" \t\n\r");
    return s.substr(first, (last - first + 1));
}

ConfigData TopicLoader::load(const std::string& filename) {
    ConfigData config;
    std::ifstream file(filename);
    std::string line;

    if (!file.is_open()) {
        std::cerr << "Failed to open config file: " << filename << std::endl;
        return config;
    }

    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        std::stringstream ss(line);
        std::string segment;
        std::vector<std::string> parts;

        // Split by ':'
        while (std::getline(ss, segment, ':')) {
            parts.push_back(trim(segment));
        }

        if (parts.size() < 4) continue;

        std::string type = parts[0]; // CMD or STAT
        int id = std::stoi(parts[1]);
        std::string actionOrName = parts[2];
        std::string topic = parts[3];

        if (type == "CMD") {
            config.buttonMap[static_cast<uint8_t>(id)].push_back({topic, actionOrName});
        } 
        else if (type == "STAT") {
            config.statusMap[topic] = id;
        }
    }

    return config;
}
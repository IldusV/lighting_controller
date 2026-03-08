#include "LoggerHandler.h"
#include <iostream>
#include <iomanip>
#include <ctime>

LoggerHandler::LoggerHandler(const std::vector<ActuatorState>& states, const ConfigData& config)
    : actuators_(states), config_(config) {}

void LoggerHandler::handleEvent(const Event& e) {
    // We only care about MQTT status updates for logging
    if (e.type() == Event::Type::MqttUpdate) {
        const auto& me = static_cast<const MqttEvent&>(e);
        const std::string& topic = me.topic();

        // Check if this topic is mapped to an actuator index
        auto it = config_.statusMap.find(topic);
        if (it != config_.statusMap.end()) {
            int idx = it->second;

            // Ensure index is within bounds
            if (idx >= 0 && static_cast<size_t>(idx) < actuators_.size()) {
                printTimestamp();
                std::cout << " [LOG] Actuator #" << idx 
                          << " (" << topic << ") updated to: " 
                          << actuators_[idx].getStateString() << std::endl;
            }
        }
    }
}

void LoggerHandler::printTimestamp() {
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    std::cout << "[" << std::put_time(&tm, "%H:%M:%S") << "]";
}
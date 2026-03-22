#include "CommandManager.h"

// 1. Clear out the constructor
CommandManager::CommandManager(MqttMgr& mqtt, std::vector<ActuatorState>& states, ConfigData config)
    : mqtt_(mqtt), actuators_(states), config_(config) {
    // Do NOT subscribe here anymore
}

// 2. Add this new method
void CommandManager::subscribeAll() {
    for (auto const& [topic, idx] : config_.statusMap) {
        mqtt_.subscribe(topic);
        std::cout << "Subscribing to: " << topic << std::endl;
    }
}

void CommandManager::handleEvent(const Event& e) {
    switch (e.type()) {
        case Event::Type::ButtonPressed:
            handleButton(static_cast<const ButtonEvent&>(e).getCode());
            break;
        case Event::Type::MqttUpdate:
            handleMqtt(static_cast<const MqttEvent&>(e).topic(), 
                       static_cast<const MqttEvent&>(e).payload());
            break;
        default:
            break;
    }
}

void CommandManager::handleButton(uint8_t code) {
    if (config_.buttonMap.count(code)) {
        for (const auto& action : config_.buttonMap[code]) {
            mqtt_.publish(action.topic, action.action);
        }
    }
}

void CommandManager::handleMqtt(const std::string& topic, const std::string& payload) {
    if (config_.statusMap.count(topic)) {
        int idx = config_.statusMap[topic];
        PowerState s = (payload == "ON") ? PowerState::ON : PowerState::OFF;
        if (idx < actuators_.size()) actuators_[idx].update(s, 0, 0);
    }
}
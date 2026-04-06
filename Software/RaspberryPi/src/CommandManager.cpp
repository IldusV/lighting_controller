#include "CommandManager.h"

CommandManager::CommandManager(MqttMgr& mqtt, std::vector<ActuatorState>& states, ConfigData config)
    : mqtt_(mqtt), actuators_(states), config_(config) {
}

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

// ildusv: 0-3 bits of the code is the button column, 4-7 bits is the button row
void CommandManager::handleButton(uint8_t code) {
   
    if (config_.buttonMap.count(code)) {
        for (const auto& action : config_.buttonMap[code]) {
            mqtt_.publish(action.topic, action.action);
        }
    }

    // Handle selector buttons (0x20 - 0x24)
    if (code >= 0x20 && code < 0x25) {
        for (auto& actuator : actuators_) {
            actuator.setSelected(false);
        }
        actuators_[code & 0x0F].setSelected(true);
        std::cout << "Selected actuator " << (code & 0x0F) << std::endl;
    }
}

void CommandManager::handleMqtt(const std::string& topic, const std::string& payload) {
    if (config_.statusMap.count(topic)) {
        int idx = config_.statusMap[topic];
        PowerState s = (payload == "ON") ? PowerState::ON : PowerState::OFF;
        
        if (idx < actuators_.size()) {
            actuators_[idx].update(s, 0, 0);
        }
    }
}
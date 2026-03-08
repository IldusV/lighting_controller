// #ifndef IEVENT_HANDLER_H
// #define IEVENT_HANDLER_H

// #include <string>
// #include <cstdint>

// class IEventHandler {
// public:
//     virtual ~IEventHandler() = default;
    
//     // Triggered by local inputs (Keypad, etc.)
//     virtual void onButtonPress(uint8_t code) = 0;
    
//     // Triggered by network updates (MQTT, etc.)
//     virtual void onMqttMessage(const std::string& topic, const std::string& payload) = 0;
// };

// #endif

#ifndef I_EVENT_HANDLER_H
#define I_EVENT_HANDLER_H

#include "Events.h"

class IEventHandler {
public:
    virtual ~IEventHandler() = default;
    virtual void handleEvent(const Event& e) = 0;
};

#endif
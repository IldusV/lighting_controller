#ifndef TOPIC_LOADER_H
#define TOPIC_LOADER_H

#include <string>
#include <vector>
#include <map>

struct ButtonAction {
    std::string topic;
    std::string action;
};

struct ConfigData {
    // Map Button ID -> List of Actions (one button can trigger multiple topics)
    std::map<uint8_t, std::vector<ButtonAction>> buttonMap;
    
    // Map MQTT Status Topic -> Actuator Index
    std::map<std::string, int> statusMap;
};

class TopicLoader {
public:
    static ConfigData load(const std::string& filename);
};

#endif
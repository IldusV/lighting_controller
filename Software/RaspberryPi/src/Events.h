#ifndef EVENTS_H
#define EVENTS_H

#include <string>
#include <cstdint>

class Event {
public:
    enum class Type { ButtonPressed, MqttUpdate };
    virtual ~Event() = default;
    virtual Type type() const = 0;
};

class ButtonEvent : public Event {
public:
    ButtonEvent(uint8_t code) : code_(code) {}
    Type type() const override { return Type::ButtonPressed; }
    uint8_t getCode() const { return code_; }
private:
    uint8_t code_;
};

class MqttEvent : public Event {
public:
    MqttEvent(std::string t, std::string p) : topic_(t), payload_(p) {}
    Type type() const override { return Type::MqttUpdate; }
    const std::string& topic() const { return topic_; }
    const std::string& payload() const { return payload_; }
private:
    std::string topic_, payload_;
};

#endif
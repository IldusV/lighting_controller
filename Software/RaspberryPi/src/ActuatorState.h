#ifndef ACTUATOR_STATE_H
#define ACTUATOR_STATE_H

#include <cstdint>
#include <mutex>
#include <string>

enum class PowerState { OFF = 0, ON = 1, NA = 2 };

class ActuatorState {
public:
    ActuatorState() = default;

    struct Data {
        PowerState state;
        uint8_t val1;
        uint8_t val2;
    };

    void update(PowerState s, uint8_t v1, uint8_t v2);

    Data getData() const;

    std::string getStateString() const;

    bool isSelected() const {
        return selected_;
    }

    void setSelected(bool sel) {
        selected_ = sel;
    }

private:
    mutable std::mutex mtx_;
    PowerState state_ = PowerState::NA;
    uint8_t value1_ = 0;
    uint8_t value2_ = 0;

    bool selected_ = false;
};

#endif
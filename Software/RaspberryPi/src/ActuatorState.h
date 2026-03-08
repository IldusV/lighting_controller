#ifndef ACTUATOR_STATE_H
#define ACTUATOR_STATE_H

#include <cstdint>
#include <mutex>
#include <string>

enum class PowerState { OFF = 0, ON = 1, NA = 2 };

class ActuatorState {
public:
    ActuatorState() = default;

    // Structure for returning a consistent snapshot of the data
    struct Data {
        PowerState state;
        uint8_t val1;
        uint8_t val2;
    };

    // Update the state (called by CommandManager)
    void update(PowerState s, uint8_t v1, uint8_t v2);

    // Get a copy of the current data (called by Display/UI)
    Data getData() const;

    // Helper to get a string representation of the power state
    std::string getStateString() const;

private:
    mutable std::mutex mtx_;
    PowerState state_ = PowerState::NA;
    uint8_t value1_ = 0;
    uint8_t value2_ = 0;
};

#endif
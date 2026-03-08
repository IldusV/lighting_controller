#include "ActuatorState.h"

void ActuatorState::update(PowerState s, uint8_t v1, uint8_t v2) {
    // lock_guard automatically unlocks when this function returns
    std::lock_guard<std::mutex> lock(mtx_);
    state_ = s;
    value1_ = v1;
    value2_ = v2;
}

ActuatorState::Data ActuatorState::getData() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return {state_, value1_, value2_};
}

std::string ActuatorState::getStateString() const {
    std::lock_guard<std::mutex> lock(mtx_);
    switch (state_) {
        case PowerState::ON:  return "ON";
        case PowerState::OFF: return "OFF";
        default:              return "N/A";
    }
}
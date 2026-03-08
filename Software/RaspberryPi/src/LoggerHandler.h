#ifndef LOGGER_HANDLER_H
#define LOGGER_HANDLER_H

#include "IEventHandler.h"
#include "Events.h"
#include "ActuatorState.h"
#include "TopicLoader.h"
#include <vector>

class LoggerHandler : public IEventHandler {
public:
    LoggerHandler(const std::vector<ActuatorState>& states, const ConfigData& config);

    // This is triggered by the MultiHandler whenever an event occurs
    void handleEvent(const Event& e) override;

private:
    const std::vector<ActuatorState>& actuators_;
    const ConfigData& config_;

    // Helper to print current time
    void printTimestamp();
};

#endif
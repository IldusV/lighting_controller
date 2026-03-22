#ifndef DISPLAY_HANDLER_H
#define DISPLAY_HANDLER_H

#include "IEventHandler.h"
#include "ActuatorState.h"
#include "DisplayRenderer.h"

#include <memory>
#include <vector>

class DisplayHandler : public IEventHandler {
public:
    DisplayHandler(const std::vector<ActuatorState>& actuators, std::shared_ptr<DisplayRenderer> renderer)
        : renderer_(renderer), actuators_(actuators) {};

    void handleEvent(const Event& event) override;

private:
    std::shared_ptr<DisplayRenderer> renderer_;
    const std::vector<ActuatorState>& actuators_;
};

#endif // DISPLAY_HANDLER_H
#ifndef DISPLAY_RENDERER_H
#define DISPLAY_RENDERER_H

#include <vector>
#include <string>
#include <memory>
#include "IDisplayCanvas.h"
#include "ActuatorState.h"

// Simple POD (Plain Old Data) for the UI snapshot
struct SystemStatus {
    bool wifiConnected = false;
    int wifiSignal = 0;
    bool mqttConnected = false;
};

class DisplayRenderer {
public:
    explicit DisplayRenderer(std::shared_ptr<IDisplayCanvas> canvas);

    // Pure Function: Data in -> Pixels out
    // No internal state is stored between calls
    void render(const std::vector<ActuatorState>& actuators, const SystemStatus& sys);

private:
    std::shared_ptr<IDisplayCanvas> canvas_;

    // Maps the index/type to a specific icon in your theme folders
    std::string getIconForActuator(int index, const ActuatorState& state);
};

#endif // DISPLAY_RENDERER_H
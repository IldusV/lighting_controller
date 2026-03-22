#include "DisplayHandler.h"
#include "Events.h"
#include "ActuatorState.h"
#include "DisplayRenderer.h"
#include "SystemState.h"

#include <iostream>
#include <vector>
#include <memory>

void DisplayHandler::handleEvent(const Event& e) {
    // 1. Get the Source of Truth
    auto& ss = SystemState::getInstance();
    
    std::cout << "DisplayHandler received event of type: " << static_cast<int>(e.type()) << std::endl;
    // 2. Create the System Snapshot
    SystemStatus sys;
    sys.wifiConnected = (ss.getWiFiStatus() == ConnectionStatus::CONNECTED);
    sys.wifiSignal = ss.getWiFiSignalLevel();
    sys.mqttConnected = (ss.getMqttStatus() == ConnectionStatus::CONNECTED);

    // 3. Call the Renderer with BOTH sets of data
    // 'actuators_' is the vector we passed to the Handler at setup
    renderer_->render(actuators_, sys);
}
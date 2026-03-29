#include "DisplayRenderer.h"
#include <iostream>

// TODO: check Model-View-Controller (MVC) or Bridge patterns

DisplayRenderer::DisplayRenderer(std::shared_ptr<IDisplayCanvas> canvas)
    : canvas_(canvas) {
        canvas_->init();
        canvas_->setTheme({"modern", "./themes", 128, 64});
        canvas_->clear();
    }

void DisplayRenderer::render(const std::vector<ActuatorState>& actuators, const SystemStatus& sys) {
    std::cout << "DisplayRenderer rendering with " << actuators.size() 
              << " actuators and WiFi: " << sys.wifiConnected 
              << " MQTT: " << sys.mqttConnected << std::endl;
    // 1. Prepare the canvas
    //canvas_->clear();

    // 2. Draw Connectivity Chrome (Top Bar)
    // The Driver handles the actual theme-based icon placement
    canvas_->drawWifiStatus(sys.wifiConnected, sys.wifiSignal);
    //canvas_->drawMqttStatus(sys.mqttConnected);

    // 3. Draw Lighting Zones
    // We iterate through the provided vector. The Renderer decides 
    // the layout logic (e.g., only show the first 5 zones).
    for (size_t i = 0; i < actuators.size(); ++i) {
        if (i >= 5) break; // Screen capacity limit

        std::string iconName = getIconForActuator(static_cast<int>(i), actuators[i]);
        bool isOn = (actuators[i].getStateString() == "ON");
        
        // Use the slot-based system so the Driver handles coordinate math
        canvas_->drawActuatorIcon(static_cast<int>(i), iconName, isOn);
    }

    // 4. Draw Footer Logic
    // if (!sys.mqttConnected) {
    //     canvas_->drawFooter("OFFLINE - CHECK BROKER");
    // } else {
    //     canvas_->drawFooter("SYSTEM READY");
    // }

    // 5. Push to hardware
    canvas_->update();
}

std::string DisplayRenderer::getIconForActuator(int index, const ActuatorState& state) {
    // Logic to determine icon type. 
    // In the future, 'ActuatorState' could store a type enum (BULB, FAN, STRIP)
    return "light"; 
}
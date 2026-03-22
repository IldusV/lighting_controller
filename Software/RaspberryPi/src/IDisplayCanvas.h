#ifndef I_DISPLAY_CANVAS_H
#define I_DISPLAY_CANVAS_H

#include <string>

struct ThemeConfig {
    std::string name;
    std::string baseDir; // e.g., "/usr/share/controller/themes/modern"
    int width;
    int height;
};

class IDisplayCanvas {
public:
    virtual ~IDisplayCanvas() = default;

    // Lifecycle
    virtual void init() = 0;
    virtual void clear() = 0;
    virtual void update() = 0;
    virtual void setTheme(const ThemeConfig& config) = 0;

    // UI Components
    virtual void drawWifiStatus(bool connected, int signalLevel) = 0;
    virtual void drawMqttStatus(bool connected) = 0;
    virtual void drawActuatorIcon(int slot, const std::string& iconName, bool active) = 0;
    virtual void drawFooter(const std::string& text) = 0;
};

#endif
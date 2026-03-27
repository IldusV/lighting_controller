#ifndef U8G2_DRIVER_H
#define U8G2_DRIVER_H

#include "IDisplayCanvas.h"
#include "I2CSerialAdapter.h"

#include <memory>
#include <u8g2.h>

class U8G2Driver : public IDisplayCanvas {
public:
    U8G2Driver(std::shared_ptr<I2CSerialAdapter> adapter);
    ~U8G2Driver();

    void init() override;
    void clear() override;
    void update() override;
    void setTheme(const ThemeConfig& config) override;

    void drawWifiStatus(bool connected, int signalLevel) override;
    void drawMqttStatus(bool connected) override;
    void drawActuatorIcon(int slot, const std::string& iconName, bool active) override;
    void drawFooter(const std::string& text) override;

private:
    u8g2_t u8g2_;
    ThemeConfig currentTheme_;
    std::shared_ptr<I2CSerialAdapter> adapter_;
    
    // Internal helper to map 'slot' to X,Y coordinates
    void getCoordinatesForSlot(int slot, int& x, int& y);
};

#endif
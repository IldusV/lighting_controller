#ifndef I2C_SERIAL_ADAPTER_H
#define I2C_SERIAL_ADAPTER_H

#include "SerialInterface.h"
#include <cstdint>

// Forward Declaration: Tells the compiler "I2CManager is a class, 
// don't worry about the details yet."
class I2CManager;

class I2CSerialAdapter : public SerialInterface {
public:
    I2CSerialAdapter(I2CManager& manager, uint8_t slaveAddr);

    int send(const uint8_t* data, size_t length) override;
    int receive(uint8_t* buffer, size_t length) override;

private:
    I2CManager& manager_;
    uint8_t slaveAddr_;
};

#endif
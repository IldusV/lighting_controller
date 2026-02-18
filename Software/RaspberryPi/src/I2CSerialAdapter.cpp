#include "I2CSerialAdapter.h"
#include "I2CManager.h"

I2CSerialAdapter::I2CSerialAdapter(I2CManager& manager, uint8_t slaveAddr)
    : manager_(manager), slaveAddr_(slaveAddr) {}

int I2CSerialAdapter::send(const uint8_t* data, size_t length) {
    // The Adapter "Adapts" the generic call to the Manager's specific one
    return manager_.performTransfer(slaveAddr_, true, const_cast<uint8_t*>(data), length);
}

int I2CSerialAdapter::receive(uint8_t* buffer, size_t length) {
    return manager_.performTransfer(slaveAddr_, false, buffer, length);
}
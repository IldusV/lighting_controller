#ifndef I2C_MANAGER_H
#define I2C_MANAGER_H

#include <mutex>
#include <string>
#include <cstdint>

class I2CManager {
public:
    // The "Access Point" for the Singleton
    static I2CManager& getInstance();

    bool openBus(const std::string& device);
    int performTransfer(uint8_t addr, bool isWrite, uint8_t* data, size_t len);

private:
    // Singleton rules: Private constructor/destructor
    I2CManager() : fd(-1) {}
    ~I2CManager();

    // Prevent copying
    I2CManager(const I2CManager&) = delete;
    I2CManager& operator=(const I2CManager&) = delete;

    int fd;
    std::mutex busMutex;
};

#endif
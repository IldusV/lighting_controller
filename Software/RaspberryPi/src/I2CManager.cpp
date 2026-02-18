#include "I2CManager.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <iostream>
#include <cstring>

// 1. Initialize the Singleton instance (Thread-safe in C++11)
I2CManager& I2CManager::getInstance() {
    static I2CManager instance;
    return instance;
}

// 2. Destructor: Clean up the file descriptor
I2CManager::~I2CManager() {
    if (fd != -1) {
        close(fd);
        fd = -1;
    }
}

// 3. Open the I2C Bus
bool I2CManager::openBus(const std::string& device) {
    std::lock_guard<std::mutex> lock(busMutex);
    
    // Only open if not already open
    if (fd != -1) return true;

    fd = open(device.c_str(), O_RDWR);
    if (fd < 0) {
        std::cerr << "I2CManager: Failed to open " << device 
                  << " - " << strerror(errno) << std::endl;
        return false;
    }
    return true;
}

// 4. Perform Transfer (The critical section)
int I2CManager::performTransfer(uint8_t addr, bool isWrite, uint8_t* data, size_t len) {
    // LOCK THE BUS: No other thread can touch the I2C wires until this function returns
    std::lock_guard<std::mutex> lock(busMutex);

    if (fd < 0) return -1;

    // Set the slave address for this specific transaction
    if (ioctl(fd, I2C_SLAVE, addr) < 0) {
        std::cerr << "I2CManager: Failed to set addr 0x" << std::hex << (int)addr 
                  << " - " << strerror(errno) << std::dec << std::endl;
        return -1;
    }

    ssize_t result;
    if (isWrite) {
        result = write(fd, data, len);
    } else {
        result = read(fd, data, len);
    }

    if (result < 0) {
        // Log error but don't crash
        return -1;
    }

    return (int)result;
}
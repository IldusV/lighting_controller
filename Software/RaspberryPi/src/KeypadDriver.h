#ifndef KEYPAD_DRIVER_H
#define KEYPAD_DRIVER_H

#include "SerialInterface.h"

#include <pigpio.h>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <memory>

class KeypadDriver {
public:
    using KeyCallback = std::function<void(uint8_t)>;

    KeypadDriver(std::shared_ptr<SerialInterface> bus, int gpioPin, KeyCallback cb);
    ~KeypadDriver();

    void start();
    void stop();
    
    // API to send data (e.g., control LEDs)
    void sendLEDCommand(uint8_t data);

private:
    void run();
    static void gpioInterruptHandler(int gpio, int level, uint32_t tick, void* userdata);

    std::shared_ptr<SerialInterface> bus_;
    int gpioPin_;
    KeyCallback callback_;

    // Threading and Synchronization
    std::thread workerThread_;
    std::atomic<bool> running_{false};
    std::atomic<int> kb_read_pending_{0};
    std::mutex mtx_;
    std::condition_variable cv_;
};

#endif
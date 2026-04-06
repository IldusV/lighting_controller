#include "KeypadDriver.h"
#include <iostream>

KeypadDriver::KeypadDriver(std::shared_ptr<SerialInterface> bus, int gpioPin, KeyCallback cb)
    : bus_(bus), gpioPin_(gpioPin), callback_(cb) {
    
    if (gpioInitialise() < 0) {
        throw std::runtime_error("pigpio initialization failed");
    }

    gpioSetMode(gpioPin_, PI_INPUT);
    gpioSetPullUpDown(gpioPin_, PI_PUD_UP);
}

KeypadDriver::~KeypadDriver() {
    stop();
    gpioTerminate();
}

void KeypadDriver::start() {
    if (running_.load()) return;
    
    running_.store(true);

    // Register the static handler with 'this' as userdata
    gpioSetAlertFuncEx(gpioPin_, KeypadDriver::gpioInterruptHandler, this);

    workerThread_ = std::thread(&KeypadDriver::run, this);
}

void KeypadDriver::stop() {
    running_.store(false);
    cv_.notify_all(); // Wake up thread to exit
    if (workerThread_.joinable()) {
        workerThread_.join();
    }
    gpioSetAlertFunc(gpioPin_, nullptr); // Unregister
}

void KeypadDriver::sendLEDCommand(LedCommand ledCmd) {
    if (bus_) {
        std::array<uint8_t, 2> buf = {
            static_cast<uint8_t>(ledCmd.value & 0xFF),
            static_cast<uint8_t>(ledCmd.value >> 8)
        };

        bus_->send(buf.data(), buf.size());
    }
}

// Static bridge between C pigpio and C++ instance
void KeypadDriver::gpioInterruptHandler(int gpio, int level, uint32_t tick, void* userdata) {
    std::cout << "GPIO Interrupt: gpio=" << gpio << " level=" << level << " tick=" << tick << std::endl;
    if (level == 0) { // Falling edge (button press)
        KeypadDriver* instance = static_cast<KeypadDriver*>(userdata);
        {
            std::lock_guard<std::mutex> lock(instance->mtx_);
            instance->kb_read_pending_++;
        }
        instance->cv_.notify_one(); 
    }
}

void KeypadDriver::run() {
    uint8_t inputBuffer[1];

    while (running_.load()) {
        std::unique_lock<std::mutex> lock(mtx_);
        // Wait for interrupt or stop signal
        cv_.wait(lock, [this] { 
            return kb_read_pending_.load() > 0 || !running_.load(); 
        });

        if (!running_.load()) break;

        // While there are pending reads, process them
        while (kb_read_pending_.load() > 0) {
            int bytesRead = bus_->receive(inputBuffer, 1);
            if (bytesRead > 0 && callback_) {
                callback_(inputBuffer[0]);
            }
            kb_read_pending_--;
        }
    }
}
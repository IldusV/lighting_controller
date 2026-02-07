#ifndef KEYPAD_HANDLER_H
#define KEYPAD_HANDLER_H

#include <functional>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <stdexcept>
#include <cstring>
#include <chrono>
#include <thread>
#include <atomic>
#include <functional>
#include <string>


class KeypadHandler {
public:
    using Callback = std::function<void(unsigned char*, size_t size)>;

    KeypadHandler(Callback callback);
    ~KeypadHandler();

    void start();
    void stop();
    void send(const unsigned char* data);

private:
    void run();
    // static void gpioInterruptHandler(int gpio, int level, uint32_t tick);
    Callback callback;
    std::thread thread;
    std::atomic<bool> running;
    int file;
    static int kb_read_pending;  // Make it static
    int gpioPin_;
};

#endif // KEYPAD_HANDLER_H
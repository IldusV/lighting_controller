#include "KeypadHandler.h"
#include <pigpio.h>
#include <fcntl.h>

// Initialize the static variable outside the class definition
int KeypadHandler::kb_read_pending = 0;

namespace KeypadConfig {
    const int I2C_ADDRESS = 0x20;
    const std::string I2C_DEVICE_FILE = "/dev/i2c-1"; // Replace with your I2C device file path
}

KeypadHandler::KeypadHandler(Callback callback)
    : callback(callback), running(false), file(-1) {
    // Open the I2C device file
    file = open(KeypadConfig::I2C_DEVICE_FILE.c_str(), O_RDWR | O_NONBLOCK);
    if (file < 0) {
        throw std::runtime_error("Failed to open the I2C device");
    }

    // Set the I2C slave address
    if (ioctl(file, I2C_SLAVE, KeypadConfig::I2C_ADDRESS) < 0) {
        close(file);
        throw std::runtime_error("Failed to set I2C slave address");
    }

    gpioPin_ = 0x4;
    if (gpioInitialise() < 0) {
        std::cerr << "Failed to initialize pigpio!" << std::endl;
        throw std::runtime_error("pigpio initialization failed");
    }
    gpioSetMode(gpioPin_, PI_INPUT);
    gpioSetPullUpDown(gpioPin_, PI_PUD_UP);  // Enable pull-up resistor

    // Register the interrupt on falling edge
    //gpioSetAlertFunc(gpioPin_, &KeypadHandler::gpioInterruptHandler);
    //gpioSetISRFunc(gpioPin_, EITHER_EDGE, 0, NULL);  // Unregister any existing ISR
    //gpioSetISRFunc(gpioPin_, RISING_EDGE, 0, &KeypadHandler::gpioInterruptHandler);
    gpioSetAlertFunc(gpioPin_, [](int gpio, int level, uint32_t tick){
        if (level == 0) {
            std::cout << "Interrupt on GPIO " << gpio << std::endl;
            kb_read_pending++;
        }
    });
}

// void KeypadHandler::gpioInterruptHandler(int gpio, int level, uint32_t tick) {
//     if (level == 0) {
//         std::cout << "Interrupt on GPIO " << gpio << std::endl;
//         kb_read_pending++;
//     }
// }

KeypadHandler::~KeypadHandler() {
    std::cout << "kp destructor" << std::endl;
    stop();
    if (file >= 0) {
        close(file);
    }
    gpioTerminate();
}

void KeypadHandler::start() {
    if (running.load()) return;
    running.store(true);
    thread = std::thread(&KeypadHandler::run, this);
}

void KeypadHandler::stop() {
    std::cout << "stop()" << std::endl;
    if (!running.load()) return;
    running.store(false);
    if (thread.joinable()) {
        thread.join();
    }
}

void KeypadHandler::run() {
    const size_t payloadSize = 1;  // Fixed payload size of 2 bytes
    unsigned char buffer[payloadSize];

    // Set the file descriptor to non-blocking mode
    int flags = fcntl(file, F_GETFL, 0);
    fcntl(file, F_SETFL, flags | O_NONBLOCK);
    while (running.load()) {
        if (kb_read_pending) {
            // Attempt to read 2 bytes from the I²C device
            ssize_t bytesRead = read(file, buffer, payloadSize);
            if (bytesRead == payloadSize) {
                // Successfully read 2 bytes, invoke the callback
                callback(buffer, payloadSize);
            } else if (bytesRead < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    // No data available; retry later
                    std::cerr << "No data read from I2C device; retrying..." << std::endl;
                } else {
                    // Error occurred while reading
                    std::cerr << "Failed to read from the I2C device: " << strerror(errno) << std::endl;
                }
            } else if (bytesRead > 0) {
                // Partial read (unexpected for fixed-size payloads)
                std::cerr << "Incomplete read: expected " << payloadSize << " bytes, got " << bytesRead << std::endl;
            }

            //FIXME: remove this test code
            uint8_t outputBuffer[2];
            outputBuffer[0] = 0x55; // Your first byte
            outputBuffer[1] = 0xAA; // Your second byte

            ssize_t bytesWritten = write(file, outputBuffer, 2);

            if (bytesWritten != 2) {
                // Handle error: could not write to I2C device
                perror("I2C write failed");
            }

            kb_read_pending--;
        }

        //std::cout << "looping..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(300)); // Adjust sleep time as needed
    }
}

void KeypadHandler::send(const unsigned char* data) {
    if (data == nullptr) {
        std::cerr << "Invalid data pointer provided to send function.\n";
        return;
    }

    constexpr size_t data_length = 2;
    ssize_t bytes_written = write(file, data, data_length);
    if (bytes_written != data_length) {
        std::cerr << "Failed to write to the I2C bus. Bytes written: " << bytes_written << "\n";
    }
}

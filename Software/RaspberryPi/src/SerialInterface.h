#ifndef SERIALINTERFACE_H
#define SERIALINTERFACE_H

#include <vector>
#include <cstdint>
#include <cstddef>

class SerialInterface {
public:
    virtual ~SerialInterface() = default;

    virtual int send(const uint8_t* data, size_t length) = 0;
    virtual int receive(uint8_t* buffer, size_t length) = 0;
};

#endif
#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

// #include "SystemEventQueue.h"

#include <cstdint>
#include <mutex>
#include <functional>
#include <iostream>

enum class ConnectionStatus {
    DISCONNECTED = 0,
    CONNECTED = 1 
};

struct ConnectionInfo {
    ConnectionStatus status = ConnectionStatus::DISCONNECTED;
    int8_t signalLevel = 0;
};

class SystemState {
public:
    // Get the single instance (thread-safe in C++11+)
    static SystemState& getInstance() {
        static SystemState instance;
        return instance;
    }

    // Delete copy and move constructors to prevent copies
    SystemState(const SystemState&) = delete;
    SystemState& operator=(const SystemState&) = delete;
    SystemState(SystemState&&) = delete;
    SystemState& operator=(SystemState&&) = delete;

    // using SystemStateCallback = std::function<void()>;

    ConnectionInfo getWiFiInfo() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return wifi_;
    }

    ConnectionStatus getWiFiStatus() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return wifi_.status;
    }

    int8_t getWiFiSignalLevel() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return wifi_.signalLevel;
    }

    void setWiFiStatus(ConnectionStatus s, int8_t signal) {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            if (wifi_.status == s && wifi_.signalLevel == signal) {
                return; // No change, don't trigger updates
            }
            wifi_ = {s, signal};
            updated_ = true;
        }
        //SystemEventQueue::getInstance().push(SystemEventType::WIFI_UPDATED);
    }

    ConnectionInfo getMqttInfo() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return mqtt_;
    }

    ConnectionStatus getMqttStatus() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return mqtt_.status;
    }

    void setMqttStatus(ConnectionStatus s) {
        std::lock_guard<std::mutex> lock(mtx_);
        mqtt_.status = s;
    }

    bool updated() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return updated_;
    }

    void clearUpdated() {
        std::lock_guard<std::mutex> lock(mtx_);
        updated_ = false;
    }

    // void setOnChange_cb(SystemStateCallback cb) {
    //     onChange = cb;
    // }

private:
    // SystemStateCallback onChange;

    // Private constructor - only called by getInstance()
    SystemState() = default;
    bool updated_ = false;
    mutable std::mutex mtx_;
    ConnectionInfo wifi_{ConnectionStatus::DISCONNECTED, 0};
    ConnectionInfo mqtt_{ConnectionStatus::DISCONNECTED, 0};
};


#endif // SYSTEM_STATE_H
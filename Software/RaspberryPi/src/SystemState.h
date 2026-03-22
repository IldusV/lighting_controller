#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

#include <cstdint>
#include <mutex>

enum class ConnectionStatus { DISCONNECTED = 0, CONNECTED = 1 };

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

    // WiFi getters
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

    // WiFi setters
    void setWiFiStatus(ConnectionStatus s, int8_t signal) {
        std::lock_guard<std::mutex> lock(mtx_);
        wifi_.status = s;
        wifi_.signalLevel = signal;
    }

    // MQTT getters
    ConnectionInfo getMqttInfo() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return mqtt_;
    }

    ConnectionStatus getMqttStatus() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return mqtt_.status;
    }

    // MQTT setters
    void setMqttStatus(ConnectionStatus s) {
        std::lock_guard<std::mutex> lock(mtx_);
        mqtt_.status = s;
    }

private:
    // Private constructor - only called by getInstance()
    SystemState() = default;
    mutable std::mutex mtx_;
    ConnectionInfo wifi_{ConnectionStatus::DISCONNECTED, 0};
    ConnectionInfo mqtt_{ConnectionStatus::DISCONNECTED, 0};
};


#endif // SYSTEM_STATE_H
#ifndef SYSTEM_EVENT_QUEUE_H
#define SYSTEM_EVENT_QUEUE_H

#include <queue>
#include <mutex>

enum class SystemEventType {
    WIFI_UPDATED,
    MQTT_UPDATED,
    TIMER_TICK
};

class SystemEventQueue {
public:
    static SystemEventQueue& getInstance() {
        static SystemEventQueue instance;
        return instance;
    }

    void push(SystemEventType type) {
        std::lock_guard<std::mutex> lock(mtx_);
        events_.push(type);
    }

    bool pop(SystemEventType& type) {
        std::lock_guard<std::mutex> lock(mtx_);
        if (events_.empty()) return false;
        type = events_.front();
        events_.pop();
        return true;
    }

private:
    SystemEventQueue() = default;
    std::queue<SystemEventType> events_;
    std::mutex mtx_;
};

#endif // SYSTEM_EVENT_QUEUE_H
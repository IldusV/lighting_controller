#ifndef MQTT_EVENT_QUEUE_H
#define MQTT_EVENT_QUEUE_H

#include "Events.h"

#include <queue>
#include <mutex>

class MqttEventQueue {
public:
    static MqttEventQueue& getInstance() {
        static MqttEventQueue instance;
        return instance;
    }

    void push(MqttEvent event) {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            // Use std::move to transfer ownership of the internal strings
            // from the 'event' parameter into the queue container.
            events_.push(std::move(event));
        }
        cv_.notify_one();
    }

    bool pop(MqttEvent& event) {
        std::lock_guard<std::mutex> lock(mtx_);
        if (events_.empty()) 
            return false;

        event = std::move(events_.front());
        events_.pop();
        return true;
    }

    bool wait_for_data(int timeout_ms) {
        std::unique_lock<std::mutex> lock(mtx_);
        return cv_.wait_for(lock, std::chrono::milliseconds(timeout_ms), [this] {
            return !events_.empty();
        });
    }

private:
    MqttEventQueue() = default;
    std::queue<MqttEvent> events_;
    std::mutex mtx_;
    std::condition_variable cv_;
};

#endif // MQTT_EVENT_QUEUE_H
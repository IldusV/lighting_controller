#ifndef BUTTON_EVENT_QUEUE_H
#define BUTTON_EVENT_QUEUE_H

#include "Events.h"

#include <queue>
#include <mutex>
#include <condition_variable>

class ButtonEventQueue {
public:
    static ButtonEventQueue& getInstance() {
        static ButtonEventQueue instance;
        return instance;
    }

    void push(const ButtonEvent& event) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(event);
        cv_.notify_one();
    }

    bool pop(ButtonEvent& event) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty())
            return false;
        event = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    bool wait_for_data(int timeout_ms) {
        std::unique_lock<std::mutex> lock(mutex_);
        return cv_.wait_for(lock, std::chrono::milliseconds(timeout_ms), [this] {
            return !queue_.empty();
        });
    }

private:
    ButtonEventQueue() = default;
    std::queue<ButtonEvent> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
};

#endif // BUTTON_EVENT_QUEUE_H
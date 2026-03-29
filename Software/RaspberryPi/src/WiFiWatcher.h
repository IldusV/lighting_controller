#ifndef WIFI_WATCHER_H
#define WIFI_WATCHER_H

#include <string>
#include <functional>
#include <atomic>
#include <cstdint>
#include <thread>

class WiFiWatcher {
public:
    using WiFiStatusCallback = std::function<void(bool connected, int8_t signalLevel)>;

    WiFiWatcher(WiFiStatusCallback callback);
    ~WiFiWatcher();

    void start();
    void stop();

private:
    void watchLoop();
    void parseProcNet(const std::string& interface);

    WiFiStatusCallback callback_;
    std::thread workerThread_;
    std::atomic<bool> running_{false};
    const int pollIntervalS = 5;
};

#endif
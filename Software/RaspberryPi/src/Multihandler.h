#ifndef MULTIHANDLER_H
#define MULTIHANDLER_H

#include "Events.h"
#include "IEventHandler.h"

#include <vector>
#include <memory>

class MultiHandler : public IEventHandler {
public:
    void addHandler(std::shared_ptr<IEventHandler> h) {
        handlers_.push_back(h);
    }
    void handleEvent(const Event& e) override {
        for (auto& h : handlers_) h->handleEvent(e);
    }
private:
    std::vector<std::shared_ptr<IEventHandler>> handlers_;
};

#endif // MULTIHANDLER_H
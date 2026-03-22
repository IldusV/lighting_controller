#ifndef I_EVENT_HANDLER_H
#define I_EVENT_HANDLER_H

#include "Events.h"

class IEventHandler {
public:
    virtual ~IEventHandler() = default;
    virtual void handleEvent(const Event& e) = 0;
};

#endif
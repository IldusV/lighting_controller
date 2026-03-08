#include "Events.h"
#include "ActuatorState.h"
#include "MqttMgr.h"
#include "TopicLoader.h"
#include "IEventHandler.h"


class CommandManager : public IEventHandler {
public:
    CommandManager(MqttMgr& mqtt, std::vector<ActuatorState>& states, ConfigData config);
    void handleEvent(const Event& e) override;
    void subscribeAll();

private:
    void handleButton(uint8_t code);
    void handleMqtt(const std::string& topic, const std::string& payload);

    MqttMgr& mqtt_;
    std::vector<ActuatorState>& actuators_;
    ConfigData config_;
};
#include "I2CManager.h"
#include "I2CSerialAdapter.h"
#include "KeypadDriver.h"
#include "MqttMgr.h"
#include "Events.h"
#include "CommandManager.h"
#include "Multihandler.h"
#include "TopicLoader.h"
#include "ActuatorState.h"
#include "LoggerHandler.h"
#include "DisplayRenderer.h"
#include "DisplayHandler.h"
#include "U8G2Driver.h"
#include "SystemState.h"
#include "MqttEventQueue.h"
#include "WiFiWatcher.h"
#include "ButtonEventQueue.h"

#include <unistd.h>
#include <iostream>
#include <stdio.h>

// void on_keypad_msg_received(unsigned char* payload, size_t size) {
//     std::cout << "Payload received: ";
//     for (size_t i = 0; i < size; ++i) {
//         std::cout << std::hex << static_cast<int>(payload[i]) << " ";
//     }
//     std::cout << std::endl;
// }

// void on_mqtt_msg_received(const std::string& topic, const std::string& payload) {
//     std::cout << "Received message on topic: " << topic
//               << " with payload: " << payload << std::endl;
// }

#define ACTUATOR_COUNT 5

int main() {
    const std::string broker_address = "tcp://127.0.0.1:1883";
    const std::string client_id = "paho_client";

    SystemState& sysState = SystemState::getInstance();

    // MqttMgr mqtt_mgr(broker_address, client_id);
    I2CManager& i2cBus = I2CManager::getInstance();

    try {
        i2cBus.openBus("/dev/i2c-1");
    }
    catch (const std::exception& e) {
        std::cerr << "Error opening I2C bus: " << e.what() << std::endl;
    }

    auto wifiWatcher = std::make_shared<WiFiWatcher>([](bool connected, int8_t signalLevel) {
        if (connected) {
            std::cout << "WiFiWatcher: Connected with signal level " << static_cast<int>(signalLevel) << std::endl;
            SystemState::getInstance().setWiFiStatus(ConnectionStatus::CONNECTED, signalLevel);
        } else {
            std::cout << "WiFiWatcher: Disconnected" << std::endl;
            SystemState::getInstance().setWiFiStatus(ConnectionStatus::DISCONNECTED, 0);
        }
    });
    
    auto keypadBus = std::make_shared<I2CSerialAdapter>(i2cBus, 0x20);
    auto displayBus = std::make_shared<I2CSerialAdapter>(i2cBus, 0x3c);

    auto config = TopicLoader::load("topics.txt");

    std::vector<ActuatorState> actuators(ACTUATOR_COUNT);
    actuators[0].setSelected(true); // By default select the first actuator

    MqttMgr mqtt(broker_address, client_id);

    auto dispatcher = std::make_shared<MultiHandler>();
    auto cmdMgr = std::make_shared<CommandManager>(mqtt, actuators, config);
    auto logger = std::make_shared<LoggerHandler>(actuators, config);

    auto displayCanvas = std::make_shared<U8G2Driver>(displayBus);
    auto displayRenderer = std::make_shared<DisplayRenderer>(displayCanvas);
    auto displayHandler = std::make_shared<DisplayHandler>(actuators, displayRenderer);

    dispatcher->addHandler(cmdMgr);
    dispatcher->addHandler(logger);
    dispatcher->addHandler(displayHandler);

    KeypadDriver keypad(keypadBus, 4, [&](uint8_t code) {
        ButtonEventQueue::getInstance().push(ButtonEvent(code));
        std::cout << "Pushed Button event to the queue, code: 0x" << std::hex << (int)code << std::endl;
    });

    mqtt.set_connection_callback([&](bool connected) {
        if (connected) {
            std::cout << "MQTT Connected callback: Connected to broker" << std::endl;
            SystemState::getInstance().setMqttStatus(ConnectionStatus::CONNECTED);
        } else {
            std::cout << "MQTT Connected callback: Disconnected from broker" << std::endl;
            SystemState::getInstance().setMqttStatus(ConnectionStatus::DISCONNECTED);
        }
    });

    mqtt.set_message_callback([&](const std::string& t, const std::string& p) {
        MqttEventQueue::getInstance().push(MqttEvent(t, p));
        std::cout << "Pushed MQTT event to the queue, topic: " << t << " with payload: " << p << std::endl;
    });

    mqtt.connect();
    cmdMgr->subscribeAll();
    keypad.start();
    wifiWatcher->start();

    KeypadDriver::LedCommand ledCmd = {0};
    KeypadDriver::LedCommand prevLedCmd = {0};

    while(1)
    {
        //MqttEventQueue::getInstance().wait_for_data(1000);
        std::cout << "main thread..." << std::endl;

        if (sysState.updated()) {
            dispatcher->handleEvent(SystemUpdateEvent());
            std::cout << "System state updated, dispatched SystemUpdateEvent" << std::endl;
            sysState.clearUpdated();
        }

        MqttEvent mqttEvent("", "");
        while (MqttEventQueue::getInstance().pop(mqttEvent)) {
            dispatcher->handleEvent(mqttEvent);
            std::cout << "Processed MQTT event from queue, topic: " << mqttEvent.topic() << std::endl;
        }

        size_t i = 0;
        ledCmd.value = 0;
        for (auto &actuator : actuators) {
            auto data = actuator.getData();

            if (data.state == PowerState::ON) {
                ledCmd.value |= (1u << i);
            }

            if (actuator.isSelected()) {
                ledCmd.value |= (1u << (i + 10));
            }
            i++;
        }
        if (ledCmd.value != prevLedCmd.value) {
            prevLedCmd = ledCmd;
            std::cout << "Sending LED command: 0x" << std::hex << ledCmd.value << std::dec << std::endl;
            keypad.sendLEDCommand(ledCmd);
        }

        ButtonEvent buttonEvent(0);
        while (ButtonEventQueue::getInstance().pop(buttonEvent)) {
            dispatcher->handleEvent(buttonEvent);
            std::cout << "Processed Button event from queue, code: 0x" << std::hex << (int)buttonEvent.getCode() << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(300)); // Adjust sleep time as needed
    }

    return 0;
}

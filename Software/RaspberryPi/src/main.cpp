#include "KeypadHandler.h"
#include "MqttMgr.h"
#include <iostream>

void on_keypad_msg_received(unsigned char* payload, size_t size) {
    std::cout << "Payload received: ";
    for (size_t i = 0; i < size; ++i) {
        std::cout << std::hex << static_cast<int>(payload[i]) << " ";
    }
    std::cout << std::endl;
}

void on_mqtt_msg_received(const std::string& topic, const std::string& payload) {
    std::cout << "Received message on topic: " << topic
              << " with payload: " << payload << std::endl;
}

int main() {
    const std::string broker_address = "tcp://127.0.0.1:1883";
    const std::string client_id = "paho_client";

    MqttMgr mqtt_mgr(broker_address, client_id);
    KeypadHandler keypad(on_keypad_msg_received);
//FIXME: move inits out of destructor

    try {
        mqtt_mgr.set_message_callback(on_mqtt_msg_received);

        mqtt_mgr.connect();

        mqtt_mgr.subscribe("home/light/light1/status");

        mqtt_mgr.publish("home/light/light1/command", "ON");

        //mqtt_mgr.disconnect();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

        
        keypad.start();

        // unsigned char data_to_send[2] = { 0xA5, 0x5A };
        // keypad.send(data_to_send);

        //std::this_thread::sleep_for(std::chrono::seconds(10));
        //keypad.stop();


    while(1)
    {
        std::cout << "main thread..." << std::endl;
        // Sleep to avoid high CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // Adjust sleep time as needed
    }

    return 0;
}

#include "I2CManager.h"
#include "I2CSerialAdapter.h"
#include "KeypadDriver.h"
#include "MqttMgr.h"
#include "linux-i2c.h"
#include "Events.h"
#include "CommandManager.h"
#include "Multihandler.h"
#include "TopicLoader.h"
#include "ActuatorState.h"
#include "LoggerHandler.h"

#include <unistd.h>
#include <u8g2.h>
#include <iostream>
#include <stdio.h>

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

u8g2_t u8g2;

void show_welcome(void) {
    u8g2_Setup_st75256_i2c_jlx25664_f(&u8g2, U8G2_R0, u8x8_byte_linux_i2c, u8x8_linux_i2c_delay);

    u8g2_SetI2CAddress(&u8g2, 0x3c); 
 
    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);
	u8g2_SetContrast(&u8g2, 75);

	u8g2_ClearBuffer(&u8g2);

    int size = 1;
    bool running = true;

    while (running) {
        u8g2_ClearBuffer(&u8g2);

        // Calculate center-based coordinates so it grows from the middle
        // Center is roughly 128, 32
        int x = 128 - (size / 2);
        int y = 32 - (size / 2);

        // Draw the frame
        u8g2_DrawFrame(&u8g2, x, y, size, size);

        // Optional: Print the current size for debugging
        // u8g2_SetFont(&u8g2, u8g2_font_5x7_tr);
        // char buf[10];
        // sprintf(buf, "%d", size);
        // u8g2_DrawStr(&u8g2, 0, 10, buf);

        u8g2_SendBuffer(&u8g2);

        // Increase size
        size += 2;

        // Reset if it hits the top/bottom border (64 pixels high)
        // or the side borders (though Y is the limiting factor here)
        if (size >= 62) {
            size = 1;
            // Optional: clear screen briefly on reset
            u8g2_ClearDisplay(&u8g2); 
        }

        // Animation speed: ~30ms delay (approx 30 FPS)
        usleep(30000);
    }
    
    u8g2_DrawFrame(&u8g2, 124, 28, 8, 8);
	u8g2_SetFont(&u8g2, u8g2_font_ncenB08_tr);
	u8g2_DrawStr(&u8g2, 0, 24, "welcome to keyboard"); 
	u8g2_SendBuffer(&u8g2);
}

int main() {
     const std::string broker_address = "tcp://127.0.0.1:1883";
     const std::string client_id = "paho_client";

    // MqttMgr mqtt_mgr(broker_address, client_id);
    I2CManager& i2cBus = I2CManager::getInstance();

    try {
        i2cBus.openBus("/dev/i2c-1");
    }
    catch (const std::exception& e) {
        std::cerr << "Error opening I2C bus: " << e.what() << std::endl;
    }
    auto keypadBus = std::make_shared<I2CSerialAdapter>(i2cBus, 0x20);

    // std::unique_ptr<KeypadDriver> keypad;

    // auto myCallback = [](uint8_t keyPressed) {
    //     std::cout << ">> Client received key: 0x" << std::hex << (int)keyPressed << std::endl;
    // };

    // try { 
    //     keypad = std::unique_ptr<KeypadDriver>(new KeypadDriver(keypadBus, 4, myCallback));
    //     keypad->start();
    //     std::cout << "Keypad driver started. Waiting for interrupts..." << std::endl;
    // }
    // catch (const std::exception& e) {
    //     std::cerr << "Error initializing KeypadDriver: " << e.what() << std::endl;
    // }

    // ---------------------------------------------------
    // 1. Initialise Shared Data & Config
    auto config = TopicLoader::load("topics.txt");

    std::vector<ActuatorState> actuators(10); // Scalable to any size

    // 2. Initialise Managers
    MqttMgr mqtt(broker_address, client_id);
    
    auto dispatcher = std::make_shared<MultiHandler>();
    auto cmdMgr = std::make_shared<CommandManager>(mqtt, actuators, config);
    auto logger = std::make_shared<LoggerHandler>(actuators, config);

    dispatcher->addHandler(cmdMgr);
    dispatcher->addHandler(logger);

    // 3. Connect Hardware to Event System
    KeypadDriver keypad(keypadBus, 4, [&](uint8_t code) {
        dispatcher->handleEvent(ButtonEvent(code));
        std::cout << ">> Client received key: 0x" << std::hex << (int)code << std::endl;
    });

    mqtt.set_message_callback([&](const std::string& t, const std::string& p) {
        dispatcher->handleEvent(MqttEvent(t, p));
        std::cout << "Received message on topic: " << t
                  << " with payload: " << p << std::endl;
    });

    // 4. Execution
    mqtt.connect();
    cmdMgr->subscribeAll();
    keypad.start();

    //FIXME: move inits out of destructor

    // try {
    //     mqtt_mgr.set_message_callback(on_mqtt_msg_received);

    //     mqtt_mgr.connect();

    //     mqtt_mgr.subscribe("home/light/light1/status");

    //     mqtt_mgr.publish("home/light/light1/command", "ON");

    //     //mqtt_mgr.disconnect();
    // } catch (const std::exception& e) {
    //     std::cerr << "Error: " << e.what() << std::endl;
    // }

    // keypad.start();

    // unsigned char data_to_send[2] = { 0xA5, 0x5A };
    // keypad.send(data_to_send);

    //std::this_thread::sleep_for(std::chrono::seconds(10));
    //keypad.stop();
    //show_welcome();


    while(1)
    {
        std::cout << "main thread..." << std::endl;
        // Sleep to avoid high CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // Adjust sleep time as needed
        // keypad->sendLEDCommand(0xFF); // Example: Send a command to turn on LEDs
    }

    return 0;
}

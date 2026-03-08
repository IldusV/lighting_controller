#include "MqttMgr.h"
#include <iostream>
#include <stdexcept>

// MqttMgr::MqttMgr(const std::string& broker_address, const std::string& client_id)
//     : client_(broker_address, client_id) {
//     client_.set_callback(*this); // Set this class as the callback handler
// }

MqttMgr::MqttMgr(const std::string& broker_address, const std::string& client_id)
    : client_(broker_address, client_id) {
    client_.set_callback(*this); // Registers this instance to receive overrides
}
// void MqttMgr::connect() {
//     try {
//         std::cout << "Connecting to the broker..." << std::endl;
//         mqtt::connect_options conn_opts;
//         conn_opts.set_clean_session(true);
//         client_.connect(conn_opts)->wait();
//         std::cout << "Connected to the broker!" << std::endl;
//     } catch (const mqtt::exception& e) {
//         throw std::runtime_error("Failed to connect to the MQTT broker: " + std::string(e.what()));
//     }
// }
void MqttMgr::connect() {
    try {
        std::cout << "Connecting to the broker..." << std::endl;
        mqtt::connect_options conn_opts;
        conn_opts.set_clean_session(true);
        
        // 1. Establish the connection
        client_.connect(conn_opts)->wait();
        
        // 2. CRITICAL: Start the background worker thread
        client_.start_consuming(); 
        
        std::cout << "Connected and consuming messages!" << std::endl;
    } catch (const mqtt::exception& e) {
        throw std::runtime_error("Failed to connect: " + std::string(e.what()));
    }
}

void MqttMgr::disconnect() {
    try {
        std::cout << "Disconnecting from the broker..." << std::endl;
        client_.disconnect()->wait();
        std::cout << "Disconnected from the broker!" << std::endl;
    } catch (const mqtt::exception& e) {
        std::cerr << "Error during disconnect: " << e.what() << std::endl;
    }
}

void MqttMgr::publish(const std::string& topic, const std::string& payload) {
    try {
        mqtt::message_ptr msg = mqtt::make_message(topic, payload);
        client_.publish(msg)->wait();
        std::cout << "Message published to topic " << topic << ": " << payload << std::endl;
    } catch (const mqtt::exception& e) {
        std::cerr << "Failed to publish message: " << e.what() << std::endl;
    }
}

void MqttMgr::subscribe(const std::string& topic) {
    try {
        client_.subscribe(topic, 1)->wait();
        std::cout << "Subscribed to topic: " << topic << std::endl;
    } catch (const mqtt::exception& e) {
        std::cerr << "Failed to subscribe to topic: " << e.what() << std::endl;
    }
}

void MqttMgr::set_message_callback(message_handler cb) {
    message_callback_ = cb;
}

void MqttMgr::connected(const std::string& cause) {
    std::cout << "Connected: " << cause << std::endl;
}

void MqttMgr::connection_lost(const std::string& cause) {
    std::cerr << "Connection lost: " << cause << std::endl;
}

void MqttMgr::message_arrived(mqtt::const_message_ptr msg) {
    std::cout << "Message arrived on topic: " << msg->get_topic()
              << " with payload: " << msg->to_string() << std::endl;

    // Invoke the user-defined callback if set
    if (message_callback_) {
        message_callback_(msg->get_topic(), msg->to_string());
    }
}

void MqttMgr::delivery_complete(mqtt::delivery_token_ptr token) {
    // This is called when a message you published successfully reaches the broker.
    // We can leave it empty for now.
}
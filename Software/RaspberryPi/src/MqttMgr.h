#ifndef MQTT_MGR_H
#define MQTT_MGR_H

#include <mqtt/async_client.h>
#include <string>
#include <functional>

// Use virtual inheritance as per Paho C++ best practices for callbacks
class MqttMgr : public virtual mqtt::callback {
public:
    using message_handler = std::function<void(const std::string& topic, const std::string& payload)>;
    using connection_handler = std::function<void(bool connected)>;

    // Constructor
    MqttMgr(const std::string& broker_address, const std::string& client_id);

    // Connects to the MQTT broker
    void connect();

    // Disconnects from the MQTT broker
    void disconnect();

    // Publishes a message to a given topic
    void publish(const std::string& topic, const std::string& payload);

    // Subscribes to a topic
    void subscribe(const std::string& topic);

    // Sets the callback to handle incoming messages
    void set_message_callback(message_handler cb);

    // Sets the callback to handle connection status changes
    void set_connection_callback(connection_handler cb);

    // Override callback methods from mqtt::callback
    void connected(const std::string& cause) override;
    void connection_lost(const std::string& cause) override;
    void message_arrived(mqtt::const_message_ptr msg) override;
    void delivery_complete(mqtt::delivery_token_ptr token) override;

private:
    mqtt::async_client client_;               // MQTT client instance
    message_handler message_callback_;        // Callback for handling received messages
    connection_handler connection_callback_;  // Callback for handling connection status changes
};

#endif // MQTT_MGR_H
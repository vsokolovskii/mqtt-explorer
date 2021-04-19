
#include "reciever.h"

Reciever::Reciever(const std::string &server_address, const std::string &id) 
         : _connection_listener(this, dummy_cb, on_connection_failure_cb),
           _subscribe_listener(this, on_subscribe_success_cb, on_subscribe_failure_cb),
           _dummy_listener(this, dummy_cb, dummy_cb),
           _listeners(_connection_listener, _subscribe_listener, _dummy_listener, _dummy_listener, _dummy_listener),
           _callbacks(this, on_connected_cb, on_message_arrived_cb, on_connection_lost_cb, on_delivery_complete_dummy_cb),
           _client(server_address, id, _listeners, _callbacks) 
{
    _mutex = new std::mutex();
}

Reciever::~Reciever()
{
    delete _mutex;
}

void Reciever::on_connected_cb(void *object, const std::string &cause)
{
    Reciever *reciever = static_cast<Reciever *>(object);
    reciever->on_connected(cause);
}

void Reciever::on_message_arrived_cb(void *object, mqtt::const_message_ptr message)
{
    Reciever *reciever = static_cast<Reciever *>(object);
    reciever->on_message_arrived(message);
}

void Reciever::on_connection_lost_cb(void *object, const std::string &cause)
{
    Reciever *reciever = static_cast<Reciever *>(object);
    reciever->on_connection_lost(cause);
}

void Reciever::on_delivery_complete_dummy_cb(void *object, mqtt::delivery_token_ptr token)
{
    //dummy
    (void)object;
    (void)token;
}

void Reciever::dummy_cb(void *object, const mqtt::token &token)
{
    //dummy
    (void)object;
    (void)token;
}

void Reciever::on_connection_failure_cb(void *object, const mqtt::token &token)
{
    Reciever *reciever = static_cast<Reciever *>(object);
    reciever->on_connection_failure(token);
}
void Reciever::on_subscribe_success_cb(void *object, const mqtt::token &token)
{
    Reciever *reciever = static_cast<Reciever *>(object);
    reciever->on_subscribe_success(token);
}
void Reciever::on_subscribe_failure_cb(void *object, const mqtt::token &token)
{
    Reciever *reciever = static_cast<Reciever *>(object);
    reciever->on_subscribe_failure(token);
}

void Reciever::on_connected(const std::string &cause)
{
    std::cerr << "Reciever connected: " << cause << std::endl;

    for (auto &topic: _topics)
    {
        std::cerr << "Reciever is subscribing to topic: " << topic << std::endl;
        _client.subscribe(topic, 1);
    }
}

void Reciever::on_message_arrived(mqtt::const_message_ptr message)
{
    Message parsed_message;
    if (_parser.parse_message(message, parsed_message))
    {
        return;
    }

    try
    {
        _map.at(parsed_message.topic + parsed_message.id)->on_message_arrived(parsed_message.state, _client, *_mutex);
    }
    catch(const std::out_of_range& e)
    {
        std::cerr << e.what() << std::endl;
    }
}

void Reciever::on_connection_lost(const std::string &cause)
{
    std::cerr << "Reciever lost connection: " << cause << std::endl;
}

template<typename T> void Reciever::register_device(std::vector<T> &devices)
{
    for (auto &device: devices)
    {
        if (std::find(_topics.begin(), _topics.end(), device.recv_topic) == _topics.end())
        {
            _topics.push_back(device.recv_topic);
        }

        _map[device.recv_topic + device.id] = &device;
    }
}

void Reciever::regiser_devices(Devices &devices)
{
    register_device<Light>(devices.lights);
    register_device<Lock>(devices.locks);
    register_device<Valve>(devices.valves);
    register_device<Thermostat>(devices.thermostats);
    register_device<Relay>(devices.relays);
}

void Reciever::on_connection_failure(const mqtt::token &token)
{
    std::cerr << "Reciever failed to connect with code: " << token.get_reason_code() << std::endl;
}

void Reciever::on_subscribe_success(const mqtt::token &token)
{
    std::cerr << "Reciever subscribed successfuly to topic: " << token.get_topics() << std::endl;
}

void Reciever::on_subscribe_failure(const mqtt::token &token)
{
    std::cerr << "Reciever could not subscribe to topics: " << token.get_topics() << std::endl;
}

bool Reciever::start_recieving(const mqtt::connect_options &connect_options)
{
    std::cerr << "Connecting reciever..." << std::endl;
    return _client.connect(connect_options);
}

bool Reciever::stop_recieving()
{
    return _client.disconnect();
}

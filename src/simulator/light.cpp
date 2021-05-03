
#include "light.h"

Light::Light(std::string topic, std::string name, int period, std::string id, std::string recv_topic)
      : RecievingAndPublishingDevice(topic, name, period, id, recv_topic) { }

void Light::add_state(std::string state)
{
    _states.push_back(state);
}

void Light::run(mqtt::client &client, const bool &run, std::mutex &mutex, std::future<void> future)
{
    mqtt::message_ptr message = mqtt::make_message(_topic, _name);
    message->set_qos(1);

    auto state_generator = std::bind(std::uniform_int_distribution<int>(0, _states.size() - 1), 
                                     std::default_random_engine(time(nullptr)));
    state_generator();

    int new_state = 0;
    std::string message_str;

    future.wait_for(std::chrono::seconds(_period));
    while (run)
    {
        message_str = _name;
        new_state = state_generator();

        _mutex->lock();
            if (new_state != _state)
            {
                message_str += ": state changed locally: " + _states[new_state];
            }
            else
            {
                message_str += ": state unchanged: " + _states[new_state];
            }
            _state = new_state;


            message->set_payload(message_str.c_str(), message_str.size());

            mutex.lock();
                client.publish(message);
            mutex.unlock();

            Log::log(message_str);
        _mutex->unlock();

        future.wait_for(std::chrono::seconds(_period));
    }
}

void Light::on_message_arrived(std::string state, Client &client, std::mutex &mutex)
{
    std::string message_str = _name;
    auto iterator = std::find(_states.begin(), _states.end(), state);

    if (iterator == _states.end())
    {
        message_str += ": change unsuccessful, unknow state: " + state;
        Log::log(message_str);
        mutex.lock();
            client.publish(_topic, message_str);
        mutex.unlock();
        return;
    }

    int index = iterator - _states.begin();

    _mutex->lock();
        if (index != _state)
        {
            message_str += ": state changed via message: " + state;
        }
        else
        {
            message_str += ": message reacieved, state remains unchanged: " + state;
        }
        _state = index;
        
        mutex.lock();
            client.publish(_topic, message_str);
        mutex.unlock();
        Log::log(message_str);
    _mutex->unlock();
}
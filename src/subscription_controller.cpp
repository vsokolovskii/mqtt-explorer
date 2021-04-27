#include "subscription_controller.h"

SubscriptionController::SubscriptionController() {}

void SubscriptionController::on_subscribe_success_cb(void *object, const mqtt::token &token)
{
    SubscriptionController *subscription_controller = static_cast<SubscriptionController *>(object);
    subscription_controller->on_subscribe_success(token);
}

void SubscriptionController::on_subscribe_failure_cb(void *object, const mqtt::token &token)
{
    SubscriptionController *subscription_controller = static_cast<SubscriptionController *>(object);
    subscription_controller->on_subscribe_failure(token);
}

void SubscriptionController::on_unsubscribe_success_cb(void *object, const mqtt::token &token)
{
    SubscriptionController *subscription_controller = static_cast<SubscriptionController *>(object);
    subscription_controller->on_unsubscribe_success(token);
}
void SubscriptionController::on_unsubscribe_failure_cb(void *object, const mqtt::token &token)
{
    SubscriptionController *subscription_controller = static_cast<SubscriptionController *>(object);
    subscription_controller->on_unsubscribe_failure(token);
}

void SubscriptionController::on_subscribe_success(const mqtt::token &token)
{
    const mqtt::string_collection *topics = token.get_topics().get();
    if (topics == nullptr)
    {
        return;
    }
    for (unsigned i = 0; i < topics->size(); i++)
    {
        Log::log("Successful subscription to topic: " + (*topics)[i]);
        emit subscription_success(QString::fromStdString((*topics)[i]));
    }
}

void SubscriptionController::on_subscribe_failure(const mqtt::token &token)
{
    const mqtt::string_collection *topics = token.get_topics().get();
    if (topics == nullptr)
    {
        return;
    }
    for (unsigned i = 0; i < topics->size(); i++)
    {
        Log::log("Unsuccessful subscription to topic: " + (*topics)[i]);
        emit subscription_failure(QString::fromStdString((*topics)[i]));
    }
}

void SubscriptionController::on_unsubscribe_success(const mqtt::token &token)
{
    const mqtt::string_collection *topics = token.get_topics().get();
    if (topics == nullptr)
    {
        return;
    }
    for (unsigned i = 0; i < topics->size(); i++)
    {
        Log::log("Successful unsubscription of topic: " + (*topics)[i]);
        emit unsubscription_success(QString::fromStdString((*topics)[i]));
    }
}

void SubscriptionController::on_unsubscribe_failure(const mqtt::token &token)
{
    const mqtt::string_collection *topics = token.get_topics().get();
    if (topics == nullptr)
    {
        return;
    }
    for (unsigned i = 0; i < topics->size(); i++)
    {
        Log::log("Unsuccessful unsubscription of topic: " + (*topics)[i]);
        emit unsubscription_failure(QString::fromStdString((*topics)[i]));
    }
}

void SubscriptionController::register_client(Client *client)
{
    _client = client;
}

void SubscriptionController::subscribe(const std::string &topic, int QOS)
{
    _client->subscribe(topic, QOS);
}

void SubscriptionController::unsubscribe(const std::string &topic)
{
    _client->unsubscribe(topic);
}

#include "message_controller.h"
#include <iostream>

MessageController::MessageController(TreeModel *tree_model)
                  : _tree_model(tree_model) {}

void MessageController::on_message_arrived_cb(void *object, const std::string &topic, const MessageData &message, FileType type)
{
    MessageController *message_controller = static_cast<MessageController *>(object);
    message_controller->on_message_arrived(topic, message, type);
}

void MessageController::on_publish_success_cb(void *object, const mqtt::token &token)
{
    MessageController *message_controller = static_cast<MessageController *>(object);
    message_controller->on_publish_success(token);
}

void MessageController::on_publish_failure_cb(void *object, const mqtt::token &token)
{
    MessageController *message_controller = static_cast<MessageController *>(object);
    message_controller->on_publish_failure(token);
}

void MessageController::on_delivery_complete_cb(void *object, mqtt::delivery_token_ptr token)
{
    MessageController *message_controller = static_cast<MessageController *>(object);
    message_controller->on_delivery_complete(token);
}

void MessageController::on_message_arrived(const std::string &topic, const MessageData &message, FileType type)
{
    Log::log("Message arrived on topic: " + topic);
    QVariant variant;
    TreeItem *topic_item = get_topic(topic);
    QJsonDocument json_document;
    bool our_message = _message_map.count(hash_function(message.binary.data, message.binary.size));

    switch (type)
    {
        case FileType::STRING_UTF8:
            variant = QVariant(QByteArray(message.binary.data, message.binary.size));
            break;

        case FileType::JSON:
            if (parse_json_message(message.binary, json_document))
            {
                variant = QVariant(json_document);
            }
            else
            {
                variant = QVariant(QByteArray(message.binary.data, message.binary.size));
            }
            break;

        case FileType::GIF:
        case FileType::JPG:
        case FileType::PNG:
            variant = QVariant(QByteArray(message.binary.data, message.binary.size));
            break;

        case FileType::BINARY:
            variant = QVariant(QByteArray(message.binary.data, message.binary.size));
            break;
        
        default:
            return;
    }

    Log::log("Adding message.");
    topic_item->addMessage(variant, static_cast<unsigned short>(type), our_message);
    
    Log::log("Updating model.");
    emit _tree_model->layoutChanged();
    emit message_arrived();
}

void MessageController::on_publish_success(const mqtt::token &token)
{
    // TODO register the ID and wait for delivery complete
    emit publish_success();
    Log::log("Message published successfuly.");
}

void MessageController::on_publish_failure(const mqtt::token &token)
{
    (void)token;

    //_main_widget_model->publish_failure(token.get_message_id());
    Log::log("Publishing failed.");
}

void MessageController::on_delivery_complete(mqtt::delivery_token_ptr token)
{
    if (token.get() == nullptr)
    {
        return;
    }
    
    //_main_widget_model->delivery_complete(token.get()->get_message_id());
}

void MessageController::register_client(Client *client)
{
    _client = client;
}

bool MessageController::parse_json_message(const Binary &binary_data, QJsonDocument &json_document)
{
    QJsonParseError parse_error;
    json_document = QJsonDocument::fromJson(QByteArray(binary_data.data, binary_data.size), &parse_error);

    Log::error(parse_error.errorString().toStdString());

    if (parse_error.error == QJsonParseError::NoError)
    {
        return true;
    }

    return false;
}

void MessageController::publish(const std::string &topic, const std::string &message)
{
    _message_map[hash_function(message.c_str(), message.size())] = true;
    _client->publish(topic, message);
}

//creates hierarchy of new topics that didn't exist before
TreeItem *MessageController::create_hierarchy(TreeItem& supertopic, std::vector<std::string> topics, 
                                              bool new_root_topic)
{
    TreeItem* supertop = &supertopic;
    for (std::string& topic : topics)
    {
        if (topic == topics.back())
        {
            supertop = &add_subtopic(*supertop, topic, {});
        } else {
            supertop = &add_subtopic(*supertop, topic, {});
        }
        if (new_root_topic) {
            this->_root_topics.push_back(supertop);

            new_root_topic = false;
        }
    }

    return supertop;
}

//adds topic to the model and returns pointer to the item of this topic
TreeItem& MessageController::add_subtopic(TreeItem& supertopic, std::string topic_name, QVariant data = {})
{
    auto child = new TreeItem({QString::fromStdString(topic_name), data}, &supertopic);
    supertopic.appendSubtopic(child);

    return *child;
}

std::vector<std::string> MessageController::parse_topic_path(std::string path)
{
    const char delimeter = '/';
    std::string buff{""};
    std::vector<std::string> topics;

    for(auto n: path)
    {
        if(n != delimeter) buff+=n; else
        if(n == delimeter && buff != "") { topics.push_back(buff); buff = "";}
    }
    if(buff != "") topics.push_back(buff);

    return topics;
}

TreeItem* MessageController::find_topic(std::string name, const QVector<TreeItem*>& topics)
{
    for (auto t : topics)
    {
        if (t->data(0).toString() == QString::fromStdString(name))
        {
            return t;
        }
    }
    return nullptr;
}

TreeItem *MessageController::get_topic(std::string topic_path)
{
    std::vector<std::string> topics = parse_topic_path(topic_path);
    TreeItem* supertopic = find_topic(topics[0], _root_topics);
    TreeItem* found_topic;

    //the first topic of the topic path was used before
    if (supertopic != nullptr)
    {
        //add message to existing first level topic
        if (topics.size() == 1)
        {
            return find_topic(topics[0], _root_topics);
        }
        topics.erase(topics.begin());

        while (topics.size())
        {
            auto subtopic = find_topic(topics[0], supertopic->getSubtopics());
            //if subtopic was already used, eliminate duplication
            if (subtopic != nullptr)
            {
                if (topics.size() == 1)
                {
                    found_topic = subtopic;
                }
                topics.erase(topics.begin());
                supertopic = subtopic;
                continue;
            } else {
                return create_hierarchy(*supertopic, topics, false);
                break;
            }
        }
    } else
    {
        return create_hierarchy(_tree_model->getRoot(), topics, true);
    }

    return found_topic;
}

void MessageController::set_message(QVariant content)
{
    this->_file_to_publish = content;
}

QVariant& MessageController::get_message()
{
    return _file_to_publish;
}

void MessageController::set_file_chosen()
{
    this->_file_chosen = true;
}
void MessageController::set_file_not_chosen()
{
    this->_file_chosen = false;
}

QString MessageController::validate_topic_path(QString path)
{
    std::string tmp = path.toStdString();

    if (tmp.back() == '/')
    {
        tmp.erase(tmp.size() - 1);
        return QString::fromStdString(tmp);
    }
    return QString::fromStdString(tmp);
}

bool MessageController::is_file_chosen()
{
    return _file_chosen;
}

size_t MessageController::hash_function(const char *data, size_t size)
{
    const unsigned char *p = (const unsigned char*)data;
    size_t h = 0;

    for (size_t i = 0; i < size; i++)
    {
        h = (h << 16) + (h << 6) + p[i] - h;
    }

    return h;
}

void MessageController::create_dir_structure(QDir parent_dir, QVector<TreeItem *>& subtopics)
{
    for (int i = 0; i < subtopics.size(); i++)
    {
        if (!parent_dir.mkdir(subtopics[i]->getName()))
        {
            parent_dir.rmdir(subtopics[i]->getName());
            parent_dir.mkdir(subtopics[i]->getName());
        }
        auto tmp_dir = QDir(parent_dir.absoluteFilePath(subtopics[i]->getName()));
        auto sub_topics = subtopics[i]->getSubtopics();
        auto msgs = subtopics[i]->getMessages();

        int j = 0;
        for (auto& [msg, type, our] : msgs)
        {
            QFile* f;
            if (type == "image")
            {
                f = new QFile(tmp_dir.path() + "/" + type + QString::number(j) + ".png");
            }
            else if (type == "binary" || type == "text")
            {
                f = new QFile(tmp_dir.path() + "/" + type + QString::number(j) + ".txt");
            }
            else if (type == "json")
            {
                f = new QFile(tmp_dir.path() + "/" + type + QString::number(j) + ".json");
            }
            else {
                continue;
            }

            f->open(QIODevice::WriteOnly);
            f->write(msg.toByteArray());
            f->close();
            j++;

            delete f;
        }
        create_dir_structure(tmp_dir, sub_topics);
    }
}

QVector<TreeItem *>& MessageController::get_root_topics()
{
    return _root_topics;
}
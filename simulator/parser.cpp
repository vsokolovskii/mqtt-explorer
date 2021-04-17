
#include "parser.h"

Parser::Parser()
{
    _reader = Json::CharReaderBuilder().newCharReader();
}

Parser::~Parser()
{
    delete _reader;
}

bool Parser::read_file_content(std::string file_name, std::string &content)
{
    std::ifstream file(file_name, std::ifstream::in);
    if (file.fail())
    {
        std::cerr << "Opening configuration file failed." << std::endl;
        return true;
    }

    std::ostringstream stream;
    stream << file.rdbuf();
    content = stream.str();

    file.close();
    return false;
}

bool Parser::parse_file(std::string file_name, Devices &devices)
{
    if (_reader == nullptr)
    {
        return true;
    }

    std::string content;

    if (read_file_content(file_name, content))
    {
        return true;
    }

    std::string errs;
    Json::Value root;

    if (!_reader->parse(content.c_str(), content.c_str() + content.size(), &root, &errs))
    {
        std::cerr << errs << std::endl;
        return true;
    }

    parse_thermometers(root["thermometers"], devices.thermometers);
    parse_hygrometers(root["hygrometers"], devices.hygrometers);
    parse_wattmeters(root["wattmeters"], devices.wattmeters);
    parse_move_sensors(root["move sensors"], devices.move_sensors);
    parse_lights(root["lights"], devices.lights);
    parse_cameras(root["cameras"], devices.cameras);
    parse_relays(root["relays"], devices.relays);
    parse_valves(root["valves"], devices.valves);

    return false;
}

void Parser::parse_thermometers(Json::Value &root, std::vector<Thermometer> &thermometers)
{
    thermometers.reserve(root.size());
    for (unsigned i = 0; root[i]; i++)
    {
        thermometers.push_back(Thermometer(root[i]["topic"].asString(),
                                           root[i]["name"].asString(), 
                                           root[i]["period"].asInt(),
                                           root[i]["min_temp"].asFloat(), 
                                           root[i]["max_temp"].asFloat(), 
                                           root[i]["min_step"].asFloat(), 
                                           root[i]["max_step"].asFloat(),
                                           root[i]["temp"].asFloat(),
                                           root[i]["unit"].asString()
                                           ));
    }
}

void Parser::parse_hygrometers(Json::Value &root, std::vector<Hygrometer> &hygrometers)
{
    hygrometers.reserve(root.size());
    for (unsigned i = 0; root[i]; i++)
    {
        hygrometers.push_back(Hygrometer(root[i]["topic"].asString(),
                                         root[i]["name"].asString(), 
                                         root[i]["period"].asInt(),
                                         root[i]["min_step"].asFloat(), 
                                         root[i]["max_step"].asFloat(),
                                         root[i]["humidity"].asFloat()
                                         ));
    }
}


void Parser::parse_wattmeters(Json::Value &root, std::vector<Wattmeter> &wattmeters)
{
    wattmeters.reserve(root.size());
    for (unsigned i = 0; root[i]; i++)
    {
        wattmeters.push_back(Wattmeter(root[i]["topic"].asString(),
                                         root[i]["name"].asString(), 
                                         root[i]["period"].asInt(),
                                         root[i]["min_val"].asInt(),
                                         root[i]["max_val"].asInt(),
                                         root[i]["min_step"].asInt(),
                                         root[i]["max_step"].asInt(),
                                         root[i]["value"].asInt(),
                                         root[i]["unit"].asString()
                                         ));
    }
}

void Parser::parse_move_sensors(Json::Value &root, std::vector<MoveSensor> &move_sensors)
{
    move_sensors.reserve(root.size());
    for (unsigned i = 0; root[i]; i++)
    {
        move_sensors.push_back(MoveSensor(root[i]["topic"].asString(),
                                          root[i]["name"].asString(), 
                                          root[i]["min_period"].asInt(),
                                          root[i]["max_period"].asInt(),
                                          root[i]["horizontal_FOV"].asInt(),
                                          root[i]["vertical_FOV"].asInt(),
                                          root[i]["type"].asString()
                                         ));
    }
}

void Parser::parse_lights(Json::Value &root, std::vector<Light> &lights)
{
    lights.reserve(root.size());
    for (unsigned i = 0; root[i]; i++)
    {
        lights.push_back(Light(root[i]["topic"].asString(),
                               root[i]["name"].asString(), 
                               root[i]["period"].asInt(),
                               root[i]["id"].asString(),
                               root[i]["recieving topic"].asString()
                               ));
        
        if (!root[i]["states"])
        {
            continue;
        }

        for (unsigned j = 0; root[i]["states"][j]; j++)
        {
            lights[i].add_state(root[i]["states"][j].asString());
        }
    }
}

void Parser::parse_cameras(Json::Value &root, std::vector<Camera> &cameras)
{
    cameras.reserve(root.size());
    for (unsigned i = 0; root[i]; i++)
    {
        cameras.push_back(Camera(root[i]["topic"].asString(),
                                 root[i]["name"].asString(), 
                                 root[i]["min_period"].asInt(),
                                 root[i]["max_period"].asInt()
                                 ));
        
        if (!root[i]["images"])
        {
            continue;
        }

        for (unsigned j = 0; root[i]["images"][j]; j++)
        {
            cameras[i].add_image(root[i]["images"][j].asString());
        }
    }
}

void Parser::parse_relays(Json::Value &root, std::vector<Relay> &relays)
{
    relays.reserve(root.size());
    for (unsigned i = 0; root[i]; i++)
    {
        relays.push_back(Relay(root[i]["answering topic"].asString(),
                               root[i]["name"].asString(), 
                               root[i]["id"].asString(),
                               root[i]["recieving topic"].asString()
                               ));
        
        if (!root[i]["states"])
        {
            continue;
        }

        for (unsigned j = 0; root[i]["states"][j]; j++)
        {
            relays[i].add_state(root[i]["states"][j].asString());
        }
    }
}

void Parser::parse_valves(Json::Value &root, std::vector<Valve> &valves)
{
    valves.reserve(root.size());
    for (unsigned i = 0; root[i]; i++)
    {
        valves.push_back(Valve(root[i]["topic"].asString(),
                               root[i]["name"].asString(), 
                               root[i]["period"].asInt(), 
                               root[i]["id"].asString(),
                               root[i]["recieving topic"].asString()
                               ));
        
        if (!root[i]["states"])
        {
            continue;
        }

        for (unsigned j = 0; root[i]["states"][j]; j++)
        {
            valves[i].add_state(root[i]["states"][j].asString());
        }
    }
}

bool Parser::parse_message(mqtt::const_message_ptr recieved_message, Message &parsed_message)
{
    const mqtt::message *message = recieved_message.get();
    if (message == nullptr)
    {
        return true;
    }

    std::string content = message->get_payload_str();
    parsed_message.topic = message->get_topic();

    std::string errs;
    Json::Value root;
    if (!_reader->parse(content.c_str(), content.c_str() + content.size(), &root, &errs))
    {
        //std::cerr << "Message parsing error: " << errs << std::endl;
        return true;
    }

    if (!root["id"] | !root["state"])
    {
        return true;
    }

    parsed_message.id = root["id"].asString();
    parsed_message.state = root["state"].asString();

    return false;
}

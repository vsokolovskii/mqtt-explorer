
/**
 * @file        dashboard_controller.cpp
 * Case:        VUT, FIT, ICP, project                                      <br>
 * Authors:     Vladislav Sokolovskii, xsokol15@stud.fit.vutbr.cz           <br>
 * Date:        summer semester 2021                                        <br>
 * Compiled:    g++ (Ubuntu 9.3.0-17ubuntu1~20.04) 9.3.0                    <br>
 * @brief       Implementation of constructors, destructors and functions of the DasboardController class.
 **/

#include "dashboard_controller.h"
#include "dashboard_view.h"

DashboardController::DashboardController(QObject *parent)
                    : QObject(parent)
{ }

DashboardController::~DashboardController()
{
    std::map<std::string, DeviceWidget*>::iterator iterator = topic_to_device.begin();
    while (iterator != topic_to_device.end())
    {
        delete iterator->second;
        iterator++;
    }
}

void DashboardController::add_device(DeviceWidget* device)
{
    topic_to_device[device->get_topic()] = device;
    emit new_device_on_topic(device->get_topic());
}

void DashboardController::process_message(std::string topic, TreeItem &topic_item)
{
    QPalette palette;
    QVariant payload = std::get<0>(topic_item.getMessages().back());

    if (topic_to_device.find(topic) == topic_to_device.end())
    {
        return;
    }
    else
    {
        auto device = topic_to_device.at(topic);
        auto type = device->get_type();

        if (type == DeviceType::CAM)
        {
            QImage i;
            i.loadFromData(payload.toByteArray());
            device->set_image(i);
        } else if (type == DeviceType::LIGHT)
        {
            QString state(payload.toString());
            if (state == "on")
            {
                device->set_image(QImage(":/images/bulb_on.png"));
            } 
            else if (state == "red")
            {
                palette.setColor(QPalette::Window, Qt::red);
                device->set_color(palette);
            } 
            else if (state == "green")
            {
                palette.setColor(QPalette::Window, Qt::green);
                device->set_color(palette);
            } 
            else if (state == "blue")
            {
                palette.setColor(QPalette::Window, Qt::blue);
                device->set_color(palette);
            } 
            else if (state == "yellow")
            {
                palette.setColor(QPalette::Window, Qt::yellow);
                device->set_color(palette);
            } 
            else if (state == "cyan")
            {
                palette.setColor(QPalette::Window, Qt::cyan);
                device->set_color(palette);
            } 
            else if (state == "magneta")
            {
                palette.setColor(QPalette::Window, Qt::magenta);
                device->set_color(palette);
            } 
            else if (state == "white")
            {
                palette.setColor(QPalette::Window, Qt::white);
                device->set_color(palette);
            } 
            else if (state == "orange")
            {
                palette.setColor(QPalette::Window, QColor::fromRgb(255, 140, 0));
                device->set_color(palette);
            } 
            else if (state == "violet")
            {
                palette.setColor(QPalette::Window, QColor::fromRgb(148, 0, 211));
                device->set_color(palette);
            } 
            else
            {
                device->set_image((QImage(":/images/bulb_off.png")));
            }
        } 
        else if (type == DeviceType::THERMOMETR || type == DeviceType::HYGR || type == DeviceType::WATT)
        {
            device->set_chart(topic_item);
            device->set_description(QString(payload.toString()));
        }
        else 
        {
            device->set_description(QString(payload.toString()));
        }
        Log::log(topic);
    }
}

/*void DashboardController::process_message(std::string topic, QByteArray payload)
{
    QPalette palette;

    if (topic_to_device.find(topic) == topic_to_device.end())
    {
        return;
    }
    else
    {
        auto device = topic_to_device.at(topic);
        auto type = device->get_type();

        if (type == DeviceType::CAM)
        {
            QImage i;
            i.loadFromData(payload);
            device->set_image(i);
        } else if (type == DeviceType::LIGHT)
        {
            QString state(payload);
            if (state == "on")
            {
                device->set_image(QImage(":/images/bulb_on.png"));
            } 
            else if (state == "red")
            {
                palette.setColor(QPalette::Window, Qt::red);
                device->set_color(palette);
            } else if (state == "green")
            {
                palette.setColor(QPalette::Window, Qt::green);
                device->set_color(palette);
            } else if (state == "blue")
            {
                palette.setColor(QPalette::Window, Qt::blue);
                device->set_color(palette);
            } else if (state == "yellow")
            {
                palette.setColor(QPalette::Window, Qt::yellow);
                device->set_color(palette);
            } else if (state == "cyan")
            {
                palette.setColor(QPalette::Window, Qt::cyan);
                device->set_color(palette);
            } else if (state == "magneta")
            {
                palette.setColor(QPalette::Window, Qt::magenta);
                device->set_color(palette);
            } else if (state == "white")
            {
                palette.setColor(QPalette::Window, Qt::white);
                device->set_color(palette);
            } else if (state == "orange")
            {
                palette.setColor(QPalette::Window, QColor::fromRgb(255, 140, 0));
                device->set_color(palette);
            } else if (state == "violet")
            {
                palette.setColor(QPalette::Window, QColor::fromRgb(148, 0, 211));
                device->set_color(palette);
            } 
            else
            {
                device->set_image((QImage(":/images/bulb_off.png")));
            }
            //device->set_description(state);
        } else if (type == DeviceType::THERMOMETR)
        {
            device->set_description(QString(payload));
        } else {
            device->set_description(QString(payload));
        }
        Log::log(topic);
    }
}*/

void DashboardController::register_dashboard_view(DashboardView *dashboard_window)
{
    _dashboard_window = dashboard_window;
}

void DashboardController::show_dashboard()
{
    _dashboard_window->show();
}

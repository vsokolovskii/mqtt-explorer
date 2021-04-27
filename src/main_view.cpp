#include "main_view.h"
#include "ui_mainwindow.h"

MainView::MainView(TreeModel *tree_model, MainWidgetModel *main_widget_model, ConnectionController *connection_controller,
                       MessageController *message_controller, SubscriptionController *subscription_controller) 
           : QMainWindow(nullptr), _ui(new Ui::MainWindow), _tree_model(tree_model), _main_widget_model(main_widget_model),
             _connection_controller(connection_controller), _message_controller(message_controller), 
             _subscription_controller(subscription_controller)
{
    Log::log("Main window initialization starting...");
    
    _tree_model->setParent(this);
    _main_widget_model->setParent(this);

    qRegisterMetaType<QList<QPersistentModelIndex>>("QList<QPersistentModelIndex>");
    qRegisterMetaType<QAbstractItemModel::LayoutChangeHint>("QAbstractItemModel::LayoutChangeHint");

    _ui->setupUi(this);
    _ui->messageList->setModel(_tree_model);
    connect(_ui->messageList->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainView::item_selection);

    Log::log("Main window initialization complete.");
}

MainView::~MainView()
{
    delete _ui;
}

void MainView::display()
{
    this->show();
    _ui->messageList->setColumnWidth(0, _ui->messageList->size().width() * 0.6);
    _ui->img_label->setVisible(false);
    _ui->listWidget->setWrapping(false);
    _ui->clear->setVisible(false);
}

void MainView::item_selection()
{
    _ui->listWidget->clear();
    const bool hasCurrent = _ui->messageList->selectionModel()->currentIndex().isValid();

    if (hasCurrent)
    {
        auto qindex = _ui->messageList->selectionModel()->currentIndex();
        auto item = _tree_model->getItem(qindex);
        auto history = item->getMessages();
        QStringList messages;

        for (std::tuple<QVariant, QString> &tuple : history)
        {
            auto tmp = std::get<1>(tuple) + ": " + std::get<0>(tuple).toString();
            messages << tmp;
        }
        _ui->listWidget->addItems(messages);
        _ui->listWidget->update();
        _ui->path->setText(_tree_model->getPath(*item));
    }
}

void MainView::on_publish_clicked()
{
    QString topic = _ui->path->text();
    QVariant qv = _ui->msg_to_publish->toPlainText();

    _message_controller->publish(topic.toStdString(), _ui->msg_to_publish->toPlainText().toStdString());
}

void MainView::on_subscribe_clicked()
{
    std::string topic = _ui->topic->text().toStdString();
    Log::log("Subscribing to topic: " + topic);
    _subscription_controller->subscribe(topic, 1);
}


void MainView::on_unsubscribe_clicked()
{
    std::string topic = _ui->topic->text().toStdString();
    Log::log("Unsubscribing to topic: " + topic);
    _subscription_controller->unsubscribe(topic);
}

void MainView::on_disconnect_clicked()
{
    _connection_controller->disconnect();
}

void MainView::on_reconnect_clicked()
{
    _connection_controller->reconnect();
}

void MainView::on_chooseFile_clicked()
{
    QString file_name = QFileDialog::getOpenFileName(this, "Choose the message content", QDir::homePath(), tr("Messages (*.png *.xml *.jpg *.json *.txt)"));
    QFile file(file_name);
    QFileInfo info_file(file_name);
    QByteArray msg;

    QString type = info_file.completeSuffix();

    if (type == "jpg" || type == "png")
    {
        //binary
        QImage img;
        QImageReader reader(file_name);

        if(reader.read(&img))
        {
            // display chosen image
            int w = _ui->img_label->width();
            int h = _ui->img_label->height();
            QPixmap image = QPixmap::fromImage(img);

            _ui->img_label->setPixmap(image.scaled(w, h, Qt::KeepAspectRatio));
            _ui->img_label->setVisible(true);
            _ui->msg_to_publish->setVisible(false);
            //----------------------
            QBuffer buffer(&msg);
            buffer.open(QIODevice::WriteOnly);
            img.save(&buffer);

            //message_controller->set_message(QVariant(msg), FileType::ALL_IMAGES);
            //message_controller->set_file_chosen();
            _ui->clear->setVisible(true);
        }
    }
    else if (type == "json" || type == "xml")
    {
        //markup
    }
    else
    {
        //plain text
        if (!file.open(QFile::ReadOnly | QFile::Text))
        {

        }
        //QTextStream in(file);
        QStringList message;
        //message << in.readAll();

        _ui->msg_to_publish->setPlainText(message.join("\n"));
    }
}


void MainView::on_clear_clicked()
{
    //message_controller->set_message({}, FileType::ALL);
//    message_controller->set_file_not_chosen();

    _ui->img_label->setVisible(false);
    _ui->msg_to_publish->setPlainText("");
    _ui->msg_to_publish->setVisible(true);
}

void MainView::on_exit_clicked()
{
    _connection_controller->go_to_login_view();
}

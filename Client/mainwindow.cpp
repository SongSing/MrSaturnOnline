#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QColorDialog>
#include <QTextBrowser>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    QCoreApplication::setOrganizationName("Team Zesty");
    QCoreApplication::setApplicationName("Mr. Saturn Online Client");
    QApplication::setStyle("Fusion");

    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);
    m_color = "#FF0000"; // do settings with this later
    ui->color->setStyleSheet("background-color:" + m_color);

    connect(ui->color, SIGNAL(clicked()), this, SLOT(chooseColor()));
    connect(ui->connect, SIGNAL(clicked()), this, SLOT(connectToServer()));
    connect(ui->send, SIGNAL(clicked()), this, SLOT(sendMessage()));
    connect(ui->input, SIGNAL(returnPressed()), this, SLOT(sendMessage()));
    connect(ui->channels, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(joinChannel(QListWidgetItem*)));
    connect(ui->chats, SIGNAL(currentChanged(int)), this, SLOT(currentChanged(int)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

Channel MainWindow::currentChannel()
{
    return m_channels[m_currentChannelId];
}

bool MainWindow::hasChannel(Channel c)
{
    return m_channels.contains(c);
}

void MainWindow::chooseColor()
{
    QColor color = QColorDialog::getColor(QColor(m_color), this, "Choose name color...");

    if (color.isValid())
    {
        m_color = color.name();
        ui->color->setStyleSheet("background-color:" + m_color);
    }
}

void MainWindow::connectToServer()
{
    if (ui->name->text().isEmpty() || ui->host->text().isEmpty())
    {
        // possible warning
        return;
    }

    m_name = ui->name->text();
    m_port = ui->port->value();
    m_host = ui->host->text();

    //ui->chat->clear();
    ui->stackedWidget->setCurrentIndex(1);

    m_socket = new QTcpSocket(this);

    connect(m_socket, SIGNAL(connected()), this, SLOT(connected()));
    connect(m_socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(readyRead()));

    //ui->chat->append("<i>Connecting to " + m_host + " on port " + QString::number(m_port) + "...");
    m_socket->connectToHost(m_host, m_port);
}

void MainWindow::disconnectFromServer()
{
    m_socket->disconnectFromHost();
    ui->stackedWidget->setCurrentIndex(0);
    //ui->chat->clear();
}

void MainWindow::connected()
{
    //ui->chat->append("<b>Connected to Server!</b>");
    qDebug() << "Connected to server";

    Packet p;
    p.begin(Enums::JoinCommand);
    p.write(m_name, Enums::NameLength);
    p.write(m_color, Enums::ColorLength);
    p.end();

    sendPacket(p);
}

void MainWindow::disconnected()
{
    //ui->chat->append("<b>Disconnected from Server!</b>");
}

void MainWindow::readyRead()
{
    while (m_socket->canReadLine())
    {
        Packet p(m_socket->readLine());
        Enums::Command command = (Enums::Command)p.readCommand();

        if (command == Enums::MessageCommand)
        {
            // expecting name, color, channelId, message
            QString name, color, message;
            int channelId;

            name = p.readString(Enums::NameLength);
            color = p.readString((Enums::ColorLength));
            channelId = p.readInt(Enums::ChannelIdLength);
            message = p.readString(Enums::MessageLength);

            if (this->hasChannel(channelFromId(channelId)))
                appendChannel(channelId, tr("<font color='%1'>%2 <b>%3:</b></font> %4").arg(color, timestamp(), name, message));
        }
        else if (command == Enums::JoinChannelCommand)
        {
            // expected channel id
            int channelId = p.readInt(Enums::ChannelIdLength);
            Channel c = channelFromId(channelId);

            if (!this->hasChannel(c))
            {
                m_channels.append(c);

                QTextBrowser *t = new QTextBrowser();
                t->setProperty("id", channelId);
                ui->chats->addTab(t, c.name());

                m_channelMap.insert(channelId, t);
            }
        }
        else if (command == Enums::ChannelListCommand)
        {
            // expected ids, names

            QList<int> ids = p.readIntList(Enums::ChannelIdListLength, Enums::ChannelIdLength);
            QStringList names = p.readStringList(Enums::ChannelNameListLength, Enums::ChannelNameLength);

            all_channels.clear();
            ui->channels->clear();

            for (int i = 0; i < ids.length(); i++)
            {
                all_channels << Channel(ids[i], names[i]);
                ui->channels->addItem(names[i]);
            }
        }
        else if (command == Enums::UserListCommand)
        {
            // expected channelId, list of ids, list of names, list of colors

            int channelId = p.readInt(Enums::ChannelIdLength);
            Channel c = channelFromId(channelId);

            if (this->hasChannel(c))
            {
                QList<int> ids = p.readIntList(Enums::IdListLength, Enums::IdLength);
                QStringList names = p.readStringList(Enums::NameListLength, Enums::NameLength);
                QStringList colors = p.readStringList(Enums::ColorListLength, Enums::ColorLength);

                if (m_userMap.contains(channelId))
                {
                    m_userMap.remove(channelId);
                }

                m_userMap[channelId] = QList<User>();

                for (int i = 0; i < ids.length(); i++)
                {
                    m_userMap[channelId].append(User(ids[i], names[i], colors[i]));
                }

                if (m_currentChannelId == channelId)
                {
                    setCurrentChannel(c);
                }
            }
        }
        else if (command == Enums::UserJoinedChannelCommand)
        {
            // expected channelId, id, name, color

            int channelId = p.readInt(Enums::ChannelIdLength);
            Channel c = channelFromId(channelId);

            if (this->hasChannel(c))
            {
                int id = p.readInt(Enums::IdLength);
                QString name = p.readString(Enums::NameLength);
                QString color = p.readString(Enums::ColorLength);
                m_userMap[channelId].append(User(id, name, color));

                if (channelId == m_currentChannelId)
                {
                    QListWidgetItem *item = new QListWidgetItem(name);
                    item->setForeground(QBrush(QColor(color)));

                    ui->users->addItem(item);
                }

                appendChannel(channelId, tr("<i>%1 <font color='%2'><b>%3</b></font> joined!</i>").arg(timestamp(), color, name));
            }
        }
        else if (command == Enums::UserLeftChannelCommand)
        {
            // expected channelId, id, name, color

            int channelId = p.readInt(Enums::ChannelIdLength);
            Channel c = channelFromId(channelId);

            if (this->hasChannel(c))
            {
                int id = p.readInt(Enums::IdLength);
                QString name = p.readString(Enums::NameLength);
                QString color = p.readString(Enums::ColorLength);
                m_userMap[channelId].removeAll(User(id, name, color));

                if (channelId == m_currentChannelId)
                    ui->users->removeItemWidget(ui->users->findItems(name, Qt::MatchFixedString)[0]);

                appendChannel(channelId, tr("<i>%1 <font color='%2'><b>%3</b></font> joined!</i>").arg(timestamp(), color, name));
            }
        }
    }
}

void MainWindow::sendMessage()
{
    QString message = ui->input->text();

    if (!message.isEmpty())
    {
        ui->input->clear();

        Packet p;
        p.begin(Enums::MessageCommand);
        p.write(m_currentChannelId, Enums::ChannelIdLength);
        p.write(message, Enums::MessageLength);
        p.end();

        sendPacket(p);
    }
}

void MainWindow::sendPacket(Packet p)
{
    m_socket->write(p.toByteArray()); // use toByteArray instead of other things
}

void MainWindow::setCurrentChannel(Channel c)
{
    if (this->hasChannel(c))
    {
        m_currentChannelId = c.id();

        ui->chats->setCurrentWidget(m_channelMap[c.id()]);

        ui->users->clear();
        QList<User> users = m_userMap[c.id()];

        for (int i = 0; i < users.length(); i++)
        {
            QListWidgetItem *item = new QListWidgetItem(users[i].name());
            item->setForeground(QBrush(QColor(users[i].color())));

            ui->users->addItem(item);
        }
    }
}

void MainWindow::joinChannel(QListWidgetItem *item)
{
    joinChannel(channelFromName(item->text()));
}

void MainWindow::joinChannel(Channel c)
{
    if (!this->hasChannel(c))
    {
        Packet p;
        p.begin(Enums::JoinChannelCommand);
        p.write(c.id(), Enums::ChannelIdLength);
        p.end();

        sendPacket(p);
    }
    else
    {
        setCurrentChannel(c);
    }
}

void MainWindow::appendChat(const QString &text)
{
    if (ui->chats->currentIndex() >= 0)
    {
        ((QTextBrowser*)ui->chats->currentWidget())->append(text);
    }
}

void MainWindow::appendChannel(int channelId, const QString &text)
{
    m_channelMap[channelId]->append(text);
}

void MainWindow::currentChanged(int ind)
{
    int id = ui->chats->widget(ind)->property("id").toInt();

    this->setCurrentChannel(channelFromId(id));
}

Channel MainWindow::channelFromId(int id)
{
    foreach (Channel channel, all_channels)
    {
        if (channel.id() == id)
        {
            return channel;
        }
    }

    return Channel::mysteryZone();
}

Channel MainWindow::channelFromName(const QString &name)
{
    foreach (Channel channel, all_channels)
    {
        if (channel.name() == name)
        {
            return channel;
        }
    }

    return Channel::mysteryZone();
}

QString MainWindow::timestamp()
{
    QDateTime now = QDateTime::currentDateTime();

    return "(" + now.toString("HH:mm:ss") + ")";
}























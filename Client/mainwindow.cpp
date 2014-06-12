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

    all = Channel::all();
    mysteryZone = Channel::mysteryZone();

    QSettings s;

    m_color = s.value("color", "#0000FF").toString();
    ui->color->setStyleSheet("background-color:" + m_color);

    connect(ui->color, SIGNAL(clicked()), this, SLOT(chooseColor()));
    connect(ui->connect, SIGNAL(clicked()), this, SLOT(connectToServer()));
    connect(ui->send, SIGNAL(clicked()), this, SLOT(sendMessage()));
    connect(ui->input, SIGNAL(returnPressed()), this, SLOT(sendMessage()));
    connect(ui->channels, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(joinChannel(QListWidgetItem*)));
    connect(ui->chats, SIGNAL(currentChanged(int)), this, SLOT(currentChanged(int)));
    connect(ui->actionDisconnect_from_Server, SIGNAL(triggered()), this, SLOT(disconnectFromServer()));
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->chats, SIGNAL(tabCloseRequested(int)), this, SLOT(leaveChannel(int)));
    connect(ui->joinChannel, SIGNAL(clicked()), this, SLOT(createChannel()));
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

    initialChat = new QTextBrowser();
    ui->chats->addTab(initialChat, m_host);

    ui->stackedWidget->setCurrentIndex(1);

    m_socket = new QWebSocket();

    connect(m_socket, SIGNAL(connected()), this, SLOT(connected()));
    connect(m_socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(m_socket, SIGNAL(textMessageReceived(QString)), this, SLOT(readyRead(QString)));

    appendChat("<i>Connecting to " + m_host + " on port " + QString::number(m_port) + "...");
    m_socket->open("ws://" + m_host + ":" + QString::number(m_port));
}

void MainWindow::disconnectFromServer()
{
    m_socket->close();
    ui->stackedWidget->setCurrentIndex(0);

    for (int i = 0; i < ui->chats->count(); i++)
    {
        delete ui->chats->widget(i);
    }

    ui->chats->clear();
    m_channels.clear();
    all_channels.clear();
    m_currentChannelId = -1;
    m_userMap.clear();
    m_channelMap.clear();
}

void MainWindow::connected()
{
    appendChat("<b>Connected to Server!</b>");

    Packet p;
    p.begin(Enums::JoinCommand);
    p.write(m_name, Enums::NameLength);
    p.write(m_color, Enums::ColorLength);
    p.write(0, Enums::SpriteLength);
    p.end();

    sendPacket(p);
}

void MainWindow::disconnected()
{
    appendChat("<b>Disconnected from Server!</b>");
}

void MainWindow::readyRead(const QString &message)
{
    Packet p(message);
    Enums::Command command = (Enums::Command)p.readCommand();

    if (command == Enums::MessageCommand)
    {
        handleMessage(p);
    }
    else if (command == Enums::ImageCommand)
    {
        handleImage(p);
    }
    else if (command == Enums::JoinCommand)
    {
        handleJoin(p);
    }
    else if (command == Enums::JoinChannelCommand)
    {
        handleJoinChannel(p);
    }
    else if (command == Enums::ChannelListCommand)
    {
        handleChannelList(p);
    }
    else if (command == Enums::UserListCommand)
    {
        handleUserList(p);
    }
    else if (command == Enums::UserJoinedChannelCommand)
    {
        handleUserJoinedChannel(p);
    }
    else if (command == Enums::UserLeftChannelCommand)
    {
        handleUserLeftChannel(p);
    }
    else if (command == Enums::CreateChannelCommand)
    {
        handleAddChannel(p);
    }
    else if (command == Enums::RemoveChannelCommand)
    {
        handleRemoveChannel(p);
    }
    else if (command == Enums::SetChatImageCommand)
    {
        handleSetChatImage(p);
    }
}

void MainWindow::sendMessage()
{
    QString message = ui->input->text();

    if (!message.isEmpty())
    {
        ui->input->clear();

        QStringList lines = message.replace("\r", "").split("\n");

        foreach (QString line, lines)
        {
            Packet p;
            p.begin(Enums::MessageCommand);
            p.write(m_currentChannelId, Enums::ChannelIdLength);
            p.write(line, Enums::MessageLength);
            p.end();

            sendPacket(p);
        }
    }
}

void MainWindow::sendPacket(Packet p)
{
    m_socket->sendTextMessage(p); // haha we're texting the server
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

void MainWindow::leaveChannel(int ind)
{
    QWidget *w = ui->chats->widget(ind);
    int id = w->property("id").toInt();

    if (channelFromId(id) != mysteryZone)
    {
        m_channels.removeAll(channelFromId(id));
        Packet p;
        p.begin(Enums::LeaveChannelCommand);
        p.write(id, Enums::ChannelIdLength);
        p.end();

        sendPacket(p);

        ui->chats->removeTab(ind);
        w->deleteLater();
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
    if (ind >= 0)
    {
        int id = ui->chats->widget(ind)->property("id").toInt();

        this->setCurrentChannel(channelFromId(id));
    }
}

void MainWindow::createChannel()
{
    QString name = ui->channelName->text();

    if (!name.isEmpty())
    {
        ui->channelName->clear();

        if (channelFromName(name, false) != mysteryZone)
        {
            joinChannel(channelFromName(name, false));
        }
        else
        {
            Packet p;
            p.begin(Enums::CreateChannelCommand);
            p.write(name, Enums::ChannelNameLength);
            p.end();

            sendPacket(p);
        }
    }
}

void MainWindow::addChannel(int id, const QString &name)
{
    Channel c(id, name);

    if (!all_channels.contains(c))
    {
        all_channels.append(c);
        QListWidgetItem *item = new QListWidgetItem(name);
        m_channelListMap.insert(id, item);
        ui->channels->addItem(item);
    }
}

// ************************************************** // command handling // ************************************************** //

void MainWindow::handleMessage(Packet p)
{
    // expecting name, color, channelId, message
    QString name, color, message;
    int channelId;

    name = p.readString(Enums::NameLength);
    color = p.readString((Enums::ColorLength));
    channelId = p.readInt(Enums::ChannelIdLength);
    message = p.readString(Enums::MessageLength);

    if (channelId != all.id())
    {
        if (this->hasChannel(channelFromId(channelId)))
        {
            appendChannel(channelId, tr("<font color='%1'>%2 <b>%3:</b></font> %4")
                          .arg(color, timestamp(), name.toHtmlEscaped(), message.toHtmlEscaped()));
        }
    }
    else
    {
        foreach (Channel c, m_channels)
        {
            appendChannel(c.id(), tr("<font color='%1'>%2 <b>%3:</b></font> %4")
                          .arg(color, timestamp(), name.toHtmlEscaped(), message.toHtmlEscaped()));
        }
    }
}

void MainWindow::handleImage(Packet p)
{
    // expecting name, color, channelId, message
    QString name, color, message;
    int channelId;

    name = p.readString(Enums::NameLength);
    color = p.readString((Enums::ColorLength));
    channelId = p.readInt(Enums::ChannelIdLength);
    message = p.readString(Enums::ImageLength);

    if (this->hasChannel(channelFromId(channelId)))
    {
        appendChannel(channelId, tr("<font color='%1'>%2 <b>%3:</b></font> %4")
                      .arg(color, timestamp(), name.toHtmlEscaped(), message.toHtmlEscaped().replace("[", "<").replace("]", ">")));
    }
}

void MainWindow::handleJoin(Packet p)
{
    // expecting id, this is sent back as we join so we can grab our info that's generated by the server (not needed atm, tho)
    int id = p.readInt(Enums::IdLength);
    m_id = id;
}

void MainWindow::handleJoinChannel(Packet p)
{
    // expected channel id
    int channelId = p.readInt(Enums::ChannelIdLength);
    Channel c = channelFromId(channelId);

    if (!this->hasChannel(c))
    {
        bool first = m_channels.length() == 0;
        QString html;

        if (first)
        {
            html = initialChat->toHtml();
            ui->chats->clear();
            initialChat->deleteLater();
        }

        m_channels.append(c);

        QTextBrowser *t = new QTextBrowser();
        t->setProperty("id", channelId);
        ui->chats->addTab(t, c.name());

        m_channelMap.insert(channelId, t);

        if (first)
        {
            t->setHtml(html);
        }
    }
}

void MainWindow::handleChannelList(Packet p)
{
    // expected ids, names

    QList<int> ids = p.readIntList(Enums::ChannelIdListLength, Enums::ChannelIdLength);
    QStringList names = p.readStringList(Enums::ChannelNameListLength, Enums::ChannelNameLength);

    all_channels.clear();
    ui->channels->clear();

    for (int i = 0; i < ids.length(); i++)
    {
        addChannel(ids[i], names[i]);
    }
}

void MainWindow::handleAddChannel(Packet p)
{
    // expected id, name
    int id = p.readInt(Enums::ChannelIdLength);
    QString name = p.readString(Enums::ChannelNameLength);

    addChannel(id, name);
}

void MainWindow::handleRemoveChannel(Packet p)
{
    // expected id, name
    int id = p.readInt(Enums::ChannelIdLength);
    QString name = p.readString(Enums::ChannelNameLength);

    Channel c(id, name);

    if (all_channels.contains(c))
    {
        all_channels.removeAll(c);
        QListWidgetItem *item = m_channelListMap[id];
        m_channelListMap.remove(id);
        ui->channels->removeItemWidget(item);
        delete item;
    }
}

void MainWindow::handleUserList(Packet p)
{
    // expected channelId, list of ids, list of names, list of colors

    int channelId = p.readInt(Enums::ChannelIdLength);
    Channel c = channelFromId(channelId);

    if (this->hasChannel(c))
    {
        QList<int> ids = p.readIntList(Enums::IdListLength, Enums::IdLength);
        QStringList names = p.readStringList(Enums::NameListLength, Enums::NameLength);
        QStringList colors = p.readStringList(Enums::ColorListLength, Enums::ColorLength);
        QList<int> sprites = p.readIntList(Enums::SpriteListLength, Enums::SpriteLength);

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

void MainWindow::handleUserJoinedChannel(Packet p)
{
    // expected channelId, id, name, color

    int channelId = p.readInt(Enums::ChannelIdLength);
    Channel c = channelFromId(channelId);

    if (this->hasChannel(c))
    {
        int id = p.readInt(Enums::IdLength);
        QString name = p.readString(Enums::NameLength);
        QString color = p.readString(Enums::ColorLength);
        int sprite = p.readInt(Enums::SpriteLength);
        m_userMap[channelId].append(User(id, name, color));

        if (channelId == m_currentChannelId)
        {
            QListWidgetItem *item = new QListWidgetItem(name);
            item->setForeground(QBrush(QColor(color)));

            ui->users->addItem(item);
        }

        appendChannel(channelId, tr("<i>%1 <font color='%2'><b>%3</b></font> joined the channel!</i>")
                      .arg(timestamp(), color, name.toHtmlEscaped()));
    }
}

void MainWindow::handleUserLeftChannel(Packet p)
{
    // expected channelId, id, name, color

    int channelId = p.readInt(Enums::ChannelIdLength);
    Channel c = channelFromId(channelId);

    if (this->hasChannel(c))
    {
        int id = p.readInt(Enums::IdLength);
        QString name = p.readString(Enums::NameLength);
        QString color = p.readString(Enums::ColorLength);
        int sprite = p.readInt(Enums::SpriteLength);
        m_userMap[channelId].removeAll(User(id, name, color));

        if (channelId == m_currentChannelId)
        {
            QList<QListWidgetItem*> items = ui->users->findItems(name, Qt::MatchCaseSensitive);
            foreach (QListWidgetItem *item, items)
            {
                ui->users->removeItemWidget(item);
                delete item;
            }
        }

        appendChannel(channelId, tr("<i>%1 <font color='%2'><b>%3</b></font> left the channel!</i>")
                      .arg(timestamp(), color, name.toHtmlEscaped()));
    }
}

void MainWindow::handleSetChatImage(Packet p)
{
    QImage image = p.readImage(Enums::ChatImageLength);
    image.save(QDir::currentPath() + "/chat.png");
    this->setStyleSheet("QTextBrowser { background-image: url(\"" + QDir::currentPath() + "/chat.png\"); background-repeat: no-repeat; background-attachment: fixed; background-position: center; background-color: white; }");
}

// ************************************************** // end command handling // ************************************************** //

Channel MainWindow::channelFromId(int id)
{
    foreach (Channel channel, all_channels)
    {
        if (channel.id() == id)
        {
            return channel;
        }
    }

    return mysteryZone;
}

Channel MainWindow::channelFromName(const QString &name, bool caseSens)
{
    foreach (Channel channel, all_channels)
    {
        if ((caseSens && channel.name() == name) || (!caseSens && channel.name().toLower() == name.toLower()))
        {
            return channel;
        }
    }

    return mysteryZone;
}

QString MainWindow::timestamp()
{
    QDateTime now = QDateTime::currentDateTime();

    return "(" + now.toString("HH:mm:ss") + ")";
}






















// zoom!

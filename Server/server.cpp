#include "server.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QFileDialog>
#include <QSettings>
#include <QTimer>

Server::Server(const QString &name, SslMode mode, QObject *parent) :
    QWebSocketServer(name, mode, parent)
{
    m_floodPenalty = 50;
    m_floodLimit = 1000;

    QSettings s;
    m_bans = s.value("bans", QStringList()).toStringList();
    m_drawBoard = s.value("drawBoard", "").toString();

    connect(this, SIGNAL(newConnection()), this, SLOT(onNewConnection()));

    m_parent = (QWidget*)parent;
    all = Channel::all();
    mysteryZone = Channel::mysteryZone();

    m_welcomeMessage = "Hi ho! You come buying?";

    createChannel("Saturn Valley", true);
    createChannel("Saturn Hot Springs", true);

    QTimer *reqTimer = new QTimer();
    reqTimer->setInterval(1800000);

    connect(reqTimer, &QTimer::timeout, [=]()
    {
        if (m_clients.length() > 0)
        {
            Packet req;
            req.begin(Enums::RequestBoardCommand);
            req.write(-1, Enums::IdLength);
            req.end();

            sendOne(m_clients[0], req.toByteArray(), all);
        }
    });

    reqTimer->start(1800000);
}

void Server::sendAll(const QByteArray &data)
{
    foreach (Client *client, m_clients)
    {
        client->write(data);
    }
}

void Server::sendOne(Client *client, const QByteArray &data, Channel *channel)
{
    if (channel == all)
    {
        client->write(data);
    }
    else
    {
        channel->sendOne(client, data);
    }
}

void Server::sendChannel(Channel *channel, const QByteArray &data)
{
    if (channel == all)
    {
        sendAll(data);
    }
    else
    {
        channel->sendAll(data);
    }
}

void Server::setWelcomeMessage(const QString &message)
{
    m_welcomeMessage = message;
}

int Server::generateId()
{
    if (m_clientMap.isEmpty())
    {
        return 0;
    }
    else
    {
        int id = m_clientMap.size();

        for (int i = 0; i < m_clientMap.size(); i++)
        {
            if (!m_clientMap.contains(i))
            {
                id = i;
                break;
            }
        }

        return id;
    }
}

int Server::generateChannelId()
{
    if (m_channels.isEmpty())
    {
        return 0;
    }
    else
    {
        int id = m_channelIdMap.size();

        for (int i = 0; i < m_channelIdMap.size(); i++)
        {
            if (!m_channelIdMap.contains(i))
            {
                id = i;
                break;
            }
        }

        return id;
    }
}

Channel *Server::defaultChannel()
{
    if (m_channels.isEmpty())
    {
        return mysteryZone;
    }

    return m_channels[0];
}

Channel *Server::allChannels()
{
    return all;
}

void Server::readyRead(const QString &message)
{
    Client *client = (Client*)sender();

    Packet p(message);

    Enums::Command command = (Enums::Command)p.readCommand();

    if (command != Enums::DrawCommand && addFlood(client->ip()))
    {
        return;
    }

    if (command == Enums::MessageCommand)
    {
        handleMessage(p, client);
    }
    else if (command == Enums::ImageCommand)
    {
        handleImage(p, client);
    }
    else if (command == Enums::JoinCommand)
    {
        handleJoin(p, client);
    }
    else if (command == Enums::UnjoinCommand)
    {
        handleUnjoin(p, client);
    }
    else if (command == Enums::JoinChannelCommand)
    {
        handleJoinChannel(p, client);
    }
    else if (command == Enums::LeaveChannelCommand)
    {
        handleUnjoinChannel(p, client);
    }
    else if (command == Enums::CreateChannelCommand)
    {
        handleCreateChannel(p, client);
    }
    else if (command == Enums::RemoveChannelCommand)
    {
        handleRemoveChannel(p, client);
    }
    else if (command == Enums::DrawCommand)
    {
        handleDraw(p, client);
    }
    else if (command == Enums::ClearCommand)
    {
        handleClear(p, client);
    }
    else if (command == Enums::BoardDataCommand)
    {
        handleBoardData(p, client);
    }
    else if (command == Enums::PMCommand)
    {
        handlePM(p, client);
    }
}

void Server::clientDisconnected()
{
    Client *client = (Client*)sender();

    QString ip = client->ip();

    emit userRemoved(client);

    QString name = client->name();
    QString color = client->color();

    m_clients.removeAll(client);
    m_clientMap.remove(client->id());
    m_clientNameMap.remove(name);

    m_ipMap[ip].removeAll(client);

    foreach (Channel *channel, client->channels())
    {
        channel->removeClient(client);

        if (channel->isEmpty() && !channel->isPermanent())
        {
            removeChannel(channel);
        }
    }

    debug(tr("<i>%1 <font color='%2'><b>%3</b></font> left!</i>").arg(timestamp(), color, name.toHtmlEscaped()));

    client->socket()->close();
    client->socket()->deleteLater();
    client->deleteLater();

    /*Packet p;
    p.begin(Enums::UnjoinCommand);
    p.write(name, Enums::NameLength);
    p.write(color, Enums::ColorLength);
    p.end();

    sendAll(p.toByteArray());
    sendUserList();*/
}

void Server::sendChannelList()
{
    // sending ids, neames

    foreach (Client *client, m_clients)
    {
        client->sendChannels(m_channels);
    }
}

void Server::sendMessageToAll(const QString &message, Channel *channel, const QString &name, const QString &color)
{
    Packet toSend;
    toSend.begin(Enums::MessageCommand);
    toSend.write(name, Enums::NameLength);
    toSend.write(color, Enums::ColorLength);
    toSend.write(channel->id(), Enums::ChannelIdLength);
    toSend.write(message, Enums::MessageLength);
    toSend.end();

    sendChannel(channel, toSend.toByteArray());

    debug(tr("[%1] <font color='%2'>%3 <b>%4:</b></font> %5").arg(channel->name(), color, timestamp(), name, message));
}

void Server::sendMessageToOne(const QString &message, Client *client, Channel *channel, const QString &name, const QString &color)
{
    Packet toSend;
    toSend.begin(Enums::MessageCommand);
    toSend.write(name, Enums::NameLength);
    toSend.write(color, Enums::ColorLength);
    toSend.write(channel->id(), Enums::ChannelIdLength);
    toSend.write(message, Enums::MessageLength);
    toSend.end();

    debug(tr("[%1] [To %6] <font color='%2'>%3 <b>%4:</b></font> %5")
          .arg(channel->name(), color, timestamp(), name, message, client->name()));

    sendOne(client, toSend.toByteArray(), channel);
}

Channel *Server::createChannel(const QString &name, bool permanent)
{
    if (!m_channelNameMap.contains(name))
    {
        int id = generateChannelId();
        Channel *chan = new Channel(id, name, permanent);

        m_channels.append(chan);
        m_channelIdMap.insert(id, chan);
        m_channelNameMap.insert(name, chan);

        emit channelAdded(chan);

        Packet p;
        p.begin(Enums::CreateChannelCommand);
        p.write(chan->id(), Enums::ChannelIdLength);
        p.write(chan->name(), Enums::ChannelNameLength);
        p.end();

        sendAll(p.toByteArray());

        debug("Channel " + chan->name() + " id " + QString::number(chan->id()) + " created");
        return chan;
    }

    return channelFromName(name);
}

QList<Channel *> Server::channels()
{
    return m_channels;
}

QList<Client *> Server::clients()
{
    return m_clients;
}

void Server::removeChannel(Channel *channel)
{
    if (m_channels.contains(channel))
    {
        emit channelRemoved(channel);

        m_channels.removeAll(channel);
        m_channelIdMap.remove(channel->id());
        m_channelNameMap.remove(channel->name());

        Packet p;
        p.begin(Enums::RemoveChannelCommand);
        p.write(channel->id(), Enums::ChannelIdLength);
        p.write(channel->name(), Enums::ChannelNameLength);
        p.end();

        sendAll(p.toByteArray());

        delete channel;
    }
}

void Server::setChatImage()
{
    QString fileName = QFileDialog::getOpenFileName(m_parent, "Choose image...", QString(), tr("Image Files (*.png *.jpg *.bmp)"));
    QFile file(fileName);

    if (file.exists())
    {
        Packet p;
        p.begin(Enums::SetChatImageCommand);
        p.writeImage(fileName, Enums::ChatImageLength);
        p.end();

        sendAll(p.toByteArray());
    }
}

bool Server::addFlood(const QString &ip)
{
    quint64 currentTime = QDateTime::currentMSecsSinceEpoch();

    if (!m_floodMap.contains(ip))
    {
        m_floodMap.insert(ip, 0);
    }
    else
    {
        quint64 lastTime = m_floodTimeMap[ip];

        int reduce = (currentTime - lastTime) * 5 / 100;
        m_floodMap[ip] = qMax(m_floodMap[ip] - reduce, 0);
    }

    m_floodMap[ip] += m_floodPenalty;
    m_floodTimeMap[ip] = currentTime;

    if (m_floodMap[ip] > m_floodLimit)
    {
        kick(ip);

        if (!m_floodCountMap.contains(ip))
        {
            m_floodCountMap.insert(ip, 0);
        }

        m_floodCountMap[ip]++;

        if (m_floodCountMap[ip] > 5)
        {
            ban(ip);
        }

        return true;
    }

    return false;
}

void Server::kick(const QString &ip)
{
    foreach (Client *client, m_ipMap[ip])
    {
        client->disconnected();
    }
}

void Server::ban(const QString &ip)
{
    kick(ip);
    m_bans.append(ip);

    QSettings s;
    s.setValue("bans", m_bans);

    debug(ip + " was banned.");
}

void Server::unban(const QString &ip)
{
    m_bans.removeAll(ip);

    QSettings s;
    s.setValue("bans", m_bans);
}

void Server::error()
{
    debug("<b color='red'>" + ((QWebSocket*)sender())->errorString() + "</b>");
}

// ************************************************** // command handling // ************************************************** //

void Server::handleMessage(Packet p, Client *client)
{
    // expecting channelId, message //
    QString name, message, color;
    int channelId;

    name = client->name();
    color = client->color();
    channelId = p.readInt(Enums::ChannelIdLength);
    message = p.readString(Enums::MessageLength);

    Channel *channel = channelFromId(channelId);

    sendMessageToAll(message, channel, name, color);
}

void Server::handlePM(Packet p, Client *client)
{
    // expecting id, message //
    int id;
    QString message;

    id = p.readInt(Enums::IdLength);
    message = p.readString(Enums::MessageLength);

    Packet s;
    s.begin(Enums::PMCommand);
    s.write(client->id(), Enums::IdLength);
    s.write(message, Enums::MessageLength);
    s.end();

    sendOne(m_clientMap[id], s.toByteArray(), all);
}

void Server::handleImage(Packet p, Client *client)
{
    QString name, message, color;
    int channelId;

    name = client->name();
    color = client->color();
    channelId = p.readInt(Enums::ChannelIdLength);
    message = p.readString(Enums::ImageLength);

    Channel *channel = channelFromId(channelId);

    Packet s;
    s.begin(Enums::ImageCommand);
    s.write(name, Enums::NameLength);
    s.write(color, Enums::ColorLength);
    s.write(channelId, Enums::ChannelIdLength);
    s.write(message, Enums::ImageLength);
    s.end();

    sendChannel(channel, s.toByteArray());

    debug(tr("[%1] <font color='%2'>%3 <b>%4:</b></font> %5")
          .arg(channel->name(), color, timestamp(), name.toHtmlEscaped(), message.replace("[", "<").replace("]", ">")));
}

void Server::handleDraw(Packet p, Client *client)
{

    QString color;

    int x1, y1, x2, y2;

    color = p.readString(Enums::ColorLength);
    x1 = p.readInt(Enums::NumberLength);
    y1 = p.readInt(Enums::NumberLength);
    x2 = p.readInt(Enums::NumberLength);
    y2 = p.readInt(Enums::NumberLength);

    Packet s;
    s.begin(Enums::DrawCommand);
    s.write(color, Enums::ColorLength);
    s.write(x1, Enums::NumberLength);
    s.write(y1, Enums::NumberLength);
    s.write(x2, Enums::NumberLength);
    s.write(y2, Enums::NumberLength);
    s.end();

    sendAll(s.toByteArray());
}

void Server::handleClear(Packet p, Client *client)
{
    Packet s;
    s.begin(Enums::ClearCommand);
    s.write(client->id(), Enums::IdLength);
    s.write(client->name(), Enums::NameLength);
    s.write(client->color(), Enums::ColorLength);
    s.end();

    sendAll(s.toByteArray());
}

void Server::handleBoardData(Packet p, Client *client)
{
    QString data = p.readString(Enums::ImageLength);
    int id = p.readInt(Enums::IdLength);

    m_drawBoard = data;
    QSettings s;
    s.setValue("drawBoard", data);

    if (m_clientMap.contains(id))
    {
        Packet s;
        s.begin(Enums::BoardDataCommand);
        s.write(data, Enums::ImageLength);
        s.end();

        sendOne(m_clientMap[id], s.toByteArray(), all);
    }
}

void Server::handleJoin(Packet p, Client *client)
{
    // expecting name, color, sprite //
    QString name, color;
    int id, sprite;

    name = p.readString(Enums::NameLength);
    color = p.readString(Enums::ColorLength);
    sprite = p.readInt(Enums::SpriteLength);
    id = generateId();

    client->setInfo(id, name, color, sprite);
    m_clientMap.insert(id, client);
    m_clientNameMap.insert(name, client);

    client->sendChannels(m_channels);

    this->defaultChannel()->addClient(client);

    emit userAdded(client);

    sendMessageToOne(m_welcomeMessage, client, all, "Welcome Message", "#0000FF");

    if (m_clients.length() > 1)
    {
        // here we're requesting the board data from a user, giving them the id of the player that just joined so they can see the board !!
        Packet req;
        req.begin(Enums::RequestBoardCommand);
        req.write(id, Enums::IdLength);
        req.end();

        sendOne(m_clients[0], req.toByteArray(), all);
    }
    else if (m_drawBoard != "")
    {
        Packet b;
        b.begin(Enums::BoardDataCommand);
        b.write(m_drawBoard, Enums::ImageLength);
        b.end();

        sendOne(client, b.toByteArray(), all);
    }

    debug(tr("<i>%1 <font color='%2'><b>%3</b></font> joined!</i>").arg(timestamp(), color, name.toHtmlEscaped()));
}

void Server::handleUnjoin(Packet p, Client *client)
{
    // just expecting command here, not really necessary but who knows??? (they can just call disconnectFromHost on their socket)
    client->disconnected();
}

void Server::handleJoinChannel(Packet p, Client *client)
{
    // expecting channel id
    int channelId = p.readInt(Enums::ChannelIdLength);
    debug(client->name() + " joined " + channelFromId(channelId)->name() + ".");

    channelFromId(channelId)->addClient(client);
}

void Server::handleUnjoinChannel(Packet p, Client *client)
{
    // expecting channel id
    int channelId = p.readInt(Enums::ChannelIdLength);
    debug(client->name() + " left " + channelFromId(channelId)->name() + ".");

    Channel *c = channelFromId(channelId);

    if (c != mysteryZone)
    {
        c->removeClient(client);

        if (c->isEmpty() && !c->isPermanent())
        {
            removeChannel(c);
        }
    }
}

void Server::handleCreateChannel(Packet p, Client *client)
{
    // expecting channel name
    QString name = p.readString(Enums::ChannelNameLength);
    Channel *c = createChannel(name);
    c->addClient(client);
}

void Server::handleRemoveChannel(Packet p, Client *client)
{
    // expecting channel id
    int channelId = p.readInt(Enums::ChannelIdLength);
    removeChannel(channelFromId(channelId));
}

// ************************************************** // end command handling // ************************************************** //

QString Server::timestamp()
{
    QDateTime now = QDateTime::currentDateTime();

    return "(" + now.toString("HH:mm:ss") + ")";
}

void Server::onNewConnection()
{
    QWebSocket *s = this->nextPendingConnection();
    QString ip = s->peerAddress().toString();

    if (m_bans.contains(ip))
    {
        //s->close();
        s->deleteLater();
        //debug("Banned user from " + ip + " was disconnected.");
        return;
    }

    Client *client = new Client(s);
    m_clients.append(client);

    if (addFlood(ip))
    {
        return;
    }

    if (!m_ipMap.contains(ip))
    {
        m_ipMap.insert(ip, QList<Client*>());
    }

    m_ipMap[ip] << client;

    debug("New client from: " + ip);

    connect(client, SIGNAL(readyRead(QString)), this, SLOT(readyRead(QString)));
    connect(client, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
    connect(s, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error()));
}

Channel *Server::channelFromId(int id)
{
    if (m_channelIdMap.contains(id))
    {
        return m_channelIdMap[id];
    }

    return mysteryZone;
}

Channel *Server::channelFromName(const QString &name)
{
    if (m_channelNameMap.contains(name))
    {
        return m_channelNameMap[name];
    }

    return mysteryZone;
}

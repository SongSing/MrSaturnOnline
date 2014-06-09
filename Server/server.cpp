#include "server.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QFileDialog>

Server::Server(const QString &name, SslMode mode, QObject *parent) :
    QWebSocketServer(name, mode, parent)
{
    connect(this, SIGNAL(newConnection()), this, SLOT(onNewConnection()));

    m_parent = (QWidget*)parent;
    all = Channel::all();
    mysteryZone = Channel::mysteryZone();

    m_welcomeMessage = "Hi ho! You come buying?";

    createChannel("Saturn Valley", true);
    createChannel("Saturn Hot Springs", true);
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
        foreach (Channel *c, client->channels())
        {
            c->sendOne(client, data);
        }
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

    if (command == Enums::MessageCommand)
    {
        handleMessage(p, client);
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

}

void Server::clientDisconnected()
{
    Client *client = (Client*)sender();

    emit userRemoved(client);

    QString name = client->name();
    QString color = client->color();

    m_clients.removeAll(client);
    m_clientMap.remove(client->id());

    foreach (Channel *channel, client->channels())
    {
        channel->removeClient(client);
    }

    delete client;

    /*Packet p;
    p.begin(Enums::UnjoinCommand);
    p.write(name, Enums::NameLength);
    p.write(color, Enums::ColorLength);
    p.end();

    sendAll(p.toByteArray());
    sendUserList();*/
}

void Server::sendUserList()
{

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

    debug(tr("[%1] <font color='%2'>%3 <b>%4:</b></font> %5").arg(channel->name(), color, timestamp(), name, message));

    sendMessageToAll(message, channel, name, color);
}

void Server::handleJoin(Packet p, Client *client)
{
    // expecting name, color //
    QString name, color;
    int id;

    name = p.readString(Enums::NameLength);
    color = p.readString(Enums::ColorLength);
    id = generateId();

    client->setInfo(id, name, color);
    m_clientMap.insert(id, client);

    client->sendChannels(m_channels);

    this->defaultChannel()->addClient(client);

    emit userAdded(client);

    sendMessageToOne(m_welcomeMessage, client, all, "Welcome Message", "#0000FF");

    debug(tr("<i>%1 <font color='%2'><b>%3</b></font> joined!</i>").arg(timestamp(), color, name));
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
    debug(client->name() + " joined " + channelFromId(channelId)->name());

    channelFromId(channelId)->addClient(client);
}

void Server::handleUnjoinChannel(Packet p, Client *client)
{
    // expecting channel id
    int channelId = p.readInt(Enums::ChannelIdLength);
    debug(client->name() + " left " + channelFromId(channelId)->name());

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
    Client *client = new Client(this->nextPendingConnection());
    m_clients.append(client);

    debug("New client from: " + client->socket()->peerAddress().toString());

    connect(client, SIGNAL(readyRead(QString)), this, SLOT(readyRead(QString)));
    connect(client, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
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

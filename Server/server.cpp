#include "server.h"

#include <QDateTime>

Server::Server(QObject *parent) :
    QTcpServer(parent)
{
    m_welcomeMessage = "Hi ho! You come buying?";
    Channel *chan = new Channel(0, "Saturn Valley");
    m_channels.append(chan);
}

void Server::sendAll(const QByteArray &data)
{
    foreach (Client *client, m_clients)
    {
        client->socket()->write(data);
    }
}

void Server::sendOne(Client *client, const QByteArray &data, Channel *channel)
{
    channel->sendOne(client, data);
}

void Server::sendChannel(Channel *channel, const QByteArray &data)
{
    if (channel == Channel::all())
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

Channel *Server::defaultChannel()
{
    if (m_channels.isEmpty())
    {
        return Channel::mysteryZone();
    }

    return m_channels[0];
}

void Server::readyRead()
{
    Client *client = (Client*)sender();

    while (client->socket()->canReadLine())
    {
        Packet p(client->socket()->readLine());
        Enums::Command command = (Enums::Command)p.readCommand();

        debug(QString::number(command) + " - " + client->name());

        if (command == Enums::MessageCommand)
        {
            // expecting channelId, message //
            QString name, message, colour;
            int channelId;

            name = client->name();
            colour = client->colour();
            channelId = p.readInt(Enums::ChannelIdLength);
            message = p.readString(Enums::MessageLength);

            Channel *channel = channelFromId(channelId);

            debug(tr("[%1] <font color='%2'>%3 <b>%4:</b> %5").arg(channel->name(), colour, timestamp(), name, message));

            sendMessageToAll(message, channel, name, colour);
        }
        else if (command == Enums::JoinCommand)
        {
            // expecting name, colour //
            QString name, colour;
            int id;

            name = p.readString(Enums::NameLength);
            colour = p.readString(Enums::ColourLength);
            id = generateId();

            client->setInfo(id, name, colour);
            m_clientMap.insert(id, client);

            client->sendChannels(m_channels);

            this->defaultChannel()->addClient(client);

            sendMessageToOne(m_welcomeMessage, client, Channel::all(), "Welcome Message", "#0000FF");

            debug(tr("<i>%1 <font color='%2'><b>%3</b></font> joined!</i>").arg(timestamp(), colour, name));
        }
        else if (command == Enums::UnjoinCommand)
        {
            // just expecting command here, not really necessary but who knows??? (they can just call disconnectFromHost on their socket)
            client->disconnected();
        }
        else if (command == Enums::JoinChannelCommand)
        {
            // expecting channel id

            int channelId = p.readInt(Enums::ChannelIdLength);

            channelFromId(channelId)->addClient(client);
        }
    }
}

void Server::clientDisconnected()
{
    Client *client = (Client*)sender();

    QString name = client->name();
    QString colour = client->colour();

    m_clients.removeAll(client);
    m_clientMap.remove(client->id());

    foreach (Channel *channel, client->channels())
    {
        channel->removeClient(client);
    }

    delete client;

    Packet p;
    p.begin(Enums::UnjoinCommand);
    p.write(name, Enums::NameLength);
    p.write(colour, Enums::ColourLength);
    p.end();

    sendAll(p.toByteArray());
    sendUserList();
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

void Server::sendMessageToAll(const QString &message, Channel *channel, const QString &name, const QString &colour)
{
    Packet toSend;
    toSend.begin(Enums::MessageCommand);
    toSend.write(name, Enums::NameLength);
    toSend.write(colour, Enums::ColourLength);
    toSend.write(channel->id(), Enums::ChannelIdLength);
    toSend.write(message, Enums::MessageLength);
    toSend.end();

    sendChannel(channel, toSend.toByteArray());
}

void Server::sendMessageToOne(const QString &message, Client *client, Channel *channel, const QString &name, const QString &colour)
{
    Packet toSend;
    toSend.begin(Enums::MessageCommand);
    toSend.write(name, Enums::NameLength);
    toSend.write(colour, Enums::ColourLength);
    toSend.write(channel->id(), Enums::ChannelIdLength);
    toSend.write(message, Enums::MessageLength);
    toSend.end();

    sendOne(client, toSend.toByteArray(), channel);
}

QString Server::timestamp()
{
    QDateTime now = QDateTime::currentDateTime();

    return "(" + now.toString("HH:mm:ss") + ")";
}

void Server::incomingConnection(int socketId)
{
    Client *client = new Client(socketId);
    m_clients.append(client);

    debug("New client from: " + client->socket()->peerAddress().toString());

    connect(client, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(client, SIGNAL(disconnected()), this, SLOT(disconnected()));
}

Channel *Server::channelFromId(int id)
{
    foreach (Channel *chan, m_channels)
    {
        if (chan->id() == id)
        {
            return chan;
        }
    }

    return Channel::mysteryZone();
}

Channel *Server::channelFromName(const QString &name)
{
    foreach (Channel *chan, m_channels)
    {
        if (chan->name().toLower() == name.toLower())
        {
            return chan;
        }
    }

    return Channel::mysteryZone();
}

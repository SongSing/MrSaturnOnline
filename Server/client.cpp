#include "client.h"

Client::Client(int socketId)
{
    m_name = "???";
    m_color = "#000000";
    m_id = -1;
    m_socketId = socketId;

    m_socket = new QTcpSocket();
    m_socket->setSocketDescriptor(socketId);

    connect(m_socket, SIGNAL(readyRead()), this, SLOT(socketReadyRead()));
    connect(m_socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
}

void Client::setInfo(int id, const QString &name, const QString &color)
{
    m_id = id;
    m_name = name;
    m_color = color;
}

QString Client::name()
{
    return m_name;
}

QString Client::color()
{
    return m_color;
}

int Client::id()
{
    return m_id;
}

QTcpSocket *Client::socket()
{
    return m_socket;
}

QList<Channel *> Client::channels()
{
    return m_channels;
}

bool Client::hasChannel(Channel *channel)
{
    return m_channels.contains(channel);
}

void Client::addChannel(Channel *channel)
{
    if (!m_channels.contains(channel))
    {
        m_channels.append(channel);

        // sending id

        Packet c;
        c.begin(Enums::JoinChannelCommand);
        c.write(channel->id());
        c.end();

        m_socket->write(c.toByteArray());

        // sending channelId, ids, names, colors

        Packet p;
        p.begin(Enums::UserListCommand);
        p.write(channel->id(), Enums::ChannelIdLength);
        p.write(channel->clientIds(), Enums::IdListLength, Enums::IdLength);
        p.write(channel->clientNames(), Enums::NameListLength, Enums::NameLength);
        p.write(channel->clientColors(), Enums::ColorListLength, Enums::ColorLength);
        p.end();

        m_socket->write(p.toByteArray());
    }
}

void Client::removeChannel(Channel *channel)
{
    m_channels.removeAll(channel);
}

bool Client::isValid()
{
    return m_id >= 0;
}

void Client::sendChannels(QList<Channel *> channels)
{
    // sending ids, names

    QList<int> ids;
    QStringList names;

    foreach (Channel *channel, channels)
    {
        ids << channel->id();
        names << channel->name();
    }

    Packet p;
    p.begin(Enums::ChannelListCommand);
    p.write(ids, Enums::ChannelIdListLength, Enums::ChannelIdLength);
    p.write(names, Enums::ChannelNameListLength, Enums::ChannelNameLength);
    p.end();

    m_socket->write(p.toByteArray());
}

void Client::socketReadyRead()
{
    emit readyRead();
}

void Client::socketDisconnected()
{
    emit disconnected();
}

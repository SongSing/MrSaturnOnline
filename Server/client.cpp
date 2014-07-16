#include "client.h"


Client::Client(QWebSocket *socket)
{
    m_name = "???";
    m_color = "#000000";
    m_id = -1;
    m_sprite = 0;

    m_socket = socket;

    // wire these through client slots instead of signals so the sender can be a client object //
    connect(m_socket, SIGNAL(textMessageReceived(QString)), this, SLOT(socketReadyRead(QString)));
    connect(m_socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
}

void Client::setInfo(int id, const QString &name, const QString &color, int sprite)
{
    m_id = id;
    m_name = name;
    m_color = color;
    m_sprite = sprite;
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

int Client::sprite()
{
    return m_sprite;
}

QWebSocket *Client::socket()
{
    return m_socket;
}

QList<Channel *> Client::channels()
{
    return m_channels;
}

QString Client::ip()
{
    return m_socket->peerAddress().toString();
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
        c.write(channel->id(), Enums::ChannelIdLength);
        c.end();

        write(c.toByteArray());

        // sending channelId, ids, names, colors

        Packet p;
        p.begin(Enums::UserListCommand);
        p.write(channel->id(), Enums::ChannelIdLength);
        p.write(channel->clientIds(), Enums::IdListLength, Enums::IdLength);
        p.write(channel->clientNames(), Enums::NameListLength, Enums::NameLength);
        p.write(channel->clientColors(), Enums::ColorListLength, Enums::ColorLength);
        p.write(channel->clientSprites(), Enums::SpriteListLength, Enums::SpriteLength);
        p.end();

        write(p.toByteArray());
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

    write(p.toByteArray());
}

void Client::write(const QByteArray &data)
{
    if (this != NULL && this->socket() != NULL)
        m_socket->sendTextMessage(QString(data));
}

void Client::socketReadyRead(const QString &message)
{
    emit readyRead(message);
}

void Client::socketDisconnected()
{
    emit disconnected();
}

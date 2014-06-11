#include "channel.h"
#include "client.h"

Channel::Channel(int id, const QString &name, bool permanent) :
    QObject()
{
    m_id = id;
    m_name = name;
    m_permanent = permanent;
}

bool Channel::operator ==(Channel *c)
{
    return m_id == c->id();
}

bool Channel::operator ==(Channel c)
{
    return m_id == c.id();
}

Channel *Channel::all() // used for all channels
{
    return new Channel(-1, "%All%");
}

Channel *Channel::mysteryZone() // error channel
{
    return new Channel(-2, "%Mystery Zone%");
}

QString Channel::name()
{
    return m_name;
}

bool Channel::isPermanent()
{
    return m_permanent;
}

bool Channel::isEmpty()
{
    return m_clients.isEmpty();
}

QList<Client *> Channel::clients()
{
    return m_clients;
}

QList<int> Channel::clientIds()
{
    QList<int> ret;

    foreach (Client *client, m_clients)
    {
        ret << client->id();
    }

    return ret;
}

QStringList Channel::clientNames()
{
    QStringList ret;

    foreach (Client *client, m_clients)
    {
        ret << client->name();
    }

    return ret;
}

QStringList Channel::clientColors()
{
    QStringList ret;

    foreach (Client *client, m_clients)
    {
        ret << client->color();
    }

    return ret;
}

QList<int> Channel::clientSprites()
{
    QList<int> ret;

    foreach (Client *client, m_clients)
    {
        ret << client->sprite();
    }

    return ret;
}

int Channel::id()
{
    return m_id;
}

void Channel::sendAll(const QByteArray &data)
{
    foreach (Client *client, m_clients)
    {
        client->write(data);
    }
}

void Channel::sendOne(Client *client, const QByteArray &data)
{
    if (this == Channel::all())
    {
        foreach (Channel *channel, client->channels())
        {
            channel->sendOne(client, data);
        }
    }
    else
    {
        if (m_clients.contains(client))
        {
            client->write(data);
        }
    }
}

void Channel::sendAllButOne(Client *excluded, const QByteArray &data)
{
    foreach (Client *client, m_clients)
    {
        if (client != excluded)
        {
            client->write(data);
        }
    }
}

void Channel::addClient(Client *client)
{
    if (!m_clients.contains(client))
    {
        m_clients.append(client);
        client->addChannel(this);

        Packet p;
        p.begin(Enums::UserJoinedChannelCommand);
        p.write(m_id, Enums::ChannelIdLength);
        p.write(client->id(), Enums::IdLength);
        p.write(client->name(), Enums::NameLength);
        p.write(client->color(), Enums::ColorLength);
        p.write(client->sprite(), Enums::SpriteLength);
        p.end();

        sendAllButOne(client, p.toByteArray());
    }
}

void Channel::removeClient(Client *client)
{
    if (m_clients.contains(client))
    {
        m_clients.removeAll(client);
        client->removeChannel(this);

        Packet p;
        p.begin(Enums::UserLeftChannelCommand);
        p.write(m_id, Enums::ChannelIdLength);
        p.write(client->id(), Enums::IdLength);
        p.write(client->name(), Enums::NameLength);
        p.write(client->color(), Enums::ColorLength);
        p.end();

        sendAll(p.toByteArray());
    }
}

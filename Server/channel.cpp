#include "channel.h"
#include "client.h"

Channel::Channel(int id, const QString &name) :
    QObject()
{
    m_id = id;
    m_name = name;
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

int Channel::id()
{
    return m_id;
}

void Channel::sendAll(const QByteArray &data)
{
    foreach (Client *client, m_clients)
    {
        client->socket()->write(data);
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
            client->socket()->write(data);
        }
    }
}

void Channel::addClient(Client *client)
{
    if (!m_clients.contains(client))
    {
        m_clients.append(client);
        client->addChannel(this);
    }
}

void Channel::removeClient(Client *client)
{
    if (m_clients.contains(client))
    {
        m_clients.removeAll(client);
        client->removeChannel(this);
    }
}

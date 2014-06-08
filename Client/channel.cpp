#include "channel.h"

Channel::Channel()
{
    m_id = mysteryZone().id();
    m_name = mysteryZone().name();
}

Channel::Channel(int id, const QString &name)
{
    m_id = id;
    m_name = name;
}

bool Channel::operator ==(Channel channel)
{
    return m_id == channel.id();
}

bool Channel::operator !=(Channel channel)
{
    return m_id != channel.id();
}

Channel Channel::all() // used for all channels
{
    return Channel(-1, "%All%");
}

Channel Channel::mysteryZone() // error channel
{
    return Channel(-2, "%Mystery Zone%");
}

QString Channel::name()
{
    return m_name;
}

int Channel::id()
{
    return m_id;
}

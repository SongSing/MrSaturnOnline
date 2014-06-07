#include "user.h"

User::User(int id, const QString &name, const QString &color)
{
    m_id = id;
    m_name = name;
    m_color = color;
}

bool User::operator ==(User user)
{
    return m_id == user.id();
}

int User::id()
{
    return m_id;
}

QString User::name()
{
    return m_name;
}

QString User::color()
{
    return m_color;
}

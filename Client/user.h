#ifndef USER_H
#define USER_H

#include <QtCore>

class User
{
public:
    User(int id, const QString &name, const QString &colour);

    bool operator ==(User user);

    int id();
    QString name();
    QString colour();

private:
    int m_id;
    QString m_name;
    QString m_colour;
};

#endif // USER_H

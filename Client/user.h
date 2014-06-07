#ifndef USER_H
#define USER_H

#include <QtCore>

class User
{
public:
    User(int id, const QString &name, const QString &color);

    bool operator ==(User user);

    int id();
    QString name();
    QString color();

private:
    int m_id;
    QString m_name;
    QString m_color;
};

#endif // USER_H

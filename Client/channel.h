#ifndef CHANNEL_H
#define CHANNEL_H

#include <QtCore>

class Channel
{
public:
    Channel();
    Channel(int id, const QString &name);

    bool operator ==(Channel channel);
    bool operator !=(Channel channel);

    static Channel all();
    static Channel mysteryZone();

    QString name();
    int id();

private:
    int m_id;
    QString m_name;
};

#endif // CHANNEL_H

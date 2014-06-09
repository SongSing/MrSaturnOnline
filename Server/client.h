#ifndef CLIENT_H
#define CLIENT_H

#include <QtDebug>
#include <QWebSocket>
#include <QObject>

#include "../lib/packet.h"
#include "../lib/enums.h"
#include "channel.h"

class Client : public QObject
{
    Q_OBJECT
public:
    explicit Client(QWebSocket *socket);

    void setInfo(int id, const QString &name, const QString &color);

    QString name();
    QString color();
    int id();
    QList<Channel*> channels();

    QWebSocket *socket();

    bool hasChannel(Channel *channel);
    void addChannel(Channel *channel);
    void removeChannel(Channel *channel);

    bool isValid();

signals:
    void readyRead(const QString &message);
    void disconnected();

public slots:
    void sendChannels(QList<Channel*> channels);
    void write(const QByteArray &data);

private slots:
    void socketReadyRead(const QString &message);
    void socketDisconnected();

private:
    QString m_name;
    QString m_color;
    int m_socketId, m_id;
    QWebSocket *m_socket;
    int m_userId;
    QList<Channel*> m_channels;
    bool m_isWeb;

};

#endif // CLIENT_H

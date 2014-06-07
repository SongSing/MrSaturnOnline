#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include "client.h"
#include "channel.h"
#include "../lib/enums.h"
#include "../lib/packet.h"

class Server : public QTcpServer
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = 0);
    void sendCash(double amount, QObject doctor);
    int generateId();

    Channel *defaultChannel();

signals:
    void debug(const QString &message);
    void usersChanged(QStringList names, QStringList colors);
    void channelsChanged(QStringList names);

public slots:
    void sendAll(const QByteArray &data);
    void sendOne(Client *client, const QByteArray &data, Channel *channel = Channel::all());
    void sendChannel(Channel *channel, const QByteArray &data);
    void setWelcomeMessage(const QString &message);
    void readyRead();
    void clientDisconnected();
    void sendUserList();
    void sendChannelList();
    void sendMessageToAll(const QString &message, Channel *channel = Channel::all(), const QString &name = "~Server~", const QString &color = "#000000");
    void sendMessageToOne(const QString &message, Client *client, Channel *channel, const QString &name = "~Server~", const QString &color = "#000000");

    private:
    QList<Client*> m_clients;
    QList<Channel*> m_channels;
    QHash<int, Client*> m_clientMap;
    QString timestamp();
    QString m_welcomeMessage;
    Channel *channelFromId(int id);
    Channel *channelFromName(const QString &name);

protected:
    void incomingConnection(int socketId);

};

#endif // SERVER_H

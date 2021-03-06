#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QWebSocketServer>
#include "client.h"
#include "channel.h"
#include "../lib/enums.h"
#include "../lib/packet.h"

class Server : public QWebSocketServer
{
    Q_OBJECT
public:
    explicit Server(const QString &name = "MrSaturnOnline", SslMode mode = SslMode::NonSecureMode, QObject *parent = 0);
    void sendCash(double amount, QObject doctor);
    int generateId();
    int generateChannelId();

    Channel *defaultChannel();
    Channel *allChannels();

    Channel *createChannel(const QString &name, bool permanent = false);

    QList<Channel*> channels();
    QList<Client*> clients();

signals:
    void debug(const QString &message);
    void userAdded(Client *client);
    void userRemoved(Client *client);
    void channelAdded(Channel *channel);
    void channelRemoved(Channel *channel);

public slots:
    void sendAll(const QByteArray &data);
    void sendOne(Client *client, const QByteArray &data, Channel *channel);
    void sendChannel(Channel *channel, const QByteArray &data);
    void setWelcomeMessage(const QString &message);
    void readyRead(const QString &message);
    void clientDisconnected();
    void sendChannelList();
    void sendMessageToAll(const QString &message, Channel *channel, const QString &name = "~Server~", const QString &color = "#000000");
    void sendMessageToOne(const QString &message, Client *client, Channel *channel, const QString &name = "~Server~", const QString &color = "#000000");
    void removeChannel(Channel *channel);
    void setChatImage();
    bool addFlood(const QString &ip);
    void kick(const QString &ip);
    void ban(const QString &ip);
    void unban(const QString &ip);
    void error();

    // command handling //
    void handleMessage(Packet p, Client *client);
    void handlePM(Packet p, Client *client);
    void handleImage(Packet p, Client *client);
    void handleDraw(Packet p, Client *client);
    void handleClear(Packet p, Client *client);
    void handleBoardData(Packet p, Client *client);
    void handleJoin(Packet p, Client *client);
    void handleUnjoin(Packet p, Client *client);
    void handleJoinChannel(Packet p, Client *client);
    void handleUnjoinChannel(Packet p, Client *client);
    void handleCreateChannel(Packet p, Client *client);
    void handleRemoveChannel(Packet p, Client *client);

    void onNewConnection();

private:
    QString m_drawBoard;
    QList<Client*> m_clients;
    QList<Channel*> m_channels;
    QHash<int, Client*> m_clientMap;
    QHash<QString, Client*> m_clientNameMap;
    QHash<int, Channel*> m_channelIdMap;
    QHash<QString, int> m_floodMap;
    QHash<QString, quint64> m_floodTimeMap;
    QHash<QString, int> m_floodCountMap;
    QStringList m_bans;
    QHash<QString, QList<Client*>> m_ipMap;
    int m_floodPenalty, m_floodLimit;
    QHash<QString, Channel*> m_channelNameMap;
    QString timestamp();
    QString m_welcomeMessage;
    Channel *channelFromId(int id);
    Channel *channelFromName(const QString &name);
    Channel *all, *mysteryZone;
    QWidget *m_parent;

protected:

};

#endif // SERVER_H

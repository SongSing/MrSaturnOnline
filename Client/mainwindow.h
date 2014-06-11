#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QSettings>
#include <QTextBrowser>
#include <QListWidgetItem>
#include <QWebSocket>
#include "channel.h"
#include "user.h"
#include "../lib/enums.h"
#include "../lib/packet.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    Channel currentChannel();
    bool hasChannel(Channel c);

public slots:
    void chooseColor();
    void connectToServer();
    void disconnectFromServer();
    void connected();
    void disconnected();
    void readyRead(const QString &message);
    void sendMessage();
    void sendPacket(Packet p);
    void setCurrentChannel(Channel c);
    void joinChannel(QListWidgetItem *item);
    void joinChannel(Channel c);
    void leaveChannel(int ind);
    void appendChat(const QString &text);
    void appendChannel(int channelId, const QString &text);
    void currentChanged(int ind);
    void createChannel();
    void addChannel(int id, const QString &name);

    // command handling //
    void handleMessage(Packet p);
    void handleImage(Packet p);
    void handleJoin(Packet p);
    void handleJoinChannel(Packet p);
    void handleChannelList(Packet p);
    void handleAddChannel(Packet p);
    void handleRemoveChannel(Packet p);
    void handleUserList(Packet p);
    void handleUserJoinedChannel(Packet p);
    void handleUserLeftChannel(Packet p);
    void handleSetChatImage(Packet p);

private:
    Ui::MainWindow *ui;
    QString m_name, m_color, m_host;
    int m_id, m_port;
    QWebSocket *m_socket;
    QList<Channel> all_channels;
    QList<Channel> m_channels;
    int m_currentChannelId;
    QTextBrowser *initialChat;

    QHash<int, QTextBrowser*> m_channelMap;
    QHash<int, QList<User>> m_userMap;
    QHash<int, QListWidgetItem*> m_channelListMap;

    Channel channelFromId(int id);
    Channel channelFromName(const QString &name, bool caseSens = true);
    Channel all, mysteryZone;

    QString timestamp();
};

#endif // MAINWINDOW_H

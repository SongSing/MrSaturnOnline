#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QSettings>
#include <QTextBrowser>
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
    void chooseColour();
    void connectToServer();
    void disconnectFromServer();
    void connected();
    void disconnected();
    void readyRead();
    void sendMessage();
    void sendPacket(Packet p);
    void setCurrentChannel(Channel c);
    void joinChannel(Channel c);
    void appendChat(const QString &text);
    void appendChannel(int channelId, const QString &text);

private:
    Ui::MainWindow *ui;
    QString m_name, m_colour, m_host;
    int m_id, m_port;
    QTcpSocket *m_socket;
    QList<Channel> all_channels;
    QList<Channel> m_channels;
    int m_currentChannelId;

    QHash<int, QTextBrowser*> m_channelMap;
    QHash<int, QList<User>> m_userMap;

    Channel channelFromId(int id);
    Channel channelFromName(const QString &name);

    QString timestamp();
};

#endif // MAINWINDOW_H

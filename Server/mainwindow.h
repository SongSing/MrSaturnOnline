#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWebSocketServer>
#include <QWebSocket>
#include "server.h"
#include "client.h"
#include "channel.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void startServer();
    void sendMessage();
    void appendChat(const QString &message);
    void addUser(Client *client);
    void removeUser(Client *client);
    void addChannel(Channel *channel);
    void removeChannel(Channel *channel);

private:
    Ui::MainWindow *ui;
    Server *m_server;
};

#endif // MAINWINDOW_H

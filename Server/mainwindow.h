#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "server.h"

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
    void updateUsers(const QStringList &names, const QStringList &colors);
    void updateChannels(const QStringList &names);

private:
    Ui::MainWindow *ui;
    Server *m_server;
};

#endif // MAINWINDOW_H

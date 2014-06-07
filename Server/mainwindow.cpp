#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSettings>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // mibu is gay
    QCoreApplication::setOrganizationName("Team Zesty");
    QCoreApplication::setApplicationName("Mr. Saturn Online Server");
    QApplication::setStyle("Fusion");
    ui->setupUi(this);

    QSettings s;

    ui->stackedWidget->setCurrentIndex(0);
    ui->welcomeMessage->setPlainText(s.value("welcomeMessage", "Welcome Message").toString());

    connect(ui->start, SIGNAL(clicked()), this, SLOT(startServer()));
    connect(ui->send, SIGNAL(clicked()), this, SLOT(sendMessage()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::startServer()
{
    m_server = new Server(this);
    m_server->setWelcomeMessage(ui->welcomeMessage->toPlainText());
    connect(m_server, SIGNAL(debug(QString)), this, SLOT(appendChat(QString)));
    connect(m_server, SIGNAL(usersChanged(QStringList,QStringList)), this, SLOT(updateUsers(QStringList,QStringList)));

    ui->stackedWidget->setCurrentIndex(1);

    appendChat("<i>Starting server...</i>");

    if (m_server->listen(QHostAddress(ui->host->text()), ui->port->value()))
    {
        appendChat("<b>Started Server!</b> Listening on " + ui->host->text() + ", port " + ui->port->text());
    }
    else
    {
        appendChat("<b><font color='red'>Server couldn't be started!</font></b>");
    }
}

void MainWindow::sendMessage()
{
    QString message = ui->input->text();

    if (!message.isEmpty())
    {
        ui->input->clear();

        m_server->sendMessageToAll(message);
    }
}

void MainWindow::appendChat(const QString &message)
{
    ui->chat->append(message);
}

void MainWindow::updateUsers(const QStringList &names, const QStringList &colours)
{
    ui->users->clear();

    for (int i = 0; i < names.length(); i++)
    {
        QListWidgetItem *item = new QListWidgetItem(names[i]);
        item->setForeground(QBrush(QColor(colours[i])));
        item->setFont(QFont(item->font().family(), item->font().pointSize(), 50));

        ui->users->addItem(item);
    }
}

void MainWindow::updateChannels(const QStringList &names)
{
    ui->channels->clear();
    ui->channels->addItems(names);
}

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSettings>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // mibu is gay hahas songsing is so angry
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
    connect(ui->actionSet_Chat_Background, SIGNAL(triggered()), m_server, SLOT(setChatImage()));

    connect(m_server, SIGNAL(debug(QString)), this, SLOT(appendChat(QString)));
    connect(m_server, SIGNAL(userAdded(Client*)), this, SLOT(addUser(Client*)));
    connect(m_server, SIGNAL(userRemoved(Client*)), this, SLOT(removeUser(Client*)));
    connect(m_server, SIGNAL(channelAdded(Channel*)), this, SLOT(addChannel(Channel*)));
    connect(m_server, SIGNAL(channelRemoved(Channel*)), this, SLOT(removeChannel(Channel*)));

    foreach (Channel *c, m_server->channels())
    {
        addChannel(c);
    }

    foreach (Client *c, m_server->clients())
    {
        addUser(c);
    }

    ui->stackedWidget->setCurrentIndex(1);

    appendChat("<i>Starting server...</i>");

    if (m_server->listen(QHostAddress(ui->host->text()), ui->port->value()))
    {
        appendChat("<b>Started Server!</b> Listening on " + ui->host->text() + ", port " + ui->port->text());
    }
    else
    {
        appendChat("<b><font color='red'>Server couldn't be started! No buyooooola?</font></b>");
    }
}

void MainWindow::sendMessage()
{
    QString message = ui->input->text();

    if (!message.isEmpty())
    {
        ui->input->clear();

        m_server->sendMessageToAll(message, m_server->allChannels());
    }
}

void MainWindow::appendChat(const QString &message)
{
    ui->chat->append(message);
}

void MainWindow::addUser(Client *client)
{
    QListWidgetItem *item = new QListWidgetItem(client->name());
    item->setForeground(QBrush(QColor(client->color())));
    item->setFont(QFont(item->font().family(), item->font().pointSize(), 60));

    ui->users->addItem(item);
}

void MainWindow::removeUser(Client *client)
{
    QList<QListWidgetItem*> items = ui->users->findItems(client->name(), Qt::MatchCaseSensitive);

    foreach (QListWidgetItem *item, items)
    {
        ui->users->removeItemWidget(item);
        delete item;
    }
}

void MainWindow::addChannel(Channel *channel)
{
    ui->channels->addItem(channel->name());
}

void MainWindow::removeChannel(Channel *channel)
{
    QList<QListWidgetItem*> items = ui->channels->findItems(channel->name(), Qt::MatchCaseSensitive);

    foreach (QListWidgetItem *item, items)
    {
        ui->channels->removeItemWidget(item);
        delete item;
    }
}

#ifndef CHANNEL_H
#define CHANNEL_H

#include <QObject>

class Client;

class Channel : public QObject
{
    Q_OBJECT
public:
    explicit Channel(int id, const QString &name);

    static Channel *all();
    static Channel *mysteryZone();

    int id();
    QString name();

    QList<Client*> clients();
    QList<int> clientIds();
    QStringList clientNames();

signals:

public slots:
    void sendAll(const QByteArray &data);
    void sendOne(Client *client, const QByteArray &data);
    void addClient(Client *client);
    void removeClient(Client *client);

private:
    QList<Client*> m_clients;
    int m_id;
    QString m_name;

};

#endif // CHANNEL_H

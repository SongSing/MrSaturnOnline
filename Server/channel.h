#ifndef CHANNEL_H
#define CHANNEL_H

#include <QObject>

class Client;

class Channel : public QObject
{
    Q_OBJECT
public:
    explicit Channel(int id, const QString &name, bool permanent = false);

    bool operator ==(Channel *c);
    bool operator ==(Channel c);

    static Channel *all();
    static Channel *mysteryZone();

    int id();
    QString name();
    bool isPermanent();

    bool isEmpty();

    QList<Client*> clients();
    QList<int> clientIds();
    QStringList clientNames();
    QStringList clientColors();
    QList<int> clientSprites();

signals:

public slots:
    void sendAll(const QByteArray &data);
    void sendOne(Client *client, const QByteArray &data);
    void sendAllButOne(Client *excluded, const QByteArray &data);
    void addClient(Client *client);
    void removeClient(Client *client);

private:
    QList<Client*> m_clients;
    int m_id;
    QString m_name;
    bool m_permanent;

};

#endif // CHANNEL_H

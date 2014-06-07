#ifndef PACKET_H
#define PACKET_H

#include <QString>
#include <QtDebug>

class Packet : public QString
{
public:
    Packet();
    Packet(const QString &string);
    Packet(const QByteArray &byteArray);

    void begin(quint16 command);
    void write(const QString &string, int bits = 4);
    void write(const double &number, int bits = 4);
    void write(const QStringList &list, int bits = 4, int bitsPerItem = 2);
    void write(const QList<int> &list, int bits = 4, int bitsPerItem = 2);
    void end();

    quint16 readCommand();
    QString readString(int bits = 4);
    int readInt(int bits = 4);
    double readDouble(int bits = 4);
    QStringList readStringList(int bits = 4, int bitsPerItem = 2);
    QList<int> readIntList(int bits = 4, int bitsPerItem = 2);

    QByteArray toByteArray();

private:
    bool began;
};

#endif // PACKET_H

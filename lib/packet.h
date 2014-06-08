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
    void write(const QString &string, int bits);
    void write(const double &number, int bits);
    void write(const QStringList &list, int bits, int bitsPerItem);
    void write(const QList<int> &list, int bits, int bitsPerItem);
    void end();

    quint16 readCommand();
    QString readString(int bits);
    int readInt(int bits);
    double readDouble(int bits);
    QStringList readStringList(int bits, int bitsPerItem);
    QList<int> readIntList(int bits, int bitsPerItem);

    QByteArray toByteArray();

private:
    bool began;
};

#endif // PACKET_H

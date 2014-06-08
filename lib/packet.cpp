/* ******************************************************* NOTES ON PACKET CLASS *******************************************************
 * Super cool class
 * Used for formatting different data types in and out of a QString/QByteArray
 * Format is 2 bits for command, followed by a series of bits and values, the bits specifying the length of the values
 * Example for a message from SongSing saying Sammy using QStrings would be:
 * 0008SongSing0005Sammy [begin(0); write("SongSing", 2); write("Sammy", 4); end();]
 * 00 is command, 08 is the length of SongSing (2 bits since 256 is enough for a name), 0005 is the length of Sammy (big messages!)
 * Anyway, always begin and end when writing, etc., and specifying the bit length is not required, but you should def do it!
 * ************************************************************************************************************************************* */

#include "packet.h"

#include <QFile>

Packet::Packet()
    : QString()
{
    began = false;
}

Packet::Packet(const QString &string)
    : QString(string)
{
    began = false;
}

Packet::Packet(const QByteArray &byteArray)
    : QString(byteArray)
{
    began = false;
}

void Packet::begin(quint16 command)
{
    this->clear();
    this->append(QString::number(command, 16).rightJustified(2, '0'));
    began = true;
}

void Packet::write(const QString &string, int bits)
{
    this->append(QString::number(string.length(), 16).rightJustified(bits, '0'));
    this->append(string);
}

void Packet::write(const double &number, int bits)
{
    QString str = QString::number(number);
    this->append(QString::number(str.length(), 16).rightJustified(bits, '0'));
    this->append(str);
}

void Packet::write(const QStringList &list, int bits, int bitsPerItem)
{
    QString len = QString::number(list.length(), 16).rightJustified(bits, '0');
    this->append(len);

    foreach (QString str, list)
    {
        this->append(QString::number(str.length(), 16).rightJustified(bitsPerItem, '0'));
        this->append(str);
    }
}

void Packet::write(const QList<int> &list, int bits, int bitsPerItem)
{
    QString len = QString::number(list.length(), 16).rightJustified(bits, '0');
    this->append(len);

    foreach (int i, list)
    {
        this->append(QString::number(QString::number(i).length(), 16).rightJustified(bitsPerItem, '0'));
        this->append(QString::number(i));
    }
}

void Packet::writeImage(const QString &fileName, int bits)
{
    QFile file(fileName);
    file.open(QIODevice::ReadOnly);
    QString image = QString(file.readAll().toBase64());
    write(image, bits);
}

void Packet::end()
{
    this->append("\n");
    began = false;
}

quint16 Packet::readCommand()
{
    quint16 ret = this->mid(0, 2).toInt(0, 16);
    this->remove(0, 2);
    return ret;
}

QString Packet::readString(int bits)
{
    int length = this->mid(0, bits).toInt(0, 16);
    QString ret = this->mid(bits, length);
    this->remove(0, bits + length);

    return ret;
}

int Packet::readInt(int bits)
{
    int length = this->mid(0, bits).toInt(0, 16);
    int ret = this->mid(bits, length).toInt();
    this->remove(0, bits + length);

    return ret;
}

double Packet::readDouble(int bits)
{
    int length = this->mid(0, bits).toInt(0, 16);
    double ret = this->mid(bits, length).toDouble();
    this->remove(0, bits + length);

    return ret;
}

QStringList Packet::readStringList(int bits, int bitsPerItem)
{
    QStringList ret;

    int length = this->mid(0, bits).toInt(0, 16);
    this->remove(0, bits);

    for (int i = 0; i < length; i++)
    {
        int len = this->mid(0, bitsPerItem).toInt(0, 16);
        ret << this->mid(bitsPerItem, len);
        this->remove(0, bitsPerItem + len);
    }

    return ret;
}

QList<int> Packet::readIntList(int bits, int bitsPerItem)
{
    QList<int> ret;

    int length = this->mid(0, bits).toInt(0, 16);
    this->remove(0, bits);

    for (int i = 0; i < length; i++)
    {
        int len = this->mid(0, bitsPerItem).toInt(0, 16);
        ret << this->mid(bitsPerItem, len).toInt();
        this->remove(0, bitsPerItem + len);
    }

    return ret;
}

QImage Packet::readImage(int bits)
{
    QByteArray imageData = readString(bits).toUtf8();
    QImage image;
    image.loadFromData(QByteArray::fromBase64(imageData));
    return image;
}

QString Packet::readImageHTML(int bits)
{
    return "<img src='" + readString(bits) + "'></img>";
}

QByteArray Packet::toByteArray()
{
    if (!this->endsWith("\n"))
        this->append("\n");

    return this->toUtf8();
}

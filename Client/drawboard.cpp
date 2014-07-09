#include "drawboard.h"
#include "ui_drawboard.h"

#include <QPainter>

DrawBoard::DrawBoard(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DrawBoard)
{
    ui->setupUi(this);
    m_color = "#000000";
    m_image = QImage(320, 320, QImage::Format_ARGB32_Premultiplied);
    m_image.fill(Qt::transparent);
    this->update();
}

void DrawBoard::giveColor(const QString &color)
{
    m_color = color;
}

DrawBoard::~DrawBoard()
{
    delete ui;
}

QImage DrawBoard::image()
{
    return m_image;
}

void DrawBoard::draw(const QString &color, int x1, int y1, int x2, int y2)
{
    QPainter p;
    p.begin(&m_image);
    p.setPen(QPen(QBrush(QColor(color)), 5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.drawLine(x1, y1, x2, y2);
    p.end();

    this->update(QRect(QPoint(x1, y1), QPoint(x2, y2)).normalized().adjusted(-6, -6, 6, 6));
}

void DrawBoard::clear()
{
    m_image.fill(Qt::transparent);
}

void DrawBoard::drawImage(QImage img)
{
    m_image.fill(Qt::transparent);

    QPainter p;
    p.begin(&m_image);
    p.drawImage(0, 0, img);
    p.end();

    update();
}

void DrawBoard::mousePressEvent(QMouseEvent *e)
{
    mouseDown = true;
    lastPoint = e->pos();

    Packet p;
    p.begin(Enums::DrawCommand);
    p.write(m_color, Enums::ColorLength);
    p.write(lastPoint.x() - 1, Enums::NumberLength);
    p.write(lastPoint.y() - 1, Enums::NumberLength);
    p.write(lastPoint.x(), Enums::NumberLength);
    p.write(lastPoint.y(), Enums::NumberLength);
    p.end();

    sendPacket(p);
}

void DrawBoard::mouseMoveEvent(QMouseEvent *e)
{
    if (mouseDown)
    {
        count = (count + 1) % 4;

        if (count != 0)
            return;

        QPoint pt = e->pos();

        Packet p;
        p.begin(Enums::DrawCommand);
        p.write(m_color, Enums::ColorLength);
        p.write(lastPoint.x(), Enums::NumberLength);
        p.write(lastPoint.y(), Enums::NumberLength);
        p.write(pt.x(), Enums::NumberLength);
        p.write(pt.y(), Enums::NumberLength);
        p.end();

        lastPoint = pt;

        sendPacket(p);
    }
}

void DrawBoard::mouseReleaseEvent(QMouseEvent *e)
{
    mouseDown = false;
}

void DrawBoard::paintEvent(QPaintEvent *e)
{
    QPainter p;
    p.begin(this);
    p.fillRect(e->rect(), Qt::white);
    p.drawImage(e->rect().topLeft(), m_image.copy(e->rect()));
    p.end();
}
